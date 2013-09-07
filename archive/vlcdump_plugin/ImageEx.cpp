/*
	This file is part of Overmix.

	Overmix is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Overmix is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Overmix.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ImageEx.hpp"
#include "Plane.hpp"
#include "MultiPlaneIterator.hpp"
#include "color.hpp"

#include <limits>


static const double DOUBLE_MAX = std::numeric_limits<double>::max();

bool ImageEx::read_dump_plane( QIODevice &f, unsigned index ){
	unsigned width, height;
	f.read( (char*)&width, sizeof(unsigned) );
	f.read( (char*)&height, sizeof(unsigned) );
	
	Plane* p = new Plane( width, height );
	planes[index] = p;
	if( !p )
		return false;
	
	if( index < 1 )
		infos[index] = new PlaneInfo( *p, 0,0, 1 );
	else
		infos[index] = new PlaneInfo( *p, 1,1, 2 );
	if( !infos )
		return false;
	
	unsigned depth;
	f.read( (char*)&depth, sizeof(unsigned) );
	unsigned byte_count = (depth + 7) / 8;
	
	for( unsigned iy=0; iy<height; iy++ ){
		color_type *row = p->scan_line( iy );
		for( unsigned ix=0; ix<width; ++ix, ++row ){
			f.read( (char*)row, byte_count );
			*row <<= 16 - depth;
		}
	}
	
	return true;
}

bool ImageEx::from_dump( QIODevice &f ){
	bool result = true;
	result &= read_dump_plane( f, 0 );
	result &= read_dump_plane( f, 1 );
	result &= read_dump_plane( f, 2 );
	
	type = YUV;
	
	return result;
}


bool ImageEx::create( unsigned width, unsigned height ){
	if( initialized )
		return false;
	
	for( int i=0; i<4; i++ ){
		if( !( planes[i] = new Plane( width, height ) ) )
			return false;
		if( !( infos[i] = new PlaneInfo( *planes[i], 0,0,1 ) ) )
			return false;
	}
	
	return initialized = true;
}

QImage ImageEx::to_qimage(){
	if( !planes || !planes[0] )
		return QImage();
	
	Plane *temp = planes[1]->scale_nearest( planes[0]->get_width(), planes[0]->get_height(), 0,0 );
	delete planes[1];
	planes[1] = temp;
	temp = planes[2]->scale_nearest( planes[0]->get_width(), planes[0]->get_height(), 0,0 );
	delete planes[2];
	planes[2] = temp;
	
	//Create iterator
	std::vector<PlaneItInfo> info;
	info.push_back( PlaneItInfo( planes[0], 0,0 ) );
	info.push_back( PlaneItInfo( planes[1], 0,0 ) );
	info.push_back( PlaneItInfo( planes[2], 0,0 ) );
	MultiPlaneIterator it( info );
	it.iterate_all();
	
	//color *line = new color[ width+1 ];
	
	//Create image
	QImage img(	it.width(), it.height()
		,	( planes[4] ) ? QImage::Format_ARGB32 : QImage::Format_RGB32
		);
	img.fill(0);
	
	//TODO: select function
	
	for( unsigned iy=0; iy<it.height(); iy++, it.next_line() ){
		QRgb* row = (QRgb*)img.scanLine( iy );
		for( unsigned ix=0; ix<it.width(); ix++, it.next_x() ){
			row[ix] = ( type == YUV ) ? it.yuv_to_qrgb() : it.rgb_to_qrgb();
		}
	}
	
	return img;
}


