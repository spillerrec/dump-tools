#include <iostream>
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
}

using namespace std;

const char* media_type_to_text( AVMediaType t ){
	switch( t ){
		case AVMEDIA_TYPE_UNKNOWN: return "unknown";
		case AVMEDIA_TYPE_VIDEO: return "video";
		case AVMEDIA_TYPE_AUDIO: return "audio";
		case AVMEDIA_TYPE_SUBTITLE: return "subtitle";
		case AVMEDIA_TYPE_ATTACHMENT: return "attachment";
		default: return "invalid";
	}
}

#include <QDebug>

class Plane{
	private:
		unsigned width;
		unsigned height;
		vector<unsigned> data;
		unsigned pos;
		
	public:
		Plane( unsigned width, unsigned height )
			:	width( width ), height( height ), data( size() ), pos( 0 ) { }
		
		void reset(){ pos = 0; }
		unsigned size() const{ return width * height; }
		
		unsigned getWidth() const{ return width; }
		unsigned getHeight() const{ return height; }
		
		
		Plane& operator<<=( unsigned value ){
			data[pos++] = value;
			return *this;
		}
		
		void save( FILE *f, unsigned depth ) const{
			fwrite( &width, sizeof(unsigned), 1, f );
			fwrite( &height, sizeof(unsigned), 1, f );
			fwrite( &depth, sizeof(unsigned), 1, f );
			
			unsigned byte_size = size() * (depth<=8 ? 1 : 2);
			char *temp = new char[ byte_size ];
			if( depth <= 8 )
				for( unsigned i=0; i<size(); i++ )
					temp[i] = data[i];
			else
				for( unsigned i=0; i<size(); i++ ){
					temp[i*2] = data[i] & 0x00FF;
					temp[i*2+1] = ( data[i] & 0xFF00 ) >> 8;
				}
			
			fwrite( temp, sizeof(char), byte_size, f );
			delete temp;
		}
};


class Planerizer{
	private:
		AVFrame *frame;
		AVCodecContext &context;
		vector<Plane> planes;
		AVPixelFormat format;
		
		unsigned depth{ 8 };
		bool planar{ true };
		
		unsigned read8( uint8_t *&data ) const{
			return *(data++);
		}
		unsigned read16( uint8_t *&data ) const{
			unsigned p1 = *(data++);
			unsigned p2 = *(data++);
			return ( p1 + (p2 << 8) );
		}
		
	public:
		Planerizer( AVCodecContext &context ) : context( context ), format( context.pix_fmt ){
			frame = avcodec_alloc_frame();
			
			bool half_width = true;
			bool half_height = true;
			
			switch( format ){
				// Half-width packed
				case AV_PIX_FMT_YUYV422:
						planar = false;
						half_height = false;
					break;
					
				// Quarter chroma planar
				case AV_PIX_FMT_YUV420P10LE:
						depth = 10;
				case AV_PIX_FMT_YUV420P: break;
				
				// Full chroma planar
				case AV_PIX_FMT_YUV444P10LE:
						depth = 10;
				case AV_PIX_FMT_YUV444P:
						half_width = half_height = false;
					break;
				
				default:
					cout << "Unknown format: " << format;
					break; //TODO: throw exception
			}
			
			
			auto chroma_width = half_width ? context.width/2 : context.width;
			auto chroma_height = half_height ? context.height/2 : context.height;
			planes.emplace_back( Plane( context.width, context.height ) );
			planes.emplace_back( Plane( chroma_width, chroma_height ) );
			planes.emplace_back( Plane( chroma_width, chroma_height ) );
			
		}
		~Planerizer(){
			av_free( frame );
		}
		
		void prepare_planes();
		void save_frame( QString name, int index ) const;
		
		operator AVFrame*(){ return frame; }
		
		bool is_keyframe() const{ return frame->key_frame; }
};

void Planerizer::prepare_planes(){
	for( Plane& p : planes )
		p.reset();
	
	if( planar ){
		auto readPlane = [=]( int index, Plane& p, unsigned (Planerizer::*f)( uint8_t*& ) const ){
				//Read an entire plane into 'p', using reader 'f'
				for( unsigned iy=0; iy<p.getHeight(); iy++ ){
					uint8_t *data = frame->data[index] + iy*frame->linesize[index];
					
					for( unsigned ix=0; ix < p.getWidth(); ix++)
						p <<= (this->*f)( data );
				}
			};
		
		for( int i=0; i<3; i++ )
			readPlane( i, planes[i], (depth <= 8) ? &Planerizer::read8 : &Planerizer::read16 );
	}
	else{	
		auto& luma = planes[0];
		
		for( unsigned iy=0; iy<luma.getHeight(); iy++ ){
			uint8_t *data = frame->data[0] + iy*frame->linesize[0];
			//TODO: support other packing formats, and higher bit depths
			
			for( unsigned ix=0; ix < luma.getWidth()/2; ix++){
				luma      <<= read8( data );
				planes[1] <<= read8( data );
				luma      <<= read8( data );
				planes[2] <<= read8( data );
			}
		}
	}
}

void Planerizer::save_frame( QString name, int index ) const{
	name += "/output";
	string id = boost::lexical_cast<string>( index );
	name += QString::fromStdString( string( "00000" ).replace( 5-id.length(), id.length(), id ) );
	if( frame->key_frame )
		name += "k";
	name += ".dump";
	cout << index << "\n";
	
	FILE *file = fopen( name.toLocal8Bit().constData(), "wb" );
	if( file ){
		for( auto& p : planes )
			p.save( file, depth );
		fclose( file );
	}
	else
		cout << "shit: " << name.toLocal8Bit().constData() << "\n";
	
}


void debug_frame( AVFrame &frame ){
	cout << "Frame:\n";
	cout << "\t" << "Size: " << frame.width << "x" << frame.height << "\n";
	if( frame.key_frame )
		cout << "\t" << "Keyframe\n";
	if( frame.interlaced_frame )
		cout << "\t" << "Interlace line: " << frame.top_field_first << "\n";
}


class VideoFile{
	private:
		QString filepath;
		AVFormatContext* format_context;
		AVCodecContext* codec_context;
		AVPacket packet;
		
		int stream_index;
		
		
	public:
		VideoFile( QString filepath )
			:	filepath( filepath )
			,	format_context( nullptr )
			,	codec_context( nullptr )
			{ }
		
		bool open();
		bool seek( unsigned min, unsigned sec );
		bool seek( int64_t byte );
		void run( QString dir );
		
		void only_keyframes(){
			codec_context->skip_loop_filter = AVDISCARD_NONKEY;
			codec_context->skip_idct = AVDISCARD_NONKEY;
			codec_context->skip_frame = AVDISCARD_NONKEY;
		//	codec_context->lowres = 2;
		}
		
		void debug_containter();
		void debug_video();
};

bool VideoFile::open(){
	if( avformat_open_input( &format_context
		,	filepath.toLocal8Bit().constData(), nullptr, nullptr ) ){
		cout << "Couldn't open file, either missing, unsupported or corrupted\n";
		return false;
	}
	
	if( avformat_find_stream_info( format_context, nullptr ) < 0 ){
		cout << "Couldn't find stream\n";
		return false;
	}
	
	//Find the first video stream (and be happy)
	for( unsigned i=0; i<format_context->nb_streams; i++ ){
		if( format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO ){
			stream_index = i;
			codec_context = format_context->streams[i]->codec;
			break;
		}
	}
	
	if( !codec_context ){
		cout << "Couldn't find a video stream!\n";
		return false;
	}
	
	AVCodec *codec = avcodec_find_decoder( codec_context->codec_id );
	if( !codec ){
		cout << "Does not support this video codec :\\\n";
		return false;
	}
	
	if( avcodec_open2( codec_context, codec, nullptr ) < 0 ){
		cout << "Couldn't open codec\n";
		return false;
	}
	
	return true;
}

void VideoFile::debug_containter(){
	cout << "Number of streams: " << format_context->nb_streams << "\n";
	for( unsigned i=0; i<format_context->nb_streams; i++ ){
		cout << "\t" << i << ":\t" << media_type_to_text( format_context->streams[i]->codec->codec_type ) << "\n";
	}
}

void VideoFile::debug_video(){
	cout << "time_base: " << format_context->streams[stream_index]->time_base.num << "." << format_context->streams[stream_index]->time_base.den << "\n";
	cout << "AV_TIME_BASE_Q: " << AV_TIME_BASE_Q.num << "." << AV_TIME_BASE_Q.den << "\n";
	cout << "Primaries: " << codec_context->color_primaries << "\n";
	cout << "Transfer: " << codec_context->color_trc << "\n";
	cout << "ColorSpace: " << codec_context->colorspace << "\n";
	cout << "Range: " << codec_context->color_range << "\n";
}

bool VideoFile::seek( unsigned min, unsigned sec ){
	int64_t target = av_rescale_q( min * 60 + sec, format_context->streams[stream_index]->time_base, AV_TIME_BASE_Q );
	if( av_seek_frame( format_context, stream_index, target, AVSEEK_FLAG_ANY ) < 0 ){
		cout << "Couldn't seek\n";
		return false;
	}
	avcodec_flush_buffers( codec_context );
	cout << "target: " << target << "\n";
	return true;
}

bool VideoFile::seek( int64_t byte ){
	if( av_seek_frame( format_context, stream_index, byte, AVSEEK_FLAG_BYTE ) < 0 ){
		cout << "Couldn't seek\n";
		return false;
	}
	avcodec_flush_buffers( codec_context );
	cout << "target: " << byte << "\n";
	return true;
}

void VideoFile::run( QString dir ){
	Planerizer frame( *codec_context );
	
	AVPacket packet;
	int frame_done;
	int current = 0;
	while( av_read_frame( format_context, &packet ) >= 0 ){
		if( packet.stream_index == stream_index ){
			avcodec_decode_video2( codec_context, frame, &frame_done, &packet );
			
			if( frame_done ){
				frame.prepare_planes();
				frame.save_frame( dir, current++ );
			}
		}
		av_free_packet( &packet );
	}
}

#include <boost/lexical_cast.hpp>
#include <QFileInfo>
static void seekFromString( VideoFile& file, QString file_name, QString position ){
	if( position.isEmpty() )
		return;
	
	int index = position.indexOf( ':' );
	if( index > 0 ){ //Exists, and must not be in the start
		QString minutes = position.left( index );
		QString seconds = position.right( position.count() - index - 1 );
		
		//TODO: catch exceptions
		unsigned min = boost::lexical_cast<unsigned>( minutes.toUtf8().constData() );
		unsigned sec = boost::lexical_cast<unsigned>( seconds.toUtf8().constData() );
		
		file.seek( min, sec );
	}
	else if( position.endsWith( '%' ) ){
		QString value = position.left( position.count() - 1 );
		double percentage = boost::lexical_cast<double>( value.toUtf8().constData() );
		
		QFileInfo info( file_name );
		file.seek( (int64_t)( info.size() * percentage / 100 ) );
		qDebug() << percentage;
	}
	else{
		cout << "Could not understand \"" << position.toLocal8Bit().constData() << "\", use [min:sec] or [percentage%]\n";
		return; //TODO: exit
	}
}

#include <QTime>
#include <QDebug>
#include <QCoreApplication>
#include <QStringList>
#include <QCommandLineParser>
#include <QDir>
int main( int argc, char* argv[] ){
	QCoreApplication app( argc, argv );
	av_register_all();
	
	QCommandLineParser parser;
	parser.addOption( { { "d", "dir" }, "Output directory, will be created if it does not exist.", "folder", "out" } );
	parser.addPositionalArgument( "video file", "File to dump video frames from" );
	parser.addPositionalArgument( "seek", "Seek to a specific time", "[min:sec]" );
	
	parser.process(app);
	
	auto args = parser.positionalArguments();
	if( args.count() < 1 ){
		parser.helpText();
		cout << "Test";
		return -1;
	}
	
	QString file_name = args[0];
	QString position = args.count() >=2 ? args[1] : "";
	
	VideoFile file( file_name );
	if( !(file.open()) )
		return -1;
	
//	file.debug_containter();
	file.debug_video();
	
	seekFromString( file, file_name, position );
	
//	file.only_keyframes();
	QTime t;
	t.start();
	auto dir = parser.value("d");
	if( !QDir(".").mkpath( dir ) ){
		cout << "Could not create output directory";
		return -1;
	}
	file.run( dir );
	qDebug( "took: %d", t.restart() );
	
	return 0;
}
