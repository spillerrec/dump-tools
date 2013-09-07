#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QFileInfo>

#include <iostream>
#include <stdint.h>
#include <cstring>
using namespace std;

#include <zlib.h>

struct Plane{
	private:
		uint32_t width;
		uint32_t height;
		uint16_t depth;
		uint16_t config;
		
		uint8_t* data;
		
	public:
		Plane() : width(0), height(0), depth(0), config(0), data(nullptr) { }
		~Plane(){ if( data ) delete[] data; }
		
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
};

bool Plane::read( QIODevice &dev ){
	width  = read_32( dev );
	height = read_32( dev );
	depth  = read_16( dev );
	config = read_16( dev );
	
	if( width == 0 || height == 0 || depth > 32 )
		return false;
	
	if( config & 0x1 ){
		uint32_t lenght = read_32( dev );
		if( lenght == 0 )
			return false;
		
		char* buf = new char[ lenght ];
		if( !buf )
			return false;
		dev.read( buf, lenght );
		
		uLongf uncompressed = size();
		data = new uint8_t[ uncompressed ];
		if( !data ){
			delete[] buf;
			return false;
		}
		
		if( uncompress( (Bytef*)data, &uncompressed, (Bytef*)buf, lenght ) != Z_OK ){
			delete[] buf;
			return false;
		}
	}
	else{
		data = new uint8_t[ size() ];
		if( !data )
			return false;
		
		if( dev.read( (char*)data, size() ) != size() )
			return false;
	}
	
	return true;
}

bool Plane::write( QIODevice &dev ){
	write_32( dev, width );
	write_32( dev, height );
	write_16( dev, depth );
	write_16( dev, 0x1 );
	
	uLongf buf_size = compressBound( size() );
	uint8_t *buf = new uint8_t[ buf_size ];
	if( !buf )
		return false;
	
	if( compress( (Bytef*)buf, &buf_size, (Bytef*)data, size() ) != Z_OK ){
		delete[] buf;
		return false;
	}
	
	write_32( dev, buf_size );
	dev.write( (char*)buf, buf_size );
	
	cout << "\tCompressed plane (" << width << "x" << height << ") to " << 100.0-(double)buf_size/size()*100 << "%\n";
	
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
		if( f.open( QIODevice::ReadOnly ) ){
			QFile copy( info.baseName() + ".compressed.dump" );
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
		}
		
	}
	
	return 0;
}