#pragma once

#include <Thumbcache.h>
#include <Propsys.h>

#include <stdint.h>
#include <vector>

struct Plane{
	uint32_t width;
	uint32_t height;
	uint8_t depth;
	uint8_t reserved;
	uint16_t config;
	const char* data;
	
	Plane() : data( nullptr ){ }
	
	int byte_count() const{ return (depth-1) / 8 + 1; }
	
	uint64_t size() const{
		return (uint64_t)width * height * byte_count();
	}
	
	const char* scanline( int y ) const{
		return data + (uint64_t)width * y * byte_count();
	}
};

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

