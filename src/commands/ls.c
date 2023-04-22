#include "../lib-header/commands/ls.h"

void ls(struct Cwd cwd, struct FAT32DriverRequest *request, char *dir_name)
{
    int32_t retcode = 0;
    int fragment_ctr = 0;
    while (fragment_ctr != -1 && retcode == 0)
    {
        if (dir_name[0] != '\0')
        { // there's a dir name argument
            memcpy(request->name, dir_name, 8);
            request->parent_cluster_number = cwd.clusters_stack[cwd.top];
            syscall(1, (uint32_t)request, (uint32_t)&retcode, (uint32_t)&fragment_ctr);
        }
        else if (strcmp(cwd.curr_dir, "root"))
        {
            syscall(6, (uint32_t)request, (uint32_t)&retcode, (uint32_t)&fragment_ctr);
        }
        else
        {
            memcpy(request->name, cwd.curr_dir, 8);
            request->parent_cluster_number = cwd.clusters_stack[cwd.top - 1];
            syscall(1, (uint32_t)request, (uint32_t)&retcode, (uint32_t)&fragment_ctr);
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
                        puts(dir_table->table[i].name, VGA_COLOR_WHITE);
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
    }
}