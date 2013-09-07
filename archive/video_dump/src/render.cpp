#include "render.hpp"
#include <QPainter>
#include <vlc/vlc.h>

#include <cstdio>
using namespace std;

struct Plane{
	char *data;
	unsigned width;
	unsigned height;
	unsigned scan_width;
	unsigned depth;
	
	unsigned byte_count() const{ return (depth + 7) / 8; } //ciel to multiple of 8
	
	void save( FILE *f );
};

void Plane::save( FILE *f ){
	if( f ){
		fwrite( &width, sizeof(unsigned), 1, f );
		fwrite( &height, sizeof(unsigned), 1, f );
		fwrite( &depth, sizeof(unsigned), 1, f );
		fwrite( data, sizeof(unsigned char), height*scan_width, f );
	}
}

//Static functions to bind callbacks
static void s_lock( void *opaque, void **planes ){
	((ARender*)opaque)->lock( planes );
}

static void s_unlock( void *opaque, void *picture, void *const *planes ){
	((ARender*)opaque)->unlock( picture, planes );
}

static void s_display( void *opaque, void *picture ){
	((ARender*)opaque)->display( picture );
}

static void bind_format( void **opaque, char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines ){
	((ARender*)*opaque)->format( chroma, *width, *height, pitches, lines );
}

static void s_cleanup( void *opaque ){
	((ARender*)opaque)->cleanup();
}

void ARender::set_calbacks( libvlc_media_player_t *mp ){
	libvlc_video_set_callbacks(
			mp
		,	(libvlc_video_lock_cb)s_lock
		,	(libvlc_video_unlock_cb)s_unlock
		,	(libvlc_video_display_cb)s_display
		,	(void*)this
		);
	
	libvlc_video_set_format_callbacks(
			mp
		,	(libvlc_video_format_cb)bind_format
		,	(libvlc_video_cleanup_cb)s_cleanup
	);
}

Render::Render( QWidget *parent ) : ARender( parent ){
	//Init planes array
	data_planes = new Plane[PLANES_AMOUNT];
	for( unsigned i=0; i<PLANES_AMOUNT; ++i )
		data_planes[i].data = NULL;
	
	save = false;
}

Render::~Render(){
}

void Render::display( void *picture ){
	update();
}

void Render::lock( void **planes ){
	surface_lock.lock();
	for( unsigned i=0; i<PLANES_AMOUNT; ++i )
		planes[i] = data_planes[i].data;
}

void Render::unlock( void *picture, void *const *planes ){
	surface_lock.unlock();
}

void Render::format( char *chroma, unsigned &width, unsigned &height, unsigned *pitches, unsigned *lines ){
	qDebug( "Chroma is: %s", chroma );
	qDebug( "Size is %dx%d", width, height );
	
	for( unsigned i=0; i<PLANES_AMOUNT; ++i ){
		//Set normal values
		unsigned div = (i>0?2:1);
		Plane &p( data_planes[i] );
		p.depth = 8;
		p.width = width / div;
		p.height = height / div;
		
		//Override normal values
		if( strcmp( chroma, "I0AL" ) == 0 )
			p.depth = 10;
		if( strcmp( chroma, "I4AL" ) == 0 ){
			p.depth = 10;
			p.width = width;
			p.height = height;
		}
		
		p.scan_width = p.width * p.byte_count();
		
		p.data = new char[ p.height * p.scan_width ];
	}
	
	for( unsigned i=0; i<PLANES_AMOUNT; ++i ){
		Plane &p( data_planes[i] );
		pitches[i] = p.scan_width;
		lines[i] = p.height;
	}
	for( unsigned i=0; i<PLANES_AMOUNT; ++i )
		qDebug( "pitches[%d]: %d, lines[%d]: %d", i, pitches[i], i, lines[i] );
}

void Render::cleanup(){
	for( unsigned i=0; i<PLANES_AMOUNT; ++i ){
		Plane &p( data_planes[i] );
		//Make sure we can't access memory by accident
		p.width = 0;
		p.height = 0;
		
		//Delete the array
		if( p.data ){
			delete[] p.data;
			p.data = NULL;
		}
	}
}

typedef unsigned short value_t;

template <class T> void draw_plane( const Plane &plane, QImage &img, unsigned x, unsigned y ){
		for( unsigned iy=0; iy<plane.height; ++iy ){
			QRgb* row = (QRgb*)img.scanLine( iy + y ) + x;
			for( unsigned ix=0; ix<plane.width; ++ix ){
				T color = ((T*)(plane.data + iy*plane.scan_width))[ix];
				
				color >>= (plane.depth > 8 ) ? plane.depth - 8 : 0;
				row[ix] = qRgb( color, color, color );
			}
		}

}

void Render::paintEvent( QPaintEvent *event ){
	QPainter painter( this );
	static unsigned save_id = 0;
	
	//Set op some areas
	QRect drawing_area( QPoint(), size() );
	painter.fillRect( drawing_area, Qt::gray );
	
	if( data_planes[0].data ){
		QImage img(
				data_planes[0].width + data_planes[1].width
			,	data_planes[1].height + data_planes[2].height
			,	QImage::Format_RGB32
			);
		
		//Draw luma
		if( data_planes[0].byte_count() == 1 ){
			draw_plane<unsigned char>( data_planes[0], img, 0,0 );
			draw_plane<unsigned char>( data_planes[1], img, data_planes[0].width,0 );
			draw_plane<unsigned char>( data_planes[2], img, data_planes[0].width,data_planes[1].height );
		}
		else{
			draw_plane<unsigned short>( data_planes[0], img, 0,0 );
			draw_plane<unsigned short>( data_planes[1], img, data_planes[0].width,0 );
			draw_plane<unsigned short>( data_planes[2], img, data_planes[0].width,data_planes[1].height );
		}
	
		painter.drawImage( 0,0, img );
		
		if( save ){
			const char* path = QString( "C:/#anime/out/" + QString::number( save_id ) + ".dump" ).toLocal8Bit().constData();
			qDebug( path );
			FILE *f = fopen( path, "wb" );
			data_planes[0].save( f );
			data_planes[1].save( f );
			data_planes[2].save( f );
			fclose( f );
			save_id++;
		}
	}
}

