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

void kernel_setup(void) {
    enter_protected_mode(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    framebuffer_clear();

    /* SPLASH SCREEN */
    splash();

    framebuffer_write(3, 8, 'H', 0, 0xF);
    framebuffer_write(3, 9, 'a', 0, 0xF);
    framebuffer_write(3, 10, 'i', 0, 0xF);
    framebuffer_write(3, 11, '!', 0, 0xF);
    framebuffer_set_cursor(15, 8);

    activate_keyboard_interrupt();


    // write_blocks(fs_signature, 0, 1);
    initialize_filesystem_fat32();
    keyboard_state_activate();

    // // uint8_t cbuf[sizeof(struct FAT32DirectoryTable)];

    // struct ClusterBuffer cbuf[5];
    // for (uint32_t i = 0; i < 5; i++)
    //     for (uint32_t j = 0; j < CLUSTER_SIZE; j++)
    //         cbuf[i].buf[j] = i + 'a';

    // // struct FAT32DriverRequest request = {
    // //     .buf = cbuf,
    // //     .name = "melvins",
    // //     .ext = "exe",
    // //     .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    // //     .buffer_size = 128,
    // // };
    // struct FAT32DriverRequest request = {
    //     .buf = cbuf,
    //     .name = "ikaaide",
    //     .ext = "txt",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    //     .buffer_size = 100,
    // };

    // write(request);
    // // read_directory(request);
    // while (TRUE)
    //     ;

    struct ClusterBuffer cbuf[5];
    for (uint32_t i = 0; i < 5; i++)
        for (uint32_t j = 0; j < CLUSTER_SIZE; j++)
            cbuf[i].buf[j] = i + 'a';

    struct FAT32DriverRequest request = {
        .buf                   = cbuf,
        .name                  = "ikanaide",
        .ext                   = "   ",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0,
    };

    // write(request);  // Create folder "ikanaide"
    // memcpy(request.name, "kano1\0\0\0", 8);
    // write(request);  // Create folder "kano1"
    // memcpy(request.name, "ikanaide", 8);

    // memcpy(request.name, "daijoubu", 8);
    // request.buffer_size = 5*CLUSTER_SIZE;
    // write(request);  // Create fragmented file "daijoubu"

    // struct ClusterBuffer readcbuf;
    // read_clusters(&readcbuf, ROOT_CLUSTER_NUMBER+1, 1); 
    // // If read properly, readcbuf should filled with 'a'

    // struct ClusterBuffer cbuf2[5];
    // request.buf = cbuf2;

    // request.buffer_size = CLUSTER_SIZE;
    // read(request);   // Failed read due not enough buffer size
    // request.buffer_size = 5*CLUSTER_SIZE;
    // read(request);   // Success read on file "daijoubu"



    // memcpy(request.name, "ikanaide", 8);
    // memcpy(request.ext, "\0\0\0", 3);
    // delete(request);
    // for (uint32_t i = 0; i < 5; i++)
    //     for (uint32_t j = 0; j < CLUSTER_SIZE; j++)
    //         cbuf[i].buf[j] = i + 'l';
    // request.buf = cbuf;      
    // memcpy(request.name, "deleteya", 8);
    // write(request);
    
    // memcpy(request.name, "deleteya", 8);
    // memcpy(request.ext, "\0\0\0", 3);
    // delete(request);

    // memcpy(request.name, "delet5ya", 8);
    // memcpy(request.ext, "\0\0\0", 3);
    // write(request);

    // delete(request);

    memcpy(request.name, "delet6ya", 8);
    memcpy(request.ext, "\0\0\0", 3);
    delete(request);




    while (TRUE);
}
