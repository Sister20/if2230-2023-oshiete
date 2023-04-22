#include "lib-header/stdtype.h"
#include "lib-header/fat32.h"
#include "lib-header/framebuffer.h"
#include "lib-header/string.h"
#include "lib-header/stdmem.h"
#include "lib-header/syscall.h"
#include "lib-header/commands/ls.h"

void pwd(struct Cwd cwd)
{
    int start_idx;
    if (cwd.top >= 10)
    {
        start_idx = cwd.top - 10;
        puts("../", VGA_COLOR_LIGHT_MAGENTA); // truncate
    }
    else
    {
        start_idx = 1;
        puts("/", VGA_COLOR_LIGHT_MAGENTA);
    }
    for (int i = start_idx; i < cwd.top; i++)
    {
        puts(cwd.dir_names[i], VGA_COLOR_LIGHT_MAGENTA);
    }
    puts("$", VGA_COLOR_GREEN);
}

int main(void)
{
    char command[12][8];
    struct Cwd cwd = {
        .clusters_stack = {2},
        .dir_names = {"root\0\0\0\0"},
        .top = 0,
        .curr_dir = "root\0\0\0\0"};

    struct ClusterBuffer cl = {0};
    struct FAT32DriverRequest request = {
        .buf = &cl,
        .name = "ikanaide",
        .ext = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = CLUSTER_SIZE,
    };
    int32_t retcode;
    syscall(0, (uint32_t)&request, (uint32_t)&retcode, 0);
    if (retcode == 0)
        syscall(5, (uint32_t) "owo\n", 4, 0xF);

    while (TRUE)
    {
        char buf[16];
        puts("user@OShiete: ", VGA_COLOR_CYAN);
        pwd(cwd);
        syscall(4, (uint32_t)buf, 16, 0);
        // syscall(5, (uint32_t)buf, 16, 0xF);
        int command_args = strparse(buf, command);
        if (command_args > 0)
        {
            if (strcmp(command[0], "ls"))
            {
                ls(cwd, &request, command[1]);
            }
        }
        puts("\n", VGA_COLOR_BLACK);
    }

    return 0;
}
