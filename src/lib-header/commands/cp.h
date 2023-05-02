#ifndef _CP_H
#define _CP_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void cp(struct CurrentWorkingDirectory cwd, char* src, char* dest, int8_t is_recursive, int8_t is_root_command);
#endif