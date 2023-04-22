#ifndef _CD_H
#define _CD_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void cd(struct CurrentWorkingDirectory *cwd, char *dir_name);

#endif