#include "../lib-header/commands/cd.h"

void cd(struct CurrentWorkingDirectory *cwd, char *dir_path)
{
    char new_dir[12] = "\0";
    int8_t retcode = read_path(dir_path, cwd, new_dir);
    if (strlen(new_dir) > 8)
    {
        retcode = 3;
    }
    if (retcode == 0 && new_dir[0] != '\0')
    {
        uint32_t found_cluster_number = 0;
        retcode = -1;
        struct FAT32DirectoryTable dir_table = {0};
        struct FAT32DriverRequest request = {
            .buf = &dir_table,
            .name = "\0\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = cwd->clusters_stack[cwd->top],
            .buffer_size = CLUSTER_SIZE,
        };
        memcpy(request.name, new_dir, 8);
        syscall(1, (uint32_t)&request, (uint32_t)&retcode, (uint32_t)&found_cluster_number);
        if (retcode == 0)
        {
            cwd->top++;
            cwd->clusters_stack[cwd->top] = found_cluster_number;
            memcpy(cwd->dir_names[cwd->top], new_dir, 8);
        }
    }
    switch (retcode)
    {
    case 0:
        break;
    case 1:
        puts("cd: No such file or directory", VGA_COLOR_RED);
        break;
    case 3:
        puts("cd: No such file or directory", VGA_COLOR_RED);
        break;
    default:
        puts("cd: Unknown error occured", VGA_COLOR_RED);
        break;
    }
}