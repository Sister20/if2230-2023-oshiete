#include "lib-header/stdtype.h"
#include "lib-header/fat32.h"
#include "lib-header/framebuffer.h"
#include "lib-header/string.h"
#include "lib-header/stdmem.h"
#include "lib-header/syscall.h"
#include "lib-header/commands/ls.h"
#include "lib-header/commands/cd.h"
#include "lib-header/commands/mkdir.h"
#include "lib-header/commands/cat.h"
#include "lib-header/commands/mv.h"

void pwd(struct CurrentWorkingDirectory cwd)
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
    for (int i = start_idx; i <= cwd.top; i++)
    {
        puts(cwd.dir_names[i], VGA_COLOR_LIGHT_MAGENTA);
        if (i != cwd.top)
        {
            puts("/", VGA_COLOR_LIGHT_MAGENTA);
        }
    }
    puts("$ ", VGA_COLOR_GREEN);
}

int main(void)
{
    char command[12][128];
    struct CurrentWorkingDirectory cwd = {
        .clusters_stack = {2},
        .dir_names = {"root\0\0\0\0"},
        .top = 0,
    };

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
    char buf[512];
    while (TRUE)
    {
        for (int i = 0; i < 512; i++)
        {
            buf[i] = '\0';
        }
        puts("user@OShiete: ", VGA_COLOR_CYAN);
        pwd(cwd);
        syscall(4, (uint32_t)buf, 512, 0);
        // syscall(5, (uint32_t)buf, 16, 0xF);
        int command_args = strparse(buf, command, " ");
        if (command_args > 0)
        {
            if (strcmp(command[0], "ls"))
            {
                ls(cwd, &request, command[1]);
            }
            if (strcmp(command[0], "cd"))
            {
                cd(&cwd, command[1]);
            } else if (strcmp(command[0], "mkdir")) {
                if (command_args >= 2) mkdir(cwd, command[1]);
            }
            if (strcmp(command[0], "cat")){
                cat(cwd, command[1]);
            }
            if (strcmp(command[0], "mv") && command_args >= 3){
                mv(cwd, command[1], command[2]);
            }
            
        }
        puts("\n", VGA_COLOR_BLACK);
    }

    return 0;
}
