#include "../lib-header/commands/ls.h"

void ls(struct Cwd cwd, struct FAT32DriverRequest *request, int32_t *retcode)
{
    if (strcmp(cwd.curr_dir, "root"))
    {
        syscall(6, (uint32_t)request, (uint32_t)retcode, 0);
    }
    else
    {
        memcpy(request->name, cwd.curr_dir, 8);
        request->parent_cluster_number = cwd.clusters_stack[cwd.top - 1];
        syscall(1, (uint32_t)request, (uint32_t)retcode, 0);
        struct FAT32DirectoryTable *dir_table = request->buf;
        for (int i = 1; i < 64; i++)
        {
            if (dir_table->table[i].name[0] != '\0')
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