//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	WAD I/O functions.
//

#include <stdio.h>

#include "config.h"

#include "doom1_wad.h"
#include "doomtype.h"
#include "w_file.h"

#include <string.h>

static wad_file_class_t doom1_wad_file_class = {W_OpenFile, W_CloseFile, W_Read};

static wad_file_t doom1_wad_file = {&doom1_wad_file_class, (byte*)doom1_wad, sizeof(doom1_wad)};

wad_file_t *W_OpenFile(char *path)
{
    if (strcmp(path, "doom1.wad") == 0)
    {
        return &doom1_wad_file;
    }
    else
    {
        return NULL;
    }
}

void W_CloseFile(wad_file_t *wad)
{
    (void)wad;
}

size_t W_Read(wad_file_t *wad, unsigned int offset,
              void *buffer, size_t buffer_len)
{
    if (offset > sizeof(doom1_wad))
    {
        return 0;
    }

    if (offset + buffer_len > sizeof(doom1_wad))
    {
        buffer_len = sizeof(doom1_wad) - offset;
    }

    memcpy(buffer, &doom1_wad[offset], buffer_len);

    return buffer_len;
}

