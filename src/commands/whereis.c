#include "../lib-header/commands/whereis.h"

void whereis(char *file_name)
{
    // Initialize Output
    puts(file_name, VGA_COLOR_WHITE);
    puts(": ", VGA_COLOR_WHITE);

    // Initialize Current Working Directory
    struct CurrentWorkingDirectory cwd = {
        .clusters_stack = {2},
        .dir_names = {"root\0\0\0\0"},
        .top = 0,
    };

    // Read Root Directory
    int8_t retcode = -1;
    struct FAT32DirectoryTable dir_table = {0};
    struct FAT32DriverRequest request = {
        .buf = &dir_table,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = cwd.clusters_stack[cwd.top],
        .buffer_size = 8 * CLUSTER_SIZE,
    };
    syscall(6, (uint32_t)&request, (uint32_t)&retcode, 0);

    // Searching File
    for (int i = 1; i < 64 * 8; i++)
    {
        if (dir_table.table[i].name[0] != '\0' && dir_table.table[i].undelete)
        {
            if (dir_table.table[i].attribute == ATTR_SUBDIRECTORY)
            {
                // If Directory then Continue Search in The Directory
                cwd.top++;
                strcpy(cwd.dir_names[cwd.top], dir_table.table[i].name);
                cwd.clusters_stack[cwd.top] = (dir_table.table[i].cluster_high << 16 | dir_table.table[i].cluster_low);
                searchUtil(&cwd, file_name);
                cwd.top--;
            }
            else
            {
                // If File then Check If It Has The Same Name
                if (!memcmp(dir_table.table[i].name, file_name, strlen(file_name)))
                {
                    // If Same Name then Print The Path
                    for (int j = 0; j <= cwd.top; j++)
                    {
                        puts(cwd.dir_names[j], VGA_COLOR_WHITE);
                        puts("/", VGA_COLOR_WHITE);
                    }
                    // Print The File Name
                    puts(file_name, VGA_COLOR_WHITE);

                    // Print Extension if Exists
                    if (dir_table.table[i].ext[0] != '\0')
                    {
                        puts(".", VGA_COLOR_WHITE);
                        puts(dir_table.table[i].ext, VGA_COLOR_WHITE);
                    }

                    puts("  ", VGA_COLOR_WHITE);
                }
            }
        }
    }
}

void searchUtil(struct CurrentWorkingDirectory *cwd, char *file_name)
{
    // Read Current Directory
    int8_t retcode = -1;
    struct FAT32DirectoryTable dir_table = {0};
    struct FAT32DriverRequest request = {
        .buf = &dir_table,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = cwd->clusters_stack[cwd->top - 1],
        .buffer_size = 8 * CLUSTER_SIZE,
    };
    strcpy(request.name, cwd->dir_names[cwd->top]);
    syscall(1, (uint32_t)&request, (uint32_t)&retcode, 0);

    // Searching File
    for (int i = 1; i < 64 * 8; i++)
    {
        if (dir_table.table[i].name[0] != '\0' && dir_table.table[i].undelete)
        {
            if (dir_table.table[i].attribute == ATTR_SUBDIRECTORY)
            {
                // If Directory then Continue Search in The Directory
                cwd->top++;
                strcpy(cwd->dir_names[cwd->top], dir_table.table[i].name);
                cwd->clusters_stack[cwd->top] = (dir_table.table[i].cluster_high << 16 | dir_table.table[i].cluster_low);
                searchUtil(cwd, file_name);
                cwd->top--;
            }
            else
            {
                // If File then Check If It Has The Same Name
                if (!memcmp(dir_table.table[i].name, file_name, strlen(file_name)))
                {
                    // If Same Name then Print The Path
                    for (int j = 0; j <= cwd->top; j++)
                    {
                        puts(cwd->dir_names[j], VGA_COLOR_WHITE);
                        puts("/", VGA_COLOR_WHITE);
                    }
                    // Print The File Name
                    puts(file_name, VGA_COLOR_WHITE);

                    // Print Extension if Exists
                    if (dir_table.table[i].ext[0] != '\0')
                    {
                        puts(".", VGA_COLOR_WHITE);
                        puts(dir_table.table[i].ext, VGA_COLOR_WHITE);
                    }

                    puts("  ", VGA_COLOR_WHITE);
                }
            }
        }
    }
}