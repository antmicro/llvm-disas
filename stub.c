//
// Copyright (c) 2019-2021 Antmicro
//
// This file is licensed under the Apache License 2.0.
// Full license text is available in the 'LICENSE' file.
//
#include <stdio.h>

#define CSTUB(x) void x() { printf("WARNING: stub %s\n", __func__); }

//
// TODO: those functions are just stubs
// of zlib and curses functions.
//

CSTUB(setupterm);
CSTUB(tigetnum);
CSTUB(set_curterm);
CSTUB(del_curterm)
CSTUB(uncompress);
CSTUB(crc32);
CSTUB(compress2);
CSTUB(compressBound);
