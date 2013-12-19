#include "bdkutil.h"
#include <Windows.h>
#include <stdio.h>

namespace BDK {

const char* new_guid()
{
    static char guidBuf[64];
    memset(guidBuf, 0, 64);

    GUID guid;
    CoInitialize(NULL);
    if (S_OK == CoCreateGuid(&guid)) {
        _snprintf(guidBuf, 64,
            "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            guid.Data1,
            guid.Data2,
            guid.Data3,
            guid.Data4[0], guid.Data4[1],
            guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5],
            guid.Data4[6], guid.Data4[7]);
    }
    CoUninitialize();
    return guidBuf;
}

const wchar_t* new_guid_w()
{
    static wchar_t guidBufW[64];
    wcsset(guidBufW, 0);

    const char* guid = new_guid();
    if (guid) {
        MultiByteToWideChar(CP_ACP, NULL, guid, 64, guidBufW, 64);
    }
    return guidBufW;
}

const char* wcs2mbs(const wchar_t* wideString)
{
    static char multiByteBuf[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, wideString, -1, multiByteBuf, MAX_PATH, NULL, NULL);
    return multiByteBuf;
}

}//namespace BDK