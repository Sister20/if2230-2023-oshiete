#ifndef _LS_H
#define _LS_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void ls(struct Cwd cwd, struct FAT32DriverRequest *request, int32_t *retcode);

#endif