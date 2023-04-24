#include "../lib-header/commands/cat.h"

void cat(struct CurrentWorkingDirectory cwd, char *file_path)
{
    // INITIALIZE VARIABLE FOR FILE
    char file_name[12] = "\0";
    int8_t retcode = read_path(file_path, &cwd, file_name);

    // SEPERATE FILE NAME INTO NAME AND EXT
    const char* delim = ".";
    char* name;
    char* ext;

    name = strtok(file_name, delim);
    ext = strtok(NULL, delim);

    // IF NO EXTENSION, REPLACE EXTENSION WITH \0
    if (ext == NULL){
        memcpy(ext, "\0\0\0", 3);
    }

    // CHECK IF NAME AND EXT OUTSIDE CONSTRAINTS
    if (strlen(name) > 8 || strlen(ext) > 3) {
        retcode = 3;
        puts("Error: File name or extension out of constraints", VGA_COLOR_RED);
        return;
    }

    // CREATE REQUEST 
    uint32_t found_cluster_number = 0;
    struct ClusterBuffer cbuf[8];
    struct FAT32DriverRequest req = {
        .buf = &cbuf,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = cwd.clusters_stack[cwd.top],
        .buffer_size = sizeof(cbuf),
    };

    // COPY NAME AND EXT TO req
    memcpy(req.name, name, strlen(name));
    memcpy(req.ext, ext, strlen(ext));

    // SYSCALL TO READ FILEs
    syscall(0, (uint32_t)&req, (uint32_t)&retcode, (uint32_t)&found_cluster_number);

    // IF SYSCALL SUCCEDEED
    if (retcode == 0){
        struct ClusterBuffer filled_buf[5];
        memcpy(filled_buf, req.buf, sizeof(filled_buf));

        for (int i = 0; i < 5; i++) {
            struct ClusterBuffer item = filled_buf[i];
            for (int j = 0; j < CLUSTER_SIZE; j++) {
                char c = (char) item.buf[j];
                char str[2] = {c, '\0'};
                puts(str, VGA_COLOR_WHITE);
            }
        }     
    } else if (retcode == 1){
        puts("Error: Not a file", VGA_COLOR_RED);
    } else if (retcode == 2){
        puts("Error: Not enough buffer", VGA_COLOR_RED);
    } else if (retcode == 3) {
        puts("Error: File not found", VGA_COLOR_RED);
    } else {
        puts("Error: Unknown", VGA_COLOR_RED);
    }
}