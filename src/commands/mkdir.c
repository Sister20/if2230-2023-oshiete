#include "../lib-header/commands/mkdir.h"

#define MAX_FOLDER_NAME_LENGTH 8

void mkdir(struct CurrentWorkingDirectory cwd, char *new_folder_path) {
    // Create A New Directory
    char folder_name[MAX_FOLDER_NAME_LENGTH] = "\0\0\0\0\0\0\0\0";
    size_t path_length = strlen(new_folder_path);

    // Find Start Index
    int start_idx = path_length - 1;
    while (start_idx >= 0) {
        if (new_folder_path[start_idx] == '/') {
            break;
        }
        start_idx--;
    }
    

    // Check If Folder Name Length is Over Max
    if (path_length - start_idx - 1 > MAX_FOLDER_NAME_LENGTH) {
        return;
    }

    // If Path Starts With / then Start From Root
    if (start_idx == 0) {
        cwd.top = 0;
        cwd.clusters_stack[0] = 2;
    }

    // Copy Folder Name
    for (size_t i = start_idx + 1; i < path_length; i++) {
        folder_name[i - start_idx - 1] = new_folder_path[i];
        new_folder_path[i] = '\0';
    }

    // Read Relative Path
    if (strlen(new_folder_path) != 0) {
        char temp[MAX_FOLDER_NAME_LENGTH] = "\0\0\0\0\0\0\0\0";
        int8_t retcode = read_path(new_folder_path, &cwd, temp);
    
        // Return if Path is Invalid
        if (retcode != 0) {
            return;
        }
    }

    // Create Request
    struct FAT32DriverRequest request = {
        .buf = 0,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = cwd.clusters_stack[cwd.top],
        .buffer_size = 0,
    };

    // Copy Folder Name
    strcpy(request.name, folder_name);

    // Create Directory
    int32_t retcode = 0;
    syscall(2, (uint32_t) &request, (uint32_t) &retcode, 0);

    return;
}
