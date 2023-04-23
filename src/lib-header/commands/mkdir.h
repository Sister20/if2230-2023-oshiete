#ifndef _MKDIR_H
#define _MKDIR_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void mkdir(struct CurrentWorkingDirectory cwd, char *folder_name);

#endif