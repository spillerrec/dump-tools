/*	This file is part of dump-tools.

	dump-tools is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dump-tools is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dump-tools.  If 'not, see <http://www.gnu.org/licenses/>.
*/

#include "DumpPlane.hpp"

#include <zlib.h>

extern "C"{
	#include <lzma.h>
}

using namespace std;

const bool USE_LZMA = true;

bool Plane::read( IStream *dev ) {
	ULONG read_bytes;

	width  = read_32( dev );
	height = read_32( dev );
	depth  = read_16( dev );
	config = read_16( dev );
	cout << "settings: " << width << "x" << height << "@" << depth << "p : " << config << endl;
	
	if( width == 0 || height == 0 || depth > 32 )
		return false;
	
	if( config & 0x1 ){
		uint32_t lenght = read_32( dev );
		if( lenght == 0 )
			return false;
		
		vector<char> buf( lenght );
		dev->Read( buf.data( ), lenght, &read_bytes );
		
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
		dev->Read( buf.data( ), lenght, &read_bytes );
		
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
		data.resize( size( ) );
		dev->Read( data.data( ), size(), &read_bytes );
	}
	
	return true;
}
