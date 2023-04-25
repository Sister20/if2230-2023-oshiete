#ifndef _MV_H
#define _MV_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void mv(struct CurrentWorkingDirectory cwd, char* src, char* dest);

#endif