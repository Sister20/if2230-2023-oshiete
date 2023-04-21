#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "stdtype.h"
#include "fat32.h"
#include "framebuffer.h"
#include "string.h"

struct Cwd
{
    uint32_t clusters_stack[CLUSTER_MAP_SIZE];
    char dir_names[CLUSTER_MAP_SIZE][8];
    int top;
    char *curr_dir;
} __attribute__((packed));

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);
void puts(char *buf, enum vga_color fg);

#endif