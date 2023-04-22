#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "stdtype.h"
#include "fat32.h"
#include "framebuffer.h"
#include "string.h"
#include "stdmem.h"

struct CurrentWorkingDirectory
{
    uint32_t clusters_stack[CLUSTER_MAP_SIZE];
    char dir_names[CLUSTER_MAP_SIZE][8];
    int top;
} __attribute__((packed));

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);
void puts(char *buf, enum vga_color fg);
int8_t read_path(char *relative_path, struct CurrentWorkingDirectory *cwd, char *to_find);

#endif