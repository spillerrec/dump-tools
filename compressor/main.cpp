#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QFileInfo>

#include <iostream>
#include <stdint.h>
#include <cstring>
#include <vector>
using namespace std;

#include <zlib.h>
#include <lzma.h>

const bool USE_LZMA = true;

struct Plane{
	private:
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint16_t depth{ 0 };
		uint16_t config{ 0 };
		
		vector<uint8_t> data;
		
	public:
		bool read( QIODevice &dev );
		bool write( QIODevice &dev );
		int32_t size(){ return width*height*((depth + 7) / 8); }
		
		uint16_t read_16( QIODevice &dev ){
			char byte1, byte2;
			if( !dev.getChar( &byte1 ) )
				return 0;
			if( !dev.getChar( &byte2 ) )
				return 0;
			return ((uint16_t)byte2 << 8) + (uint8_t)byte1;
		}
		
		uint32_t read_32( QIODevice &dev ){
			uint16_t byte1 = read_16( dev );
			uint32_t byte2 = read_16( dev );
			return (byte2 << 16) + byte1;
		}
		
		void write_16( QIODevice &dev, uint16_t val ){
			dev.write( (char*)&val, 2 );
		}
		void write_32( QIODevice &dev, uint32_t val ){
			dev.write( (char*)&val, 4 );
		}
		
		void compression_ratio( unsigned compressed_size ) const{
			cout << "\tCompressed (" << width << "x" << height << ") to " << 100.0-compressed_size*100.0/size() << "%\n";
		}
};

bool Plane::read( QIODevice &dev ){
	width  = read_32( dev );
	height = read_32( dev );
	depth  = read_16( dev );
	config = read_16( dev );
//	cout << "settings: " << width << "x" << height << "@" << depth << "p : " << config << endl;
	
	if( width == 0 || height == 0 || depth > 32 )
		return false;
	
	if( config & 0x1 ){
		uint32_t lenght = read_32( dev );
		if( lenght == 0 )
			return false;
		
		vector<char> buf( lenght );
		dev.read( buf.data(), lenght );
		
		uLongf uncompressed = size();
		data.resize( uncompressed );
		
		if( uncompress( (Bytef*)data.data(), &uncompressed, (Bytef*)buf.data(), lenght ) != Z_OK )
			return false;
	}
	else if( config & 0x2 ){
		//Initialize decoder
		lzma_stream strm = LZMA_STREAM_INIT;
		if( lzma_stream_decoder( &strm, UINT64_MAX, 0 ) != LZMA_OK )
			return false;
		
		//Read data
		uint32_t lenght = read_32( dev );
		if( lenght == 0 )
			return false;
		
		vector<char> buf( lenght );
		dev.read( buf.data(), lenght );
		
		data.resize( size() );
		
		//Decompress
		strm.next_in = (uint8_t*)buf.data();
		strm.avail_in = buf.size();
		
		strm.next_out = data.data();
		strm.avail_out = data.size();
		
		if( lzma_code( &strm, LZMA_FINISH ) != LZMA_STREAM_END ){
			cout << "Shit, didn't finish decompressing!" << endl;
			return false;
		}
		
		lzma_end(&strm);
	}
	else{
		data.resize( size() );
		return dev.read( (char*)data.data(), size() ) == size();
	}
	
	return true;
}

bool Plane::write( QIODevice &dev ){
	write_32( dev, width );
	write_32( dev, height );
	write_16( dev, depth );
	
	if( USE_LZMA ){
		write_16( dev, 0x2 );
		
		lzma_stream strm = LZMA_STREAM_INIT;
		if( lzma_easy_encoder( &strm, 9 | LZMA_PRESET_EXTREME, LZMA_CHECK_CRC64 ) != LZMA_OK )
			return false;
		
		strm.next_in = data.data();
		strm.avail_in = size();
		
		auto buf_size = strm.avail_in * 2;
		strm.avail_out = buf_size;
		vector<uint8_t> buf( buf_size );
		strm.next_out = buf.data();
		
		lzma_ret ret = lzma_code( &strm, LZMA_FINISH );
		if( ret != LZMA_STREAM_END ){
			cout << "Nooo, didn't finish compressing!" << endl;
			lzma_end( &strm ); //TODO: Use RAII
			return false;
		}
		
		auto final_size = buf_size - strm.avail_out;
		write_32( dev, final_size );
		dev.write( (char*)buf.data(), final_size );
		
		lzma_end( &strm );
		
		compression_ratio( final_size );
	}
	else{
		write_16( dev, 0x1 );
		
		uLongf buf_size = compressBound( size() );
		vector<uint8_t> buf( buf_size );
		
		if( compress( (Bytef*)buf.data(), &buf_size, (Bytef*)data.data(), size() ) != Z_OK )
			return false;
		
		write_32( dev, buf_size );
		dev.write( (char*)buf.data(), buf_size );
		
		compression_ratio( buf_size );
	}
	return true;
}

int main( int argc, char* argv[] ){
	QCoreApplication app( argc, argv );
	
	QStringList args = app.arguments();
	for( int i=1; i<args.count(); ++i ){
		QFileInfo info( args[i] );
		if( info.suffix() != "dump" )
			continue;
		
		cout << "[" << i << "/" << args.count()-1 << "] Processing: " << args[i].toLocal8Bit().constData() << "\n";
		QFile f( args[i] );
		QString new_name = info.completeBaseName() + ".compressed.dump";
		if( f.open( QIODevice::ReadOnly ) ){
			QFile copy( new_name );
			if( copy.open( QIODevice::WriteOnly ) ){
				while( true ){
					Plane p;
					if( !p.read( f ) )
						break;
					p.write( copy );
				}
				
				copy.close();
			}
			f.close();
			
			if( QFileInfo( new_name ).size() > 0 ){
				QFile::remove( args[i] );
			}
		}
		
	}
	
	return 0;
}