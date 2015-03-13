#include "DumpHandler.h"

#include <zlib.h>

#include <iostream>
#include <algorithm>

DumpHandler::DumpHandler() : ref_count( 0 ), loaded( false )
{
	
}


DumpHandler::~DumpHandler(){

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
	if( iid == IID_IUnknown || iid == IID_IThumbnailProvider )
		*ppv = static_cast<IThumbnailProvider*>(this);
	else if( iid == IID_IInitializeWithStream )
		*ppv = static_cast<IInitializeWithStream*>(this);
	else if( iid == IID_IWICBitmapDecoder )
		*ppv = static_cast<IWICBitmapDecoder*>(this);
	else if( iid == IID_IWICBitmapFrameDecode )
		*ppv = static_cast<IWICBitmapFrameDecode*>(this);
	else if( iid == IID_IWICBitmapSource )
		*ppv = static_cast<IWICBitmapSource*>(this);
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

HRESULT __stdcall DumpHandler::QueryCapability( IStream *pIStream, DWORD *pdwCapabilities ) {
	//TODO: check stream for sane size
	*pdwCapabilities = WICBitmapDecoderCapabilityCanDecodeAllImages;
	return S_OK;
}

HRESULT __stdcall DumpHandler::Initialize( IStream *pIStream, WICDecodeOptions ) {
	return Initialize( pIStream, 0 );
}

HRESULT __stdcall DumpHandler::GetContainerFormat( GUID *pguidContainerFormat ) {
	*pguidContainerFormat = CLSID_DumpHandler;
	return S_OK;
}

HRESULT __stdcall DumpHandler::GetDecoderInfo( IWICBitmapDecoderInfo **pIDecoderInfo ) {
	//TODO: 
	IWICComponentInfo* pComponentInfo = NULL;

	IWICImagingFactory *pFactory = nullptr;
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		(LPVOID*)&pFactory
		);
	hr = pFactory->CreateComponentInfo( CLSID_DumpHandler, &pComponentInfo );
	pFactory->Release();

	hr = pComponentInfo->QueryInterface( IID_IWICBitmapDecoderInfo, (void**)pIDecoderInfo );
	return hr;
}

HRESULT __stdcall DumpHandler::GetFrameCount( UINT *pCount ) {
	*pCount = 1;
	return S_OK;
}

HRESULT __stdcall DumpHandler::GetFrame( UINT index, IWICBitmapFrameDecode **ppIBitmapFrame ) {
	if( index != 0 )
		return S_FALSE;

	*ppIBitmapFrame = this;
	return S_OK;
}


HRESULT __stdcall DumpHandler::GetThumbnail( IWICBitmapSource **ppIThumbnail ) {
	UNREFERENCED_PARAMETER( ppIThumbnail );
	return WINCODEC_ERR_CODECNOTHUMBNAIL;
}

HRESULT __stdcall DumpHandler::GetColorContexts( UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount ) {
	*pcActualCount = 1;
	if( cCount == 0 && ppIColorContexts == nullptr )
		return S_OK;

	IWICImagingFactory *pFactory = nullptr;
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		(LPVOID*)&pFactory
		);
	pFactory->CreateColorContext( ppIColorContexts );
	pFactory->Release();

	(*ppIColorContexts)[0].InitializeFromExifColorSpace( 1 ); //TODO: actually provide a YUV one

	return S_OK;
}

HRESULT __stdcall DumpHandler::GetMetadataQueryReader( IWICMetadataQueryReader **ppIMetadataQueryReader ) {
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}


HRESULT __stdcall DumpHandler::GetSize( UINT *puiWidth, UINT *puiHeight ) {
	*puiWidth = *puiHeight = 0;
	for( auto& plane : planes ) {
		*puiWidth  = (std::max)( *puiWidth,  plane.width );
		*puiHeight = (std::max)( *puiHeight, plane.height );
	}
	return S_OK;
}

bool DumpHandler::is16Bit() const {
	for( auto& plane : planes )
		if( plane.depth > 8 )
			return true;
	return false;
}

HRESULT __stdcall DumpHandler::GetPixelFormat( WICPixelFormatGUID *pPixelFormat ) {
	*pPixelFormat = GUID_WICPixelFormatUndefined;
	switch( planes.size() ) {
		case 0:
		case 2: //Gray with alpha, don't know how to represent
			return S_FALSE;

		case 1:
			*pPixelFormat = is16Bit() ? GUID_WICPixelFormat16bppGray : GUID_WICPixelFormat8bppGray;
			return S_OK;

		case 3:
			*pPixelFormat = is16Bit() ? GUID_WICPixelFormat48bppRGB : GUID_WICPixelFormat24bppRGB;
			return S_OK;

		case 4:
			*pPixelFormat = is16Bit() ? GUID_WICPixelFormat64bppRGBA : GUID_WICPixelFormat32bppRGBA;
			return S_OK;

		default:
			return S_OK;
	}
}

HRESULT __stdcall DumpHandler::CopyPixels( const WICRect *prc, UINT cbStride, UINT cbBufferSize, BYTE *pbBuffer ) {
	//Find area to copy
	UINT width, height;
	GetSize( &width, &height );
	WICRect rect = { 0, 0, width, height };
	if( prc )
		rect = *prc;

	auto write = [&]( Plane& p, int offset ) {
		auto scale_x = (double)p.width / width;
		auto scale_y = (double)p.height / height;
		
		for( int iy = 0; iy<rect.Height; iy++ )
			for( int ix = 0; ix<rect.Width; ix++ ) {
				auto x = (int)(scale_x * (ix+rect.X));
				auto y = (int)(scale_y * (iy+rect.Y));
				auto pix = p.byte_count() == 2 ? ((uint16_t*)p.scanline( y ))[x] >> 2 : p.scanline( y )[x];
				pbBuffer[iy*cbStride + ix*3 + offset] = pix;
				//TODO: more!
			}
	};

	//Copy channels
	for( int i = 0; i<3; i++ )
		write( planes[i], i );

	//Translate YUV
	for( int iy = 0; iy<rect.Height; iy++ )
		for( int ix = 0; ix<rect.Width; ix++ ) {
			uint16_t r = pbBuffer[iy*cbStride + ix*3 + 0];
			uint16_t g = pbBuffer[iy*cbStride + ix*3 + 1];
			uint16_t b = pbBuffer[iy*cbStride + ix*3 + 2];
			yuv_to_rgb( r, g, b, (1<<8)-1, 0.2126, 0.7152, 0.0722 );
			pbBuffer[iy*cbStride + ix*3 + 0] = r;
			pbBuffer[iy*cbStride + ix*3 + 1] = g;
			pbBuffer[iy*cbStride + ix*3 + 2] = b;
		}

	return S_OK;
}


#define BUFFER_SIZE 1000
HRESULT __stdcall DumpHandler::Initialize( IStream *pstream, DWORD grfMode ){
	UNREFERENCED_PARAMETER(grfMode);
	ULONG read_bytes;

	while( true ){
		Plane plane;
		if( !plane.read( pstream ) )
			break;
		planes.emplace_back( plane );
	}

	std::cout << "Planes amount: " << planes.size() << "\n";
	return (planes.size() == 3) ? S_OK : S_FALSE;
}
