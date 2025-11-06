#include "../ZL_PlatformConfig.h"
#ifdef ZL_HAS_NATIVEZLIB
#include <zlib.h>
#else
#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"
#undef MINIZ_HEADER_FILE_ONLY
#endif
