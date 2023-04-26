#include "../lib-header/commands/ls.h"

void ls(struct CurrentWorkingDirectory cwd, struct FAT32DriverRequest *request, char *dir_name)
{
    static struct FAT32DirectoryTable dir_table[8] = {0};
    request->buf = dir_table;
    request->buffer_size = 8 * CLUSTER_SIZE;
    int32_t retcode = 0;
    uint32_t found_cluster_number = 0;
    if (dir_name[0] != '\0')
    { // there's a dir name argument
        char parsed_dir_name[8] = {'\0'};
        retcode = read_path(dir_name, &cwd, parsed_dir_name);
        if (retcode == 0)
        {
            memcpy(request->name, parsed_dir_name, 8);
            request->parent_cluster_number = cwd.clusters_stack[cwd.top];
            syscall(1, (uint32_t)request, (uint32_t)&retcode, (uint32_t)&found_cluster_number);
        }
    }
    else if (strcmp(cwd.dir_names[cwd.top], "root"))
    {
        syscall(6, (uint32_t)request, (uint32_t)&retcode, 0);
    }
    else
    {
        memcpy(request->name, cwd.dir_names[cwd.top], 8);
        request->parent_cluster_number = cwd.clusters_stack[cwd.top - 1];
        syscall(1, (uint32_t)request, (uint32_t)&retcode, (uint32_t)&found_cluster_number);
    }
    if (retcode == 0)
    {
        struct FAT32DirectoryTable *dir_table = request->buf;
        for (int i = 1; i < 64; i++)
        {
            if (dir_table->table[i].name[0] != '\0' && dir_table->table[i].undelete)
            {
                if (dir_table->table[i].attribute)
                {
                    puts(dir_table->table[i].name, VGA_COLOR_LIGHT_RED);
                }
                else
                {
                    char name[8];
                    memcpy(name, dir_table->table[i].name, 8);
                    puts(name, VGA_COLOR_WHITE);
                    if (dir_table->table[i].ext[0] != '\0')
                    {
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
        puts("Error: Not a directory", VGA_COLOR_RED);
        break;
    case 3:
        puts("Error: Directory not found", VGA_COLOR_RED);
        break;
    default:
        puts("Error: Unknown", VGA_COLOR_RED);
        break;
    }
}