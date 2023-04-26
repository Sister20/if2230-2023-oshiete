#ifndef _TOUCH_H
#define _TOUCH_H

#include "../stdtype.h"
#include "../syscall.h"
#include "../fat32.h"
#include "../framebuffer.h"
#include "../string.h"
#include "../stdmem.h"

void touch(struct CurrentWorkingDirectory cwd, char command[12][128]);

#endif