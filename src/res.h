/*
 *  res-packager - Simple file packager
 *  Copyright (C) 2019 A. Roldán
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * 	arldn.dev -at- gmail.com
 */

#ifndef _respackager_h_
#define _respackager_h_

#include <stdint.h>

struct _res_header {
    uint32_t    flags;
    uint32_t    fatSize;
    char        none[8];
};

struct _res_fat_entry {
    uint32_t    offset;
    uint32_t    length;
    char        filename[16];
};

struct _res_fat {
    struct _res_fat_entry *entries;
};

typedef struct _res {
    struct _res_header header;
    struct _res_fat fat;
    char    **files;

    int     errors;
    int     size;
} RES;

int res_load(RES* res, const char* filename);

void res_dispose(RES* res);

int res_extract(RES* res, uint32_t index, const char* path);

char *res_get_file(RES* res, uint32_t index);

int res_create(RES* res, const char* filename, const char* path);

int res_save(RES* res, const char* filename);

char *res_fetch_file(const char *filename, const char *name, uint32_t* length);

#endif  // _respackager_h_