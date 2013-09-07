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

#include "image.h"

#include <QRect>

image::image( char* img_data, unsigned size ){
	width = ((unsigned*)img_data)[0];
	height = ((unsigned*)img_data)[1];
	unsigned depth = ((unsigned*)img_data)[2];
	
	data = new color[ height * width ];
	
	unsigned char* img_contents = (unsigned char*)((unsigned*)img_data + 3);
	for( unsigned iy=0; iy<height; iy++ ){
		color* row = scan_line( iy );
		for( unsigned ix=0; ix<width; ++ix, ++row ){
			unsigned short c=127*255;
			c = *img_contents;
			img_contents++;
			c <<= 6;
			*row = color( c,c,c );
		}
	}
}

image::~image(){
	qDebug( "deleting image %p", this );
	if( data )
		delete[] data;
}

