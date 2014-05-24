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
	along with dump-tools.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DUMP_PLANE_HPP
#define DUMP_PLANE_HPP

#include <propsys.h>
#include <ObjIdl.h>

#include <iostream>
#include <stdint.h>
#include <cstring>
#include <vector>


struct Plane{
	public:
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint16_t depth{ 0 };
		uint16_t config{ 0 };
		
	private:
		std::vector<uint8_t> data;

		template<typename T>
		T read( IStream *pstream, bool &success ) {
			//TODO: have alternative implementation for big-endian
			ULONG read_bytes;
			T temp;
			success &= S_OK == pstream->Read( &temp, sizeof(T), &read_bytes );
			return temp;
		}

	public:
		bool read( IStream *dev );
		int32_t size() const{ return width*height*((depth + 7) / 8); }

		int byte_count( ) const { return (depth-1) / 8 + 1; }

		const char* scanline( int y ) const {
			return (const char*)data.data() + (uint64_t)width * y * byte_count( );
		}
		
		uint16_t read_16( IStream *dev ) {
			bool still_has_data = true;
			char byte1 = read<uint8_t>( dev, still_has_data );
			char byte2 = read<uint8_t>( dev, still_has_data );
			return ((uint16_t)byte2 << 8) + (uint8_t)byte1;
		}
		
		uint32_t read_32( IStream *dev ) {
			uint16_t byte1 = read_16( dev );
			uint32_t byte2 = read_16( dev );
			return (byte2 << 16) + byte1;
		}
};

#endif
