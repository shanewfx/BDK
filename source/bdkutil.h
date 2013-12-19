#ifndef BASE_DEV_KIT_UTIL_H
#define BASE_DEV_KIT_UTIL_H

namespace BDK {

const char*    new_guid();
const wchar_t* new_guid_w();

const char*    wcs2mbs(const wchar_t* wideString);

}

#endif//BASE_DEV_KIT_UTIL_H