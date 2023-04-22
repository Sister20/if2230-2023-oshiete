#ifndef _LS_H
#define _LS_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void ls(struct CurrentWorkingDirectory cwd, struct FAT32DriverRequest *request, char *dir_name);

#endif