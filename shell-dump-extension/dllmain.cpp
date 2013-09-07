// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include "ClassFactory.h"
#include <new>

// {98E669D7-CD64-47DD-9111-5DEB438FC7E0}
const GUID CLSID_DumpHandler = 
{ 0x98e669d7, 0xcd64, 0x47dd, { 0x91, 0x11, 0x5d, 0xeb, 0x43, 0x8f, 0xc7, 0xe0 } };



BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved ){
	switch( ul_reason_for_call ){
		case DLL_PROCESS_ATTACH:
				DisableThreadLibraryCalls( hModule );
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

HRESULT __stdcall DllGetClassObject( REFCLSID objGuid, REFIID riid, void **ppv ){
	*ppv = NULL;
	
	if( IsEqualCLSID( objGuid, CLSID_DumpHandler ) ){
		ClassFactory *factory = new (std::nothrow) ClassFactory();
		if( !factory )
			return E_OUTOFMEMORY;

		factory->AddRef();
		HRESULT hr = factory->QueryInterface( riid, ppv );
		factory->Release();
		return hr;
	}
	else
		return CLASS_E_CLASSNOTAVAILABLE;
}

HRESULT __stdcall DllCanUnloadNow(){
	return (ClassFactory::dll_ref>0) ? S_FALSE : S_OK;
}

