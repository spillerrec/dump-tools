## shell-dump-extension

Provides thumbnails in Windows file explorer. Windows Vista/7/8/8.1 only.

### Installation

To install you need to manually modify your registry. Add the following to register the component:

```
HKEY_CLASSES_ROOT
 |-- .dump
 |    |-- ShellEx
 |    |    |-- {E357FCCD-A995-4576-B01F-234630154E96}
 |    |    |    |-- Key:REG_SZ  (Default) = {98E669D7-CD64-47DD-9111-5DEB438FC7E0}
 |
 |-- CLSID
 |    |-- {98E669D7-CD64-47DD-9111-5DEB438FC7E0}
 |    |    |-- InprocServer32
 |    |    |    |-- Key:REG_SZ  (Default) = "Full path to DLL file"
```

*{E357FCCD-A995-4576-B01F-234630154E96}* is the thumbnailer interface. {98E669D7-CD64-47DD-9111-5DEB438FC7E0} is the actual dump thumbnailer implementation.

To enable the component, either restart *explorer.exe* or run a program which calls the following function:

```C++
SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0 );
```

### Building

**Dependencies**

- Windows SDK
- Visual Studio 2012
- zlib

**Building**

Ensure that zlib is properly linked. Static linking was used, dynamic linking might work, don't know where the zlib DLL must be located though. Don't know enough about Visual Studio to provide better advise.

Make sure to select the same platform as your system, if you are running 64-bit Windows, compile a 64-bit DLL. A 32-bit DLL will not work.
