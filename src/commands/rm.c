#include "../lib-header/commands/rm.h"

void rm(struct CurrentWorkingDirectory cwd, char* src,int8_t is_recursive, int8_t is_root_command)
{
    // CONSTRUCT CWD FOR SRC 
    struct CurrentWorkingDirectory src_cwd = cwd;

    char src_file[12] = "\0";

    char full_src_path[50] = "\0";
    strcpy(full_src_path, src);

    char changeable_src[50] = "\0";
    strcpy(changeable_src, src);

    int8_t src_retcode = read_path(changeable_src, &src_cwd, src_file);

    if (src_retcode == 3 && is_root_command){
        puts("Error: Source file not valid", VGA_COLOR_RED);
        return;
    }

    // SEPERATE NAME AND EXT
    char src_name[8] = "\0";
    char src_ext[3] = "\0";
    src_retcode = separate_filename(src_file, src_name, src_ext);

    if (src_retcode == 3){
        puts("Error: Source file name or extension out of constraints", VGA_COLOR_RED);
        return;
    }

    // REQUEST STRUCT
    uint32_t src_cluster_number = 0;
    struct FAT32DirectoryTable src_dir_table[8] = {0};
    struct ClusterBuffer src_cbuf[8];
    struct FAT32DriverRequest src_req = {
        .buf = src_dir_table,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = src_cwd.clusters_stack[src_cwd.top],
        .buffer_size = 8 * CLUSTER_SIZE,
    };

    // CHECK IF SRC IS DIRECTORY
    memcpy(src_req.name, src_name, 8);
    memcpy(src_req.ext, src_ext,3);

    syscall(1, (uint32_t)&src_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);

    // src_type 0 : dir, 1 : file
    int8_t src_type = -1;
    if (src_retcode == 0){
        src_type = 0;
    } else {
        // CHECK IF SRC IS FILE
        src_req.buf = src_cbuf;
        src_req.buffer_size = sizeof(src_cbuf);

        syscall(0, (uint32_t)&src_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);

        if (src_retcode == 0){
            src_type = 1;
        }
    }

    if (src_type == -1) {
        puts("Error: Source file not valid.", VGA_COLOR_RED);
        return;
    } else {
        // FILE PART
        if (src_type == 1) {
            syscall(3, (uint32_t)&src_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);
            if (src_retcode == 0 && is_root_command) {
                return;
            } else if (src_retcode != 0) {
                puts("Error: Delete failed.", VGA_COLOR_RED);
                return;
            }
        } 
        // FOLDER PART
        else if (src_type == 0) {
            struct FAT32DirectoryTable *dir_table = src_req.buf;
            
            // if empty
            if (dir_table->table[0].user_attribute != UATTR_NOT_EMPTY) {
                syscall(3, (uint32_t)&src_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);
                if (src_retcode == 0 && is_root_command) {
                    return;
                } else if (src_retcode != 0 ) {
                    puts("Error: Delete failed.", VGA_COLOR_RED);
                    return;
                }
            } 
            // if not empty
            else {
                if (is_recursive) {
                    for (int i = 1 ; i < 8*64; i++){
                        if (dir_table->table[i].undelete) {
                            char file_name[50] = "\0";
                            strcpy(file_name, dir_table->table[i].name);
                            if (dir_table->table[i].attribute != ATTR_SUBDIRECTORY) {
                                strcat(file_name, ".");
                                strcat(file_name, dir_table->table[i].ext);
                            }
                            char new_src[50] = "\0";
                            strcpy(new_src, full_src_path);
                            strcat(new_src, "/");
                            strcat(new_src, file_name);

                            rm(cwd, new_src, 1, 0);
                        }
                    }
                    syscall(3, (uint32_t)&src_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);
                    if (src_retcode == 0 && is_root_command) {
                        return;
                    } else if (src_retcode != 0 ) {
                        puts("Error: Delete failed.", VGA_COLOR_RED);
                        return;
                    }
                } else {
                    puts("Error : -r not specified.", VGA_COLOR_RED);
                    return;
                }
            }
        }

        

    }
    return;
    
}