#include "../lib-header/commands/cp.h"

void cp(struct CurrentWorkingDirectory cwd, char* src, char* dest, int8_t is_recursive, int8_t is_root_command)
{
   // CONSTRUCT CWD FOR SRC AND DEST
    struct CurrentWorkingDirectory src_cwd = cwd;
    struct CurrentWorkingDirectory dest_cwd = cwd;

    char src_file[12] = "\0";
    char dest_file[12] = "\0";

    char full_src_path[50] = "\0";
    char full_dest_path[50] = "\0";
    strcpy(full_src_path, src);
    strcpy(full_dest_path, dest);

    char changeable_src[50] = "\0";
    char changeable_dest[50] = "\0";
    strcpy(changeable_src, src);
    strcpy(changeable_dest, dest);

    int8_t src_retcode = read_path(changeable_src, &src_cwd, src_file);
    int8_t dest_retcode = read_path(changeable_dest, &dest_cwd, dest_file);

    if (src_retcode == 3){
        puts("Error: Source file not valid1", VGA_COLOR_RED);
        return;
    }

    // SEPERATE NAME AND EXT
    char src_name[8] = "\0";
    char src_ext[3] = "\0";
    src_retcode = separate_filename(src_file, src_name, src_ext);

    if (src_retcode == 3 && is_root_command){
        puts("Error: Source file name or extension out of constraints", VGA_COLOR_RED);
        return;
    }

    char dest_name[8] = "\0";
    char dest_ext[3] = "\0";
    dest_retcode = separate_filename(dest_file, dest_name, dest_ext);

    if (dest_retcode == 3 && is_root_command){
        puts("Error: Destination file name or extension out of constraints", VGA_COLOR_RED);
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

    uint32_t dest_cluster_number = 0;
    struct FAT32DirectoryTable dest_dir_table[8] = {0};
    struct ClusterBuffer dest_cbuf[8];
    struct FAT32DriverRequest dest_req = {
        .buf = dest_dir_table,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = dest_cwd.clusters_stack[dest_cwd.top],
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

    if (src_type == -1){
        puts("Error: Source file not valid2", VGA_COLOR_RED);
        return;
    }

    // dest_type 0 : dir, 1 : file
    int8_t dest_type = -1;

    // IF dest IS .
    if (strcmp(dest, ".")){
        dest_type = 0;
        dest_cluster_number = cwd.clusters_stack[cwd.top];
    }
    else {
        // CHECK IF DEST IS DIRECTORY
        memcpy(dest_req.name, dest_name, 8);
        memcpy(dest_req.ext, dest_ext,3);

        syscall(1, (uint32_t)&dest_req, (uint32_t)&dest_retcode, (uint32_t)&dest_cluster_number);
        
        if (dest_retcode == 0){
            dest_type = 0;
        } else {
            // CHECK IF DEST IS FILE
            dest_req.buf = dest_cbuf;
            dest_req.buffer_size = sizeof(dest_cbuf);

            syscall(0, (uint32_t)&dest_req, (uint32_t)&dest_retcode, (uint32_t)&dest_cluster_number);

            if (dest_retcode == 0){
                dest_type = 1;
            }
        }
    }

    // FILE PART
    // if source is a file but destination file exist, fail
    if (src_type == 1 && dest_type == 1 && is_root_command){
        puts("Error: Failed to copy because file name already exists.", VGA_COLOR_RED);
        return;
    }
    // if source is a file and destination file doesn't exist, proceed to copy
    else if (src_type == 1) {
        // CREATE NEW REQUEST STRUCT
        struct FAT32DriverRequest new_req = dest_req;
        memcpy(new_req.name, dest_req.name, 8);
        memcpy(new_req.ext, dest_req.ext, 3);
        new_req.buffer_size = sizeof(src_req.buf);
        new_req.buf = src_req.buf;

        // WRITE FILE
        syscall(2, (uint32_t)&new_req, (uint32_t)&dest_retcode, (uint32_t)&dest_cluster_number);

        // CHECK IF WRITE SUCCESS
        if (dest_retcode == 0 && is_root_command){
            puts("Success : File copied.", VGA_COLOR_GREEN);
        }
        else if (is_root_command) {
            puts("Error : Failed to copy file3.", VGA_COLOR_RED);
        }

    }
    // FOLDER PART
    // if source is a folder, recursive is required
    else if (src_type == 0) {
        if (is_recursive != 1 && is_root_command) {
            puts("Error : -r not specified.", VGA_COLOR_RED);
        } else {
            
            // if destination folder already exists, fail
            if (dest_type == 0) {
                puts("Error : Failed to copy because file name already exists.", VGA_COLOR_RED);
            } else {
                // Copy the buffer of source driver request to destination driver request    
                dest_req.buffer_size = 0;
                dest_req.buf = src_req.buf;

                syscall(2, (uint32_t)&dest_req, (uint32_t)&dest_retcode, (uint32_t)&dest_cluster_number);

                // loop through the directory table of src
                struct FAT32DirectoryTable *dir_table = src_req.buf;
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
                        puts(new_src, VGA_COLOR_GREEN);


                        char new_dest[50] = "\0";
                        strcpy(new_dest, full_dest_path);
                        strcat(new_dest, "/");
                        strcat(new_dest, file_name);
                        puts(new_dest, VGA_COLOR_BLUE);

                        cp(cwd, new_src, new_dest, 1, 0);
                    }
                }
                return ;

            }



        }

    }



}