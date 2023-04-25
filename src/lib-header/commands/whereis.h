#ifndef _WHEREIS_H
#define _WHEREIS_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void whereis(char *folder_name);

void searchUtil(struct CurrentWorkingDirectory* cwd, char* folder_name);

#endif