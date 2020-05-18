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

#include "res.h"
#include "../../engine/src/defs.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

int res_save(RES* res, const char* filename)    {
    FILE* file;
    int i;

    file = fopen(filename, "wb");
    if (!file) {
#ifdef __RES_DEBUG__
        printf("res_save(): error saving RES file %s.\n", filename);
#endif
        return -1;
    }

    fwrite(&res->header, sizeof(struct _res_header), 1, file);

    for (i = 0; i < res->header.fatSize; i++) {
        fwrite(&res->fat.entries[i], sizeof(struct _res_fat_entry), 1, file);
    }

    for (i = 0; i < res->header.fatSize; i++) {
        fwrite(res->files[i], res->fat.entries[i].length, 1, file);
    }

    res->size = ftell(file);

    fclose(file);

    return 0;
}

int res_load(RES* res, const char* filename)  {
    FILE* file;
    int filesize;
    int i;

    file = fopen(filename, "rb");
    if (!file) {
#ifdef __RES_DEBUG__
        printf("res_load(): error loading RES file %s.\n", filename);
#endif
        return -1;
    }

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read header
    fread(&res->header, sizeof(struct _res_header), 1, file);

    // Create mem and FAT
    res->fat.entries = (struct _res_fat_entry*)malloc(sizeof(struct _res_fat_entry) * res->header.fatSize);
    for (i = 0; i < res->header.fatSize; i++) {
        fread(&res->fat.entries[i], sizeof(struct _res_fat_entry), 1, file);
    }

    // Check if FAT is valid
    const uint32_t expectedSize = res->fat.entries[res->header.fatSize - 1].offset + res->fat.entries[res->header.fatSize - 1].length;
    if (expectedSize != filesize) {
        fclose(file);
#ifdef __RES_DEBUG__
        printf("res_load(): corrupt RES file %d/%d.\n", expectedSize, filesize);
#endif
        return -1;
    }

    // Read files
    res->files = (char**)malloc(sizeof(char*) * res->header.fatSize);
    for (i = 0; i < res->header.fatSize; i++) {
        res->files[i] = (char*)malloc(sizeof(char) * res->fat.entries[i].length);
        fseek(file, res->fat.entries[i].offset, SEEK_SET);
        fread(res->files[i], res->fat.entries[i].length, 1, file);
    }

    fclose(file);

    return 0;
}

void res_dispose(RES* res)  {
    int i;

    for (i = 0; i < res->header.fatSize; i++) {
        if (res->fat.entries[i].length != 0) {
            free(res->files[i]);
        }
    }
    free(res->files);

    free(res->fat.entries);
}

int res_extract(RES* res, uint32_t index, const char* path)   {

    struct _res_fat_entry* entry = NULL;
    FILE* file = NULL;
    char filename[256];

    if (res->header.fatSize <= index) {
#ifdef __RES_DEBUG__
        printf("res_extract(): invalid file index %d.\n", index);
#endif
        return -1;
    }

    entry = &res->fat.entries[index];

    strcpy(filename, path);
    strcat(filename, entry->filename);

    file = fopen(filename, "wb");
    if (!file) {
#ifdef __RES_DEBUG__
        printf("res_extract(): error creating output file %s.\n", entry->filename);
#endif
        return -1;
    }

    fwrite(res->files[index], res->fat.entries[index].length, 1, file);

    fclose(file);

    return 0;
}

char *res_get_file(RES* res, uint32_t index)    {
    if (res->header.fatSize <= index) {
#ifdef __RES_DEBUG__
        printf("res_get_file(): invalid file index %d.\n", index);
#endif
        return NULL;
    }

    return res->files[index];
}

char *res_fetch_file(const char *filename, const char* name, uint32_t *length)  {
    FILE* file;
    char *data = NULL;
    struct _res_header header;
    struct _res_fat fat;
    int i, filesize;
    BOOL found = FALSE;

    *length = 0;

    file = fopen(filename, "rb");
    if (!file) {
#ifdef __RES_DEBUG__
        printf("res_fetch_file(): error opening resources source file %s.\n", filename);
#endif
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read header
    fread(&header, sizeof(struct _res_header), 1, file);

    // Create mem and FAT
    fat.entries = (struct _res_fat_entry*)malloc(sizeof(struct _res_fat_entry) * header.fatSize);
    for (i = 0; i < header.fatSize; i++) {
        fread(&fat.entries[i], sizeof(struct _res_fat_entry), 1, file);
    }

    // Check if FAT is valid
    const uint32_t expectedSize = fat.entries[header.fatSize - 1].offset + fat.entries[header.fatSize - 1].length;
    if (expectedSize != filesize) {
        fclose(file);
#ifdef __RES_DEBUG__
        printf("res_load(): corrupt RES file %d/%d.\n", expectedSize, filesize);
#endif
        return NULL;
    }

    // Fetch file
    for (i = 0; i < header.fatSize; i++) {
        if (strcmp(name, fat.entries[i].filename) == 0) {
            data = (char*)malloc(sizeof(char) * fat.entries[i].length);
            fseek(file, fat.entries[i].offset, SEEK_SET);
            fread(data, fat.entries[i].length, 1, file);

            *length = fat.entries[i].length;
            found = TRUE;

            break;
        }
    }

    fclose(file);

    if (!found) {
#ifdef __RES_DEBUG__
        printf("res_load(): file not found in FAT\n");
#endif
        return NULL;
    }

    return data;
}

int res_create(RES* res, const char* filename, const char* path)  {
    FILE* file;
    FILE* resourceFile;
    uint32_t currentOffset;
    struct _res_fat_entry *entry = NULL;
    const uint32_t maxBufLength = 256;
    char buf[maxBufLength];
    int i;
    char finalPath[256];

    file = fopen(filename, "rb");
    if (!file) {
#ifdef __RES_DEBUG__
        printf("res_create(): error opening resources source file %s.\n", filename);
#endif
        return -1;
    }

    currentOffset = sizeof(struct _res_header);

    char files[512][256];
    uint32_t currentFile = 0;

    // Lee el nombre de los ficheros
    while(fgets(buf, maxBufLength, file)) {
        if (buf[0] != '#' && buf[0] != 32 && buf[0] != '\n') {
            // Elimina el último intro
            if(buf[strlen(buf)-1] == '\n') buf[strlen(buf) - 1] = '\0';
            strcpy(files[currentFile], buf);
            currentFile++;
        }
    }

    fclose(file);

    // Empty header
    res->header.flags = 0;
    res->header.fatSize = 0;
    memset(res->header.none, 0, 8);

    res->errors = 0;

    // Conocemos la cantidad de ficheros, creamos la información para la FAT
    res->header.fatSize = currentFile;
    res->fat.entries = (struct _res_fat_entry*)malloc(sizeof(struct _res_fat_entry) * res->header.fatSize);
    res->files = (char**)malloc(sizeof(char*) * res->header.fatSize);

    currentOffset += sizeof(struct _res_fat_entry) * res->header.fatSize;

    for (i = 0; i < currentFile; i++) {

        strcpy(finalPath, path);
        strcat(finalPath, files[i]);

        entry = &res->fat.entries[i];

        printf("Packaging %s...", finalPath);

        resourceFile = fopen(finalPath, "rb");
        if (resourceFile) {

            // Set length file
            fseek(resourceFile, 0, SEEK_END);
            entry->length = ftell(resourceFile);
            fseek(resourceFile, 0, SEEK_SET);

            // Set offset
            entry->offset = currentOffset;
            currentOffset += entry->length;

            // Set filename
            memset(entry->filename, 0, 16);
            strcpy(entry->filename, files[i]);

            // Read file
            res->files[i] = (char*)malloc(sizeof(char) * entry->length);
            fread(res->files[i], entry->length, 1, resourceFile);

            fclose(resourceFile);

            printf("[DONE]\n");
        }
        else {
            printf("[ERROR]\n");
#ifdef __RES_DEBUG__
            printf("res_create(): error opening resource file %s.\n", files[i]);
#endif
            res->errors++;
            entry->length = 0;
            entry->offset = currentOffset;
        }
    }

    return 0;
}