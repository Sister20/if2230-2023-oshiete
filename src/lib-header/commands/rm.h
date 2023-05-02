#ifndef _RM_H
#define _RM_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void rm(struct CurrentWorkingDirectory cwd, char* src, int8_t is_recursive, int8_t is_root_command);

#endif