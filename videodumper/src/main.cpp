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

const char* const media_type_to_text( AVMediaType t ){
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
		unsigned *data;
		unsigned pos;
		
	public:
		Plane( unsigned width, unsigned height ) : width( width ), height( height ), pos( 0 ){
			data = new unsigned[ size() ];
		}
		~Plane(){
			delete[] data;
		}
		
		void reset(){ pos = 0; }
		unsigned size() const{ return width * height; }
		
		
		Plane& operator<<=( const unsigned &value ){
			data[pos++] = value;
			return *this;
		}
		
		void save( FILE *f, unsigned depth ){
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
		vector<Plane*> planes;
		unsigned depth;
		AVPixelFormat format;
		
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
			
			depth = 8;
			unsigned amount = 3;
			
			switch( format ){
				case AV_PIX_FMT_YUV420P:
						
					break;
				
				case AV_PIX_FMT_YUV420P10LE:
						depth = 10;
					break;
				
				case AV_PIX_FMT_YUV444P10LE:
						depth = 10;
					break;
				
				default:
					cout << "Unknown format: " << format;
					break; //TODO: throw exception
			}
			
			
			planes.push_back( new Plane( context.width, context.height ) );
			planes.push_back( new Plane( context.width/2, context.height/2 ) );
			planes.push_back( new Plane( context.width/2, context.height/2 ) );
			
		}
		~Planerizer(){
			av_free( frame );
		}
		
		void prepare_planes();
		void save_frame( int index ) const;
		
		operator AVFrame*(){ return frame; }
		
		bool is_keyframe() const{ return frame->key_frame; }
};

void Planerizer::prepare_planes(){
	for( Plane *p : planes )
		p->reset();
	
	for( int iy=0; iy<frame->height; iy++ ){
		uint8_t *data = frame->data[0] + iy*frame->linesize[0];
		
		if( depth <= 8 )
			for( int ix=0; ix<frame->width; ix++)
				*(planes[0]) <<= read8( data );
		else
			for( int ix=0; ix<frame->width; ix++)
				*(planes[0]) <<= read16( data );
	}
	
	for( int iy=0; iy<frame->height/2; iy++ ){
		uint8_t *data2 = frame->data[1] + iy*frame->linesize[1];
		uint8_t *data3 = frame->data[2] + iy*frame->linesize[2];
		
		if( depth <= 8 )
			for( int ix=0; ix<frame->width/2; ix++ ){
				*(planes[1]) <<= read8( data2 );
				*(planes[2]) <<= read8( data3 );
			}
		else
			for( int ix=0; ix<frame->width/2; ix++ ){
				*(planes[1]) <<= read16( data2 );
				*(planes[2]) <<= read16( data3 );
			}
	}
	
}

void Planerizer::save_frame( int index ) const{
	string name = "out/output";
	string id = boost::lexical_cast<string>( index );
	name += string( "00000" ).replace( 5-id.length(), id.length(), id );
	if( frame->key_frame )
		name += "k";
	name += ".dump";
	cout << index << "\n";
	
	FILE *file = fopen( name.c_str(), "wb" );
	if( file ){
		for( Plane *p : planes )
			p->save( file, depth );
		fclose( file );
	}
	else
		cout << "shit: " << name.c_str() << "\n";
	
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
		void run();
		
		void only_keyframes(){
			codec_context->skip_loop_filter = AVDISCARD_NONKEY;
			codec_context->skip_idct = AVDISCARD_NONKEY;
			codec_context->skip_frame = AVDISCARD_NONKEY;
			codec_context->lowres = 2;
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

void VideoFile::run(){
	Planerizer frame( *codec_context );
	
	AVPacket packet;
	int frame_done;
	int current = 0;
	while( av_read_frame( format_context, &packet ) >= 0 ){
		if( packet.stream_index == stream_index ){
			avcodec_decode_video2( codec_context, frame, &frame_done, &packet );
			
			if( frame_done ){
			//	if( frame.is_keyframe() ){
					frame.prepare_planes();
					frame.save_frame( current );
			//		cout << current << "\n";
			//	}
				current++;
				if( current >= 25000 )
					return;
			}
			
		}
		av_free_packet( &packet );
	}
	
}

#include <QTime>
#include <QCoreApplication>
#include <QStringList>
#include <boost/lexical_cast.hpp>
int main( int argc, char* argv[] ){
	QCoreApplication app( argc, argv );
	av_register_all();
	
	//TODO: check arguments
	QStringList args = app.arguments();
	if( args.count() < 2 ){
		cout << "Usage: video_dumper filename [min [sec]]";
		return -1;
	}
	
	//Get seeking values
	unsigned min=0;
	unsigned sec=0;
	if( argc >= 3 )
		min = boost::lexical_cast<unsigned>( args[2].toUtf8().constData() );
	if( argc >= 4 )
		sec = boost::lexical_cast<unsigned>( args[3].toUtf8().constData() );
	
	VideoFile file( args[1] );
	if( !(file.open()) ){
		cout << "Couldn't open file!";
		return -1;
	}
	
//	file.debug_containter();
	file.debug_video();
	
	if( min || sec ){
		cout << "Seeking to " << min << ":" << sec << "\n";
		file.seek( min, sec );
	}
	
//	file.only_keyframes();
	QTime t;
	t.start();
	file.run();
	qDebug( "took: %d", t.restart() );
	
	//codec_context->thread_count = 4;
	
	
	
//	cout << "Size of video is: " << codec_context->coded_width << "x" << codec_context->coded_height << " with type " << codec_context->pix_fmt << "\n";
	
	return 0;
}
