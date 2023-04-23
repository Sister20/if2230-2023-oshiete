#include "../lib-header/commands/cat.h"

void cat(struct CurrentWorkingDirectory cwd, char *file_path)
{
    // INITIALIZE VARIABLE FOR FILE
    char file_name[12] = "\0";
    int8_t retcode = read_path(file_path, &cwd, file_name);

    // SEPERATE FILE NAME INTO NAME AND EXT
    const char* delim = ".";

    char* name = strtok(file_name, delim);
    char* ext = strtok(NULL, delim);

    // CHECK IF NAME AND EXT OUTSIDE CONSTRAINTS
    if (strlen(name) > 8 || strlen(ext) > 3) {
        retcode = 3;
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
    memcpy(req.name, name, 8);
    memcpy(req.ext, ext, 3);

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
                puts(str, VGA_COLOR_RED);
            }
        }     
    }
}