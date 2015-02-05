#pragma once

#include <Thumbcache.h>
#include <Propsys.h>
#include <Wincodec.h>

#include <stdint.h>
#include <vector>

#include "DumpPlane.hpp"

// {98E669D7-CD64-47DD-9111-5DEB438FC7E0}
const GUID CLSID_DumpHandler =
{ 0x98e669d7, 0xcd64, 0x47dd, { 0x91, 0x11, 0x5d, 0xeb, 0x43, 0x8f, 0xc7, 0xe0 } };

class DumpHandler : public IWICBitmapDecoder, public IWICBitmapFrameDecode, public IThumbnailProvider, public IInitializeWithStream {
	public:
		//IUnknown
		virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv ); 
		virtual ULONG __stdcall AddRef();
		virtual ULONG __stdcall Release();

		//IWICBitmapDecoder
		virtual HRESULT __stdcall QueryCapability( IStream *pIStream, DWORD *pdwCapabilities );
		virtual HRESULT __stdcall Initialize( IStream *pIStream, WICDecodeOptions cacheOptions );
		virtual HRESULT __stdcall GetContainerFormat( GUID *pguidContainerFormat );
		virtual HRESULT __stdcall GetDecoderInfo( IWICBitmapDecoderInfo **pIDecoderInfo );
		virtual HRESULT __stdcall GetFrameCount( UINT *pCount );
		virtual HRESULT __stdcall GetFrame( UINT index, IWICBitmapFrameDecode **ppIBitmapFrame );

		virtual HRESULT __stdcall GetPreview( IWICBitmapSource **ppIBitmapSource ) {
			*ppIBitmapSource = nullptr;
			return S_FALSE;
		}
		virtual HRESULT __stdcall CopyPalette( IWICPalette* ) { return WINCODEC_ERR_PALETTEUNAVAILABLE; }

		//IWICBitmapFrameDecode
		virtual HRESULT GetThumbnail( IWICBitmapSource **ppIThumbnail );
		virtual HRESULT GetColorContexts( UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount );
		virtual HRESULT GetMetadataQueryReader( IWICMetadataQueryReader **ppIMetadataQueryReader );

		//IWICBitmapSource
		virtual HRESULT __stdcall GetSize( UINT *puiWidth, UINT *puiHeight );
		virtual HRESULT __stdcall GetPixelFormat( WICPixelFormatGUID *pPixelFormat );
		virtual HRESULT __stdcall GetResolution( double *pDpiX, double *pDpiY ) {
			*pDpiX = 96.0;
			*pDpiY = 96.0;
			return S_OK;
		}
		virtual HRESULT __stdcall CopyPixels( const WICRect *prc, UINT cbStride, UINT cbBufferSize, BYTE *pbBuffer );

		//IThumbnailProvider
		virtual HRESULT __stdcall GetThumbnail( UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha );

		//IInitializeWithFile
		virtual HRESULT __stdcall Initialize( IStream *pstream, DWORD grfMode );
	
	public:
		DumpHandler();
		~DumpHandler();

	public:


	private:
		bool loaded;
		std::vector<Plane> planes;
		ULONG ref_count;

		bool is16Bit() const;
};

