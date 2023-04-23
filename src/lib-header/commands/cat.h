#ifndef _CAT_H
#define _CAT_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void cat(struct CurrentWorkingDirectory cwd, char *file_path);

#endif