#include <stdio.h>
#include <tchar.h>

#include "..\source\bdkutil.h"

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\bin\\BDK_d.lib")
#else
#pragma comment(lib, "..\\..\\bin\\BDK.lib")
#endif

int _tmain(int argc, _TCHAR* argv[])
{
    printf("guid 0: %s\n", BDK::new_guid());
    printf("guid 1: %s\n", BDK::new_guid());
    printf("guid 2: %S\n", BDK::new_guid_w());
    printf("guid 3: %ls\n", BDK::new_guid_w());
    printf("guid 4: %s\n", BDK::wcs2mbs(BDK::new_guid_w()));
	return 0;
}

