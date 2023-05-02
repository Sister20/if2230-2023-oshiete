#include "lib-header/portio.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/gdt.h"
#include "lib-header/framebuffer.h"
#include "lib-header/kernel_loader.h"
#include "lib-header/splash.h"
#include "lib-header/fat32.h"
#include "lib-header/cmos.h"
#include "lib-header/idt.h"
#include "lib-header/interrupt.h"
#include "lib-header/keyboard.h"
#include "lib-header/paging.h"
#include "lib-header/string.h"

void kernel_setup(void)
{
    enter_protected_mode(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

    // Allocate first 4 MiB virtual memory
    allocate_single_user_page_frame((uint8_t *)0);

    // Write shell into memory
    struct FAT32DriverRequest request = {
        .buf = (uint8_t *)0,
        .name = "shell",
        .ext = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = 0x100000,
    };
    uint32_t found_cluster_number;
    read(request, &found_cluster_number);

    struct ClusterBuffer cbuf[1];

    struct FAT32DriverRequest createFolder = {
        .buf = cbuf,
        .name = "oshiete",
        .ext = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = 0,
    };

    write(createFolder);

    struct ClusterBuffer stringBuffer[1];
    char* text = "Team Oshiete :\n13521044 Rachel Gabriela Chen\n13521046 Jeffrey Chow\n13521052 Melvin Kent Jonathan\n13521114 Farhan Nabil Suryono";

    for (uint32_t i = 0; i < 1; i++){
        for (uint32_t j = 0; j < strlen(text); j++){
            stringBuffer[i].buf[j] = i + text[j];
        }
    }

    struct FAT32DriverRequest createFile = {
        .buf = stringBuffer,
        .name = "oshiete",
        .ext = "os",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = sizeof(stringBuffer),
    };

    write(createFile);

    // Set TSS $esp pointer and jump into shell
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t *)0);

    while (TRUE);
}
