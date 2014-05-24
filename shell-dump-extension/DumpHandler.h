#pragma once

#include <Thumbcache.h>
#include <Propsys.h>

#include <stdint.h>
#include <vector>

#include "DumpPlane.hpp"

class DumpHandler : public IThumbnailProvider, public IInitializeWithStream{
	public:
		//IUnknown
		virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv ); 
		virtual ULONG __stdcall AddRef();
		virtual ULONG __stdcall Release();

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
};

