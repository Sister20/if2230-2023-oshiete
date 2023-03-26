#include "lib-header/portio.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/gdt.h"
#include "lib-header/framebuffer.h"
#include "lib-header/kernel_loader.h"
#include "lib-header/splash.h"
#include "lib-header/fat32.h"

void kernel_setup(void)
{
    enter_protected_mode(&_gdt_gdtr);
    framebuffer_clear();

    /* SPLASH SCREEN */
    splash();

    framebuffer_write(3, 8, 'H', 0, 0xF);
    framebuffer_write(3, 9, 'a', 0, 0xF);
    framebuffer_write(3, 10, 'i', 0, 0xF);
    framebuffer_write(3, 11, '!', 0, 0xF);
    framebuffer_set_cursor(3, 10);

    // write_blocks(fs_signature, 0, 1);
    initialize_filesystem_fat32();

    uint8_t cbuf[sizeof(struct FAT32DirectoryTable)];

    struct FAT32DriverRequest request = {
        .buf = cbuf,
        .name = "ikanaide",
        .ext = "uwu",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = sizeof(struct FAT32DirectoryTable),
    };

    while (TRUE)
        ;
}
