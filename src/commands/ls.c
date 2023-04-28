#include "../lib-header/commands/ls.h"

void ls(struct CurrentWorkingDirectory cwd, char *dir_name)
{
    static struct FAT32DirectoryTable dir_table[8] = {0};
    struct FAT32DriverRequest request =
        {
            .buf = dir_table,
            .name = "\0\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = cwd.clusters_stack[cwd.top],
            .buffer_size = 8 * CLUSTER_SIZE};

    int32_t retcode = 0;
    uint32_t found_cluster_number = 0;
    if (dir_name[0] != '\0')
    { // there's a dir name argument
        char parsed_dir_name[8] = {'\0'};
        retcode = read_path(dir_name, &cwd, parsed_dir_name);
        if (retcode == 0)
        {
            memcpy(request.name, parsed_dir_name, 8);
            request.parent_cluster_number = cwd.clusters_stack[cwd.top];
            syscall(1, (uint32_t)&request, (uint32_t)&retcode, (uint32_t)&found_cluster_number);
        }
    }
    else if (strcmp(cwd.dir_names[cwd.top], "root"))
    {
        syscall(6, (uint32_t)&request, (uint32_t)&retcode, 0);
    }
    else
    {
        memcpy(request.name, cwd.dir_names[cwd.top], 8);
        request.parent_cluster_number = cwd.clusters_stack[cwd.top - 1];
        syscall(1, (uint32_t)&request, (uint32_t)&retcode, (uint32_t)&found_cluster_number);
    }
    if (retcode == 0)
    {
        struct FAT32DirectoryTable *dir_table = request.buf;
        for (int i = 1; i < 64 * 8; i++)
        {
            if (dir_table->table[i].name[0] != '\0' && dir_table->table[i].undelete)
            {
                char name[8] = "\0\0\0\0\0\0\0";
                strcpy(name, dir_table->table[i].name);
                if (dir_table->table[i].attribute)
                {
                    puts(name, VGA_COLOR_LIGHT_RED);
                }
                else
                {
                    puts(name, VGA_COLOR_WHITE);
                    if (dir_table->table[i].ext[0] != '\0')
                    {
                        char ext[3] = "\0";
                        strcpy(ext, dir_table->table[i].ext);
                        puts(".", VGA_COLOR_WHITE);
                        puts(dir_table->table[i].ext, VGA_COLOR_WHITE);
                    }
                }
                if (i % 6 != 0)
                {
                    puts("       ", VGA_COLOR_WHITE);
                }
                else
                {
                    puts("\n", VGA_COLOR_WHITE);
                }
            }
        }
    }

    switch (retcode)
    {
    case 0:
        break;
    case 1:
        puts("ls: No such file or directory", VGA_COLOR_RED);
        break;
    case 3:
        puts("ls: No such file or directory", VGA_COLOR_RED);
        break;
    default:
        puts("ls: Unknown error occured", VGA_COLOR_RED);
        break;
    }
}