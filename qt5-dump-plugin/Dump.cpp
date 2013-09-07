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

#include "Dump.hpp"

#include <vector>
#include <cmath>
using namespace std;

#include <QColor>

uint16_t* Dump::gamma = nullptr;

void yuv_to_rgb( uint16_t &r, uint16_t &g, uint16_t &b, uint16_t max, double kr, double kg, double kb ){
	double y = r / (double)max;
	double cb = g / (double)max;
	double cr = b / (double)max;
	
	//Remove foot- and head-room
	y = (y - (16 / 255.0)) * ( 1 + 16.0 / 255.0 + (256-235) / 255.0 );
	cb = (cb - (16 / 255.0)) * ( 1 + 16.0 / 255.0 + (256-240) / 255.0 );
	cr = (cr - (16 / 255.0)) * ( 1 + 16.0 / 255.0 + (256-240) / 255.0 );
	
	//Don't let it outside the allowed range
	y = (y < 0 ) ? 0 : (y > 1 ) ? 1 : y;
	cb = (cb < 0 ) ? 0 : (cb > 1 ) ? 1 : cb;
	cr = (cr < 0 ) ? 0 : (cr > 1 ) ? 1 : cr;
	
	//Move chroma range
	cb -= 0.5;
	cr -= 0.5;
	
	//Convert to R'G'B'
	double rr = y + 2*(1-kr) * cr;
	double rb = y + 2*(1-kb) * cb;
	double rg = y - 2*kr*(1-kr)/kg * cr - 2*kb*(1-kb)/kg * cb;
	
	//Don't let it outside the allowed range
	//Should not happen, so we can probably remove this later
	rr = (rr < 0 ) ? 0 : (rr > 1 ) ? 1 : rr;
	rg = (rg < 0 ) ? 0 : (rg > 1 ) ? 1 : rg;
	rb = (rb < 0 ) ? 0 : (rb > 1 ) ? 1 : rb;
	
	//Transform range
	r = (rr) * UINT16_MAX;
	g = (rg) * UINT16_MAX;
	b = (rb) * UINT16_MAX;
}

static double ycbcr2srgb( double v ){
	//rec. 709
	v = ( v < 0.08125 ) ? 1.0/4.5 * v : pow( (v+0.099)/1.099, 1.0/0.45 );
	v = ( v <= 0.0031308 ) ? 12.92 * v : 1.055*pow( v, 1.0/2.4 ) - 0.055;
	return v;
}

void Dump::generate_gamma(){
	if( !gamma )
		return;
	
	gamma = new uint16_t[UINT16_MAX];
	
	for( int_fast32_t i=0; i<UINT16_MAX; i++ ){
		long double v = i;
		v /= UINT16_MAX;
		
		v = ycbcr2srgb( v );
		
		gamma[i] = v*UINT16_MAX + 0.5;
	}
}

struct Plane{
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	const char* const data;
	
	Plane( const char* const data ) : data( data ){ }
	
	int byte_count() const{ return (depth-1) / 8 + 1; }
	
	const char* end() const{
		return data + (uint64_t)width * height * byte_count();
	}
	
	const char* scanline( int y ) const{
		return data + (uint64_t)width * y * byte_count();
	}
};

QImage Dump::to_qimage() const{
	//Make gamma LUT
	if( !gamma )
		generate_gamma();
	
	//Initialize planes
	vector<Plane> planes;
	const char* plane_start = data;
	
	//Keep reading planes until we reach the end of the input
	while( plane_start < data + size ){
		uint32_t* properties = (uint32_t*)plane_start;
		Plane plane( (char*)(properties + 3) );
		
		//Read properties
		//TODO: have alternative implementation for big-endian
		plane.width = properties[0];
		plane.height = properties[1];
		plane.depth = properties[2];
		
		plane_start = plane.end();
		planes.push_back( plane );
	}
	qDebug( "Amount of planes: %d", (int)planes.size() );
	
	//TODO: check that we have all the data
	//TODO: do sanity-checks
	if( planes.size() != 3 )
		return QImage();
	
	//QImage output
	uint32_t width = planes[0].width;
	uint32_t height = planes[0].height;
	
	QImage output( width, height, QImage::Format_RGB32 );
	output.fill( 0 );
	
	double chroma_stride_x = (double)planes[1].width / planes[0].width;
	double chroma_stride_y = (double)planes[1].height / planes[0].height;
	
	//Simple Y' output
	for( uint64_t iy=0; iy<height; iy++ ){
		unsigned chroma_y = iy * chroma_stride_y;
		QRgb* out = (QRgb*)output.scanLine( iy );
		const unsigned char* row1 = (unsigned char*)planes[0].scanline( iy );
		const unsigned char* row2 = (unsigned char*)planes[1].scanline( chroma_y );
		const unsigned char* row3 = (unsigned char*)planes[2].scanline( chroma_y );
		
		for( uint64_t ix=0; ix<width; ix++ ){
			uint16_t r, g, b;
			unsigned chroma_x = ix * chroma_stride_x;
			r = ( planes[0].byte_count() == 1 ) ? row1[ix] : ((uint16_t*)row1)[ix];
			g = ( planes[1].byte_count() == 1 ) ? row2[chroma_x] : ((uint16_t*)row2)[chroma_x];
			b = ( planes[2].byte_count() == 1 ) ? row3[chroma_x] : ((uint16_t*)row3)[chroma_x];
			
			yuv_to_rgb( r, g, b, (1<<planes[0].depth)-1, 0.2126, 0.7152, 0.0722 );
			
			out[ix] = qRgb( r/256, g/256, b/256 );
		}
	}
	return output;
}
