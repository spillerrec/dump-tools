#include "DumpHandler.h"

#include <zlib.h>

#include <iostream>

DumpHandler::DumpHandler() : ref_count( 0 ), loaded( false )
{
	
}


DumpHandler::~DumpHandler(){
	if( planes.size() > 0 ){
		for( Plane p : planes )
			if( p.data )
				delete[] p.data;
		planes.clear();
	}
}


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
	r = uint16_t( rr * 255 + 0.5 );
	g = uint16_t( rg * 255 + 0.5 );
	b = uint16_t( rb * 255 + 0.5 );
}


template<typename T>
T read( IStream *pstream, bool &success ){
	//TODO: have alternative implementation for big-endian
	ULONG read_bytes;
	T temp;
	success &= S_OK == pstream->Read( &temp, sizeof(T), &read_bytes );
	return temp;
}



HRESULT __stdcall DumpHandler::QueryInterface( const IID& iid, void** ppv ){
	if( iid == IID_IUnknown || iid == IID_IThumbnailProvider ){
		*ppv = static_cast<IThumbnailProvider*>(this);
	}
	else if( iid == IID_IInitializeWithStream ){
		*ppv = static_cast<IInitializeWithStream*>(this);
	}
	else{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG __stdcall DumpHandler::AddRef(){
	return InterlockedIncrement( &ref_count );
}

ULONG __stdcall DumpHandler::Release(){
	if( InterlockedDecrement( &ref_count ) == 0 )
		delete this;
	return ref_count;
}


#include <algorithm>
HRESULT __stdcall DumpHandler::GetThumbnail( UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha ){
	HRESULT result = S_FALSE;
//	if( !loaded )
//		return S_FALSE;

	//Scale size
	uint32_t width = planes[0].width;
	uint32_t height = planes[0].height;
	double scale = (double)cx / (double)max( width, height );
	width *= scale;
	height *= scale;
	std::cout << "Size: " << width << "x" << height << "\n";

	//Create bitmap
	HDC hdc = ::GetDC( NULL );
   void *bits = 0;
   BITMAPINFO bmi = {0};
   bmi.bmiHeader.biWidth = width;
   bmi.bmiHeader.biHeight = height;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 32;
   bmi.bmiHeader.biSizeImage = 0;
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biClrUsed = 0;
   bmi.bmiHeader.biClrImportant = 0;
	
	
	HBITMAP bmp = CreateDIBSection( hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0 );
	if( bmp ){
		//Switch to drawing on our bitmap
		HDC memDC = CreateCompatibleDC( hdc );
		if( !memDC )
			return S_FALSE;
		HGDIOBJ oldbmp = SelectObject( memDC, bmp );
		
		double luma_stride_x = (double)planes[0].width / width;
		double luma_stride_y = (double)planes[0].height / height;
		double chroma_stride_x = (double)planes[1].width / width;
		double chroma_stride_y = (double)planes[1].height / height;

		for( uint64_t iy=0; iy<height; iy++ ){
			unsigned luma_y = iy * luma_stride_y + 0.5;
			unsigned chroma_y = iy * chroma_stride_y + 0.5;
			const unsigned char* row1 = (unsigned char*)planes[0].scanline( luma_y );
			const unsigned char* row2 = (unsigned char*)planes[1].scanline( chroma_y );
			const unsigned char* row3 = (unsigned char*)planes[2].scanline( chroma_y );
		
			for( uint64_t ix=0; ix<width; ix++ ){
				uint16_t r, g, b;
				unsigned luma_x = ix * luma_stride_x + 0.5;
				unsigned chroma_x = ix * chroma_stride_x + 0.5;
				r = ( planes[0].byte_count() == 1 ) ? row1[luma_x] : ((uint16_t*)row1)[luma_x];
				g = ( planes[1].byte_count() == 1 ) ? row2[chroma_x] : ((uint16_t*)row2)[chroma_x];
				b = ( planes[2].byte_count() == 1 ) ? row3[chroma_x] : ((uint16_t*)row3)[chroma_x];
			
				yuv_to_rgb( r, g, b, (1<<planes[0].depth)-1, 0.2126, 0.7152, 0.0722 );
			
				SetPixel( memDC, ix, iy, RGB( r, g, b ) );
			}
		}
		*phbmp = bmp;

		SelectObject( memDC, oldbmp );
		DeleteDC( memDC );
	}
	ReleaseDC( NULL, hdc );


	return S_OK;
}

#define BUFFER_SIZE 1000
HRESULT __stdcall DumpHandler::Initialize( IStream *pstream, DWORD grfMode ){
	UNREFERENCED_PARAMETER(grfMode);
	ULONG read_bytes;

	bool still_has_data = true;
	while( still_has_data ){
		//Read properties
		Plane plane;
		plane.width    = read<uint32_t>( pstream, still_has_data );
		plane.height   = read<uint32_t>( pstream, still_has_data );
		plane.depth    = read<uint8_t>( pstream, still_has_data );
		plane.reserved = read<uint8_t>( pstream, still_has_data );
		plane.config   = read<uint16_t>( pstream, still_has_data );
		if( !still_has_data )
			break;

		uLongf uncompressed = plane.size();
		if( plane.config & 0x1 ){
			//Data is compressed
			uint32_t lenght = read<uint32_t>( pstream, still_has_data );

			//Prepare
			char *raw = new char[ lenght ];
			if( !raw )
				break;
			char *data = new char[ uncompressed ];
			if( !data ){
				delete[] raw;
				break;
			}

			//Read
			still_has_data &=
				S_OK == pstream->Read( raw, lenght, &read_bytes );
			if( !still_has_data ){
				delete[] raw;
				delete[] data;
				break;
			}

			//Uncompress
			if( uncompress( (Bytef*)data, &uncompressed, (Bytef*)raw, lenght ) != Z_OK ){
				still_has_data = false;
				delete[] raw;
				delete[] data;
				break;
			}

			//Assign
			plane.data = data;
		}
		else{
			//Data is uncompressed
			char *data = new char[ uncompressed ];
			if( !data )
				break;
			
			//Read
			still_has_data &=
				S_OK == pstream->Read( data, uncompressed, &read_bytes );
			if( !still_has_data ){
				delete[] data;
				break;
			}
			
			//Assign
			plane.data = data;
		}

		planes.push_back( plane );
	}

	std::cout << "Planes amount: " << planes.size() << "\n";
	return (planes.size() == 3) ? S_OK : S_FALSE;
}
