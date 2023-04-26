#include "../lib-header/commands/touch.h"

void touch(struct CurrentWorkingDirectory cwd, char command[12][128])
{

    int32_t retcode = 0;
    for (int i = 1; i < 12; i++)
    {
        if (command[i][0] == '\0')
        {
            if (i == 0)
            {
                puts("touch: missing file operand\n", VGA_COLOR_RED);
            }
            break;
        }
        else
        {
            char file_path[8] = "\0";
            memcpy(file_path, command[i], 8);
            char file_name[8] = "\0";
            char *dump = "\0";
            retcode = read_path(command[i], &cwd, file_name);
            if (retcode == 0)
            {
                char name[8] = "\0";
                char ext[3] = "\0";
                retcode = separate_filename(file_name, name, ext);
                if (retcode == 0)
                {
                    struct FAT32DriverRequest request =
                        {
                            .buf = dump,
                            .name = "\0\0\0\0\0\0\0\0",
                            .ext = "\0\0\0",
                            .parent_cluster_number = cwd.clusters_stack[cwd.top],
                            .buffer_size = 1,
                        };
                    memcpy(request.name, name, 8);
                    memcpy(request.ext, ext, 3);
                    syscall(2, (uint32_t)&request, (uint32_t)&retcode, 0);
                    if (retcode == 3)
                    {
                        puts("touch: File ", VGA_COLOR_RED);
                        puts(file_path, VGA_COLOR_RED);
                        puts(" already exist!", VGA_COLOR_RED);
                    }
                }
            }
            else
            {
                puts("touch: No such file or directory", VGA_COLOR_RED);
            }
        }
    }
}