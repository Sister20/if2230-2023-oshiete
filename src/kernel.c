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

void kernel_setup(void)
{
    // enter_protected_mode(&_gdt_gdtr);
    // pic_remap();
    // initialize_idt();
    // framebuffer_clear();
    // splash();
    // framebuffer_set_cursor(15, 0);
    // activate_keyboard_interrupt();
    // initialize_filesystem_fat32();

    // allocate_single_user_page_frame((void *)0x0);
    // *((uint8_t*) 0x500000) = 1;

    // struct ClusterBuffer cbuf[5];
    // for (uint32_t i = 0; i < 5; i++)
    //     for (uint32_t j = 0; j < CLUSTER_SIZE; j++)
    //         cbuf[i].buf[j] = i + 'a';

    // struct FAT32DriverRequest request = {
    //     .buf                   = cbuf,
    //     .name                  = "ikanaide",
    //     .ext                   = "uwu",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    //     .buffer_size           = 0,
    // };

    // write(request);  // Create folder "ikanaide"
    // memcpy(request.name, "kano1\0\0\0", 8);
    // write(request);  // Create folder "kano1"
    // memcpy(request.name, "ikanaide", 8);
    // memcpy(request.ext, "\0\0\0", 3);
    // delete(request); // Delete first folder, thus creating hole in FS

    // memcpy(request.name, "daijoubu", 8);
    // request.buffer_size = 5*CLUSTER_SIZE;
    // write(request);  // Create fragmented file "daijoubu"

    // struct ClusterBuffer readcbuf;
    // read_clusters(&readcbuf, ROOT_CLUSTER_NUMBER+1, 1);
    // // If read properly, readcbuf should filled with 'a'

    // request.buffer_size = CLUSTER_SIZE;
    // read(request);   // Failed read due not enough buffer size
    // request.buffer_size = 5*CLUSTER_SIZE;
    // read(request);   // Success read on file "daijoubu"

    // while (TRUE) {
    //     keyboard_state_activate();
    // }

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
    for (uint32_t i = 0; i < 1; i++)
        for (uint32_t j = 0; j < 20; j++)
            cbuf[i].buf[j] = i + 'a';

    // struct FAT32DriverRequest request2 = {
    //     .buf                   = cbuf,
    //     .name                  = "ikanaide",
    //     .ext                   = "uwu",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    //     .buffer_size           = 2 * CLUSTER_SIZE,
    // };

    // write(request2);  // Create file "ikanaide"

    struct FAT32DriverRequest request3 = {
        .buf = cbuf,
        .name = "lol",
        .ext = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = 0,
    };

    write(request3);

    struct FAT32DriverRequest request4 = {
        .buf = cbuf,
        .name = "lolsss",
        .ext = "txt",
        .parent_cluster_number = 10,
        .buffer_size = CLUSTER_SIZE,
    };

    write(request4);

    // struct FAT32DriverRequest request5 = {
    //     .buf = cbuf,
    //     .name = "sleep",
    //     .ext = "\0\0\0",
    //     .parent_cluster_number = 10,
    //     .buffer_size = CLUSTER_SIZE,
    // };

    // write(request5);

    // struct FAT32DriverRequest request6 = {
    //     .buf = cbuf,
    //     .name = "ktl",
    //     .ext = "ktl",
    //     .parent_cluster_number = 10,
    //     .buffer_size = CLUSTER_SIZE,
    // };

    // write(request6);

    // struct FAT32DriverRequest request7 = {
    //     .buf = cbuf,
    //     .name = "ktlsss",
    //     .ext = "ktl",
    //     .parent_cluster_number = 10,
    //     .buffer_size = CLUSTER_SIZE,
    // };

    // write(request7);

    // Set TSS $esp pointer and jump into shell
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t *)0);

    while (TRUE)
        ;
}
