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

#ifndef COLOR_H
#define COLOR_H


typedef unsigned short color_type;

struct color{
	color_type r;
	color_type b;
	color_type g;
	color_type a;
	
	
	public:
		
	void clear(){
		r = b = g = a = 0;
	}
	color(){
		clear();
	}
	color( color_type r, color_type g, color_type b, color_type a = 255*256 ){
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	color( color* c ){
		r = c->r;
		g = c->g;
		b = c->b;
		a = c->a;
	}
	color& operator+=( const color &rhs ){
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		a += rhs.a;
		return *this;
	}
	
	color& operator-=( const color &rhs ){
		r -= rhs.r;
		g -= rhs.g;
		b -= rhs.b;
		a -= rhs.a;
		return *this;
	}
	
	color& operator+=( const int &rhs ){
		r += rhs;
		g += rhs;
		b += rhs;
		a += rhs;
		return *this;
	}
	
	color& operator*=( const int &rhs ){
		r *= rhs;
		g *= rhs;
		b *= rhs;
		a *= rhs;
		return *this;
	}
	
	color& operator/=( const int &rhs ){
		r /= rhs;
		g /= rhs;
		b /= rhs;
		a /= rhs;
		return *this;
	}
	
	color& operator<<=( const int &rhs ){
		r <<= rhs;
		g <<= rhs;
		b <<= rhs;
		a <<= rhs;
		return *this;
	}
	
	color& operator>>=( const int &rhs ){
		r >>= rhs;
		g >>= rhs;
		b >>= rhs;
		a >>= rhs;
		return *this;
	}
	
	const color operator+( const color &other ) const{
		return color(*this) += other;
	}
	const color operator+( const int &other ) const{
		return color(*this) += other;
	}
	const color operator-( const color &other ) const{
		return color(*this) -= other;
	}
	const color operator*( const int &other ) const{
		return color(*this) *= other;
	}
	const color operator/( const int &other ) const{
		return color(*this) /= other;
	}
	const color operator<<( const int &other ) const{
		return color(*this) <<= other;
	}
	const color operator>>( const int &other ) const{
		return color(*this) >>= other;
	}
};

#endif