#include "../lib-header/commands/mv.h"

void mv(struct CurrentWorkingDirectory cwd, char* src, char* dest)
{
    // CONSTRUCT CWD FOR SRC AND DEST
    struct CurrentWorkingDirectory src_cwd = cwd;
    struct CurrentWorkingDirectory dest_cwd = cwd;

    char src_file[12] = "\0";
    char dest_file[12] = "\0";

    int8_t src_retcode = read_path(src, &src_cwd, src_file);
    int8_t dest_retcode = read_path(dest, &dest_cwd, dest_file);

    if (src_retcode == 3){
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

    char dest_name[8] = "\0";
    char dest_ext[3] = "\0";
    dest_retcode = separate_filename(dest_file, dest_name, dest_ext);

    if (dest_retcode == 3){
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
        puts("Error: Source file not valid", VGA_COLOR_RED);
        return;
    }

    // CHECK IF DEST IS DIRECTORY
    memcpy(dest_req.name, dest_name, 8);
    memcpy(dest_req.ext, dest_ext,3);

    syscall(1, (uint32_t)&dest_req, (uint32_t)&dest_retcode, (uint32_t)&dest_cluster_number);

    // dest_type 0 : dir, 1 : file
    int8_t dest_type = -1;
    if (dest_retcode == 0){
        dest_type = 0;
    } else {
        // CHECK IF DEST IS FILE
        src_req.buf = dest_cbuf;
        dest_req.buffer_size = sizeof(dest_cbuf);

        syscall(0, (uint32_t)&dest_req, (uint32_t)&dest_retcode, (uint32_t)&dest_cluster_number);

        if (dest_retcode == 0){
            dest_type = 1;
        }
    }

    // if (src_type == 0){
    //     puts("src is folder", VGA_COLOR_LIGHT_BLUE);    
    // } else if (src_type == 1){
    //     puts("src is file", VGA_COLOR_LIGHT_GREEN);
    // } 

    // if (dest_type == 0){
    //     puts("dest is folder", VGA_COLOR_LIGHT_BLUE);    
    // } else if (dest_type == 1){
    //     puts("dest is file", VGA_COLOR_LIGHT_GREEN);
    // } else {
    //     puts("Error: Destination file not valid", VGA_COLOR_RED);
    // }
    
    // mv dir1:exist dir2:exist, move dir 1 to dir 2
    if (src_type == 0 && dest_type == 0){
        puts("1", VGA_COLOR_BLUE);

        struct FAT32DriverRequest prev_req = src_req;

        src_req.parent_cluster_number = dest_cluster_number;

        syscall(2, (uint32_t)&src_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);

        if (src_retcode == 0){
            puts("success", VGA_COLOR_GREEN);
        } else {
            puts("failed", VGA_COLOR_RED);
        }

        syscall(3, (uint32_t)&prev_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);

        if (src_retcode == 0){
            puts("delete success", VGA_COLOR_GREEN);
        } else {
            puts("delete failed", VGA_COLOR_RED);
        }

        

    } 
    // mv dir1:exist dir2:not-exist, rename dir 1 to dir 2
    else if (src_type == 0){
        puts("2", VGA_COLOR_BLUE);



    } 
    // mv file:exist dir:exist, move file to dir
    else if (src_type == 1 && dest_type == 0){
    
        struct FAT32DriverRequest prev_req = src_req;

        // write file to dir
        src_req.parent_cluster_number = dest_cluster_number;
        syscall(2, (uint32_t)&src_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);

        if (src_retcode == 0){
            puts("success", VGA_COLOR_GREEN);
        } else {
            puts("failed", VGA_COLOR_RED);
        }

        // delete file from prev dir
        syscall(3, (uint32_t)&prev_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);

        if (src_retcode == 0){
            puts("delete success", VGA_COLOR_GREEN);
        } else {
            puts("delete failed", VGA_COLOR_RED);
        }
        
        return;
    } 
    // mv file1:exist file2:exist, 
    else if (src_type == 1 && dest_type == 1){
        puts("4", VGA_COLOR_BLUE);
    } 
    // mv file1:exist file2:not-exist
    else if (src_type == 1){
        puts("5", VGA_COLOR_BLUE);

        struct FAT32DriverRequest prev_req = src_req;

        // write file to dir
        memcpy(src_req.name, dest_req.name, 8);
        memcpy(src_req.ext, dest_req.ext, 3);
        src_req.parent_cluster_number = src_cwd.clusters_stack[src_cwd.top];
        syscall(2, (uint32_t)&src_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);

        if (src_retcode == 0){
            puts("success", VGA_COLOR_GREEN);
        } else {
            puts("failed", VGA_COLOR_RED);
        }

        // struct FAT32DriverRequest prev_req = src_req;

        // memcpy(src_req.name, dest_req.name, 8);
        // memcpy(src_req.ext, dest_req.ext, 3);
        // src_req.parent_cluster_number = dest_req.parent_cluster_number;

        // syscall(2, (uint32_t)&src_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);

        // if (src_retcode == 0){
        //     puts("success", VGA_COLOR_GREEN);
        // } else {
        //     puts("failed", VGA_COLOR_RED);
        // }

        // delete file from prev dir
        syscall(3, (uint32_t)&prev_req, (uint32_t)&src_retcode, (uint32_t)&src_cluster_number);

        if (src_retcode == 0){
            puts("delete success", VGA_COLOR_GREEN);
        } else {
            puts("delete failed", VGA_COLOR_RED);
        }


    } 

    return;
}