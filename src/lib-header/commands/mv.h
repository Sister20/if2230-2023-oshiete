#ifndef _LS_H
#define _LS_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void mv(struct Cwd cwd, struct FAT32DriverRequest *request, int32_t *retcode, char* src, char* dest);

#endif