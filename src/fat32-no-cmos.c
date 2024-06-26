#include <time.h>
// #include "lib-header/stdtype.h"
#include <stdio.h>
#include <string.h>
#include "lib-header/fat32-no-cmos.h"
// #include "external-inserter.c"
// #include "lib-header/disk.h"
// #include "lib-header/stdmem.h"
// #include <stdbool.h>

// FAT32 - IF2230 edition"
const uint8_t fs_signature[BLOCK_SIZE] = {
    'F',
    'A',
    'T',
    '3',
    '2',
    ' ',
    '-',
    ' ',
    'I',
    'F',
    '2',
    '2',
    '3',
    '0',
    ' ',
    'e',
    'd',
    'i',
    't',
    'i',
    'o',
    'n',
    '\n',
};

bool is_empty_storage(void)
{
    uint8_t boot_sector[BLOCK_SIZE];
    read_blocks(boot_sector, BOOT_SECTOR, 1);
    return memcmp(boot_sector, fs_signature, sizeof(fs_signature));
};

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster)
{
    struct FAT32DirectoryEntry *entry = &(dir_table->table[0]); // updates index 0

    memcpy(entry->name, name, 8);
    time_t current_time;
    struct tm *time_info;

    time(&current_time);
    time_info = localtime(&current_time);
    entry->create_time = time_info->tm_hour << 8 | time_info->tm_min;
    entry->create_date = (time_info->tm_year - 100) << 9 | (time_info->tm_mon + 1) << 5 | time_info->tm_mday;
    entry->cluster_low = (uint16_t)(parent_dir_cluster & 0xFFFF); // points to parent dir
    entry->cluster_high = (uint16_t)(parent_dir_cluster >> 16);
    entry->attribute = ATTR_SUBDIRECTORY;
    entry->filesize = 0;
    entry->undelete = 1;

    // for (uint32_t i = 1; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++)
    // {
    //     struct FAT32DirectoryEntry entry = {0};
    //     dir_table->table[i] = entry;
    // }
};

void create_fat32(void)
{
    write_blocks(fs_signature, BOOT_SECTOR, 1); // write fs_signature into boot sector

    struct FAT32FileAllocationTable fat = {0};

    // initialize root
    struct FAT32DirectoryTable root_dir_table = {0};
    init_directory_table(&root_dir_table, "root\0\0\0\0", ROOT_CLUSTER_NUMBER);

    fat.cluster_map[0] = (uint32_t)CLUSTER_0_VALUE;
    fat.cluster_map[1] = (uint32_t)CLUSTER_1_VALUE;
    fat.cluster_map[2] = (uint32_t)FAT32_FAT_END_OF_FILE;

    // write fat and root dir table to disk
    write_blocks(&fat, cluster_to_lba(FAT_CLUSTER_NUMBER), CLUSTER_BLOCK_COUNT);
    write_blocks(&root_dir_table, cluster_to_lba(ROOT_CLUSTER_NUMBER), CLUSTER_BLOCK_COUNT);
}

void initialize_filesystem_fat32(void)
{
    if (is_empty_storage())
    {
        create_fat32();
    }

    // load fat to driver state
    struct FAT32FileAllocationTable fat = {0};
    read_clusters(&fat, FAT_CLUSTER_NUMBER, 1);
    driver_state.fat_table = fat;

    // load root directory table to driver state
    struct FAT32DirectoryTable root_dir_table = {0};
    read_clusters(&root_dir_table, ROOT_CLUSTER_NUMBER, 1);
    driver_state.dir_table_buf = root_dir_table;
}

int8_t read_directory(struct FAT32DriverRequest request)
{
    // struct ClusterBuffer cluster_data;
    // read parent cluster
    read_clusters((void *)&driver_state.dir_table_buf, request.parent_cluster_number, 1);

    // the parent cluster should be a directory cluster
    if (driver_state.dir_table_buf.table[0].attribute != ATTR_SUBDIRECTORY)
    {
        return -1;
    }

    int8_t ret = 2;
    // traverse the parent directory from i = 1 bc i = 0 stores the information about the parent dir
    uint32_t parent_cluster_number = request.parent_cluster_number;
    bool is_last_dir_table = driver_state.fat_table.cluster_map[parent_cluster_number] == FAT32_FAT_END_OF_FILE;
    do
    {
        for (int i = 1; i < (int)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++)
        {
            // current dir entry has the same name as req
            if (memcmp((void *)&driver_state.dir_table_buf.table[i].name, request.name, 8) == 0 && driver_state.dir_table_buf.table[i].undelete)
            {
                ret = 1;
                //  current dir entry is a subdir
                if (driver_state.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY)
                {
                    // copies the dir table to buf
                    int fragment_ctr = 0;
                    uint32_t dir_cluster_number;
                    // struct FAT32DirectoryTable *dir_table = (struct FAT32DirectoryTable *)request.buf;
                    dir_cluster_number = (driver_state.dir_table_buf.table[i].cluster_high << 16) | driver_state.dir_table_buf.table[i].cluster_low;
                    ret = 0;

                    time_t current_time;
                    struct tm *time_info;
                    time(&current_time);
                    time_info = localtime(&current_time);
                    driver_state.dir_table_buf.table[i].create_time = time_info->tm_hour << 8 | time_info->tm_min;
                    driver_state.dir_table_buf.table[i].create_date = (time_info->tm_year - 100) << 9 | (time_info->tm_mon + 1) << 5 | time_info->tm_mday;
                    write_clusters((void *)&driver_state.dir_table_buf, request.parent_cluster_number, 1);

                    // read the fat
                    read_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);

                    dir_cluster_number = (driver_state.dir_table_buf.table[i].cluster_high << 16) | driver_state.dir_table_buf.table[i].cluster_low;

                    do
                    {
                        read_clusters(request.buf + fragment_ctr * sizeof(struct FAT32DirectoryTable), dir_cluster_number, 1);
                        dir_cluster_number = dir_cluster_number = driver_state.fat_table.cluster_map[dir_cluster_number];
                        fragment_ctr++;
                        request.buffer_size -= sizeof(struct FAT32DirectoryTable);
                    } while (dir_cluster_number != FAT32_FAT_END_OF_FILE && request.buffer_size >= sizeof(struct FAT32DirectoryTable));
                    break;
                }
            }
        }
        parent_cluster_number = driver_state.fat_table.cluster_map[parent_cluster_number];
        is_last_dir_table = parent_cluster_number == FAT32_FAT_END_OF_FILE;
        if (!is_last_dir_table)
        {
            read_clusters((void *)&driver_state.dir_table_buf, parent_cluster_number, 1);
        }
    } while (!is_last_dir_table);
    return ret;
}

uint32_t findEmptyCluster()
{
    // LOOP THROUGH CLUSTER MAP, FIND CLUSTER TO FILL FILE/TABLE
    uint32_t i = 3;
    while (driver_state.fat_table.cluster_map[i] != 0 && i < CLUSTER_MAP_SIZE)
    {
        i++;
    }

    return i;
}

/**
 * FAT32 write, write a file or folder to file system.
 *
 * @param request All attribute will be used for write, buffer_size == 0 then create a folder / directory
 * @return Error code: 0 success - 1 file/folder already exist - 2 invalid parent cluster - -1 unknown
 */
int8_t write(struct FAT32DriverRequest request)
{
    // GET FAT TABLE
    int currentClusterNumber = request.parent_cluster_number;
    int currentParentClusterNumber = request.parent_cluster_number;
    read_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
    read_clusters((void *)&driver_state.dir_table_buf, currentParentClusterNumber, 1);

    // CHECK IF REQUEST IS FOLDER OR FILE
    bool isFolder = request.buffer_size == 0;

    // check if parent valid
    bool parent_valid = driver_state.dir_table_buf.table[0].attribute == ATTR_SUBDIRECTORY;

    if (parent_valid)
    {
        // LOOP THROUGH CLUSTER MAP, FIND CLUSTER TO FILL FILE/TABLE
        uint32_t clusterIndex = findEmptyCluster();

        // CHECK IF FILE OR FOLDER HAS EXISTED IN PARENT DIRECTORY
        bool foundEntry = 0;
        uint32_t entry = 0;
        uint32_t indexToCheck = 0;

        while (indexToCheck == 0)
        {
            // FIND WHICH ENTRY TO INSERT
            while (indexToCheck < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)))
            {
                // CHECK IF FILE OR FOLDER EXIST IN PARENT DIRECTORY
                if (isFolder && (memcmp((void *)driver_state.dir_table_buf.table[indexToCheck].name, request.name, 8) == 0) && (driver_state.dir_table_buf.table[indexToCheck].attribute == ATTR_SUBDIRECTORY) && driver_state.dir_table_buf.table[indexToCheck].undelete && indexToCheck != 0)
                {
                    return 1;
                }

                if (!isFolder && memcmp((void *)driver_state.dir_table_buf.table[indexToCheck].name, request.name, 8) == 0 && memcmp((void *)driver_state.dir_table_buf.table[indexToCheck].ext, request.ext, 3) == 0 && driver_state.dir_table_buf.table[indexToCheck].undelete && indexToCheck != 0)
                {
                    return 1;
                }

                // IF ENTRY HAS NOT BEEN FOUND
                if (!foundEntry)
                {
                    // IF CURRENT ENTRY IS EMPTY, USE THIS ENTRY
                    if (!driver_state.dir_table_buf.table[entry].undelete && entry != 0)
                    {
                        foundEntry = 1;
                    }
                    else
                    { // INCREMENT ENTRY AND SEARCH AGAIN
                        entry++;
                    }
                }
                indexToCheck++;
            }

            // IF CURRENT DIR_TABLE HAS MORE THAN 1 CLUSTER, CHECK IT!
            if (driver_state.fat_table.cluster_map[currentClusterNumber] != FAT32_FAT_END_OF_FILE)
            {
                read_clusters((void *)&driver_state.dir_table_buf, driver_state.fat_table.cluster_map[currentClusterNumber], 1);
                currentClusterNumber = driver_state.fat_table.cluster_map[currentClusterNumber];
                indexToCheck = 0;

                if (!foundEntry)
                {
                    entry = 0;
                }
            }
            else if (driver_state.fat_table.cluster_map[currentClusterNumber] == FAT32_FAT_END_OF_FILE && !foundEntry)
            {
                // make a new directory table
                struct FAT32DirectoryTable new_dir_table;
                uint32_t dir_cluster_number = (driver_state.dir_table_buf.table[0].cluster_high << 16) | driver_state.dir_table_buf.table[0].cluster_low;
                init_directory_table(&new_dir_table, (void *)driver_state.dir_table_buf.table[0].name, dir_cluster_number);
                write_clusters(&new_dir_table, clusterIndex, 1);
                driver_state.fat_table.cluster_map[currentClusterNumber] = clusterIndex;
                driver_state.fat_table.cluster_map[clusterIndex] = FAT32_FAT_END_OF_FILE;
                currentParentClusterNumber = clusterIndex;
                clusterIndex = findEmptyCluster();
            }
        }

        if (isFolder)
        {
            // create sub directory table from parent
            struct FAT32DirectoryTable new_table = {0};

            // init new_table
            init_directory_table(&new_table, request.name, request.parent_cluster_number);

            // write the new_table to cluster
            write_clusters(&new_table, clusterIndex, 1);

            // add directory entry to parent
            struct FAT32DirectoryEntry new_entry = new_table.table[0];
            new_entry.cluster_low = (uint16_t)(clusterIndex & 0xFFFF); // points to parent dir
            new_entry.cluster_high = (uint16_t)(clusterIndex >> 16);

            // GANTI USER ATTRIBUTE ROOT/PARENT FOLDER JADI NOT EMPTY
            driver_state.dir_table_buf.table[0].user_attribute = UATTR_NOT_EMPTY;

            // update driver_state
            driver_state.dir_table_buf.table[entry] = new_entry;
            driver_state.fat_table.cluster_map[clusterIndex] = FAT32_FAT_END_OF_FILE;

            // write the driver_state
            write_clusters((void *)&driver_state.dir_table_buf, currentParentClusterNumber, 1);
            write_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
        }
        else
        {
            int totalSize = request.buffer_size;
            uint8_t fragment_ctr = 0;
            struct FAT32DirectoryEntry *new_entry = (void *)&(driver_state.dir_table_buf.table[entry]);
            memcpy(new_entry->name, request.name, 8);
            memcpy(new_entry->ext, request.ext, 3);
            new_entry->cluster_high = (uint16_t)(clusterIndex >> 16);
            new_entry->cluster_low = (uint16_t)(clusterIndex & 0xFFFF);
            new_entry->filesize = totalSize;
            new_entry->attribute = 0;
            new_entry->user_attribute = UATTR_NOT_EMPTY;
            new_entry->undelete = 1;
            time_t current_time;
            struct tm *time_info;
            time(&current_time);
            time_info = localtime(&current_time);
            new_entry->create_time = time_info->tm_hour << 8 | time_info->tm_min;
            new_entry->create_date = (time_info->tm_year - 100) << 9 | (time_info->tm_mon + 1) << 5 | time_info->tm_mday;

            // GANTI USER ATTRIBUTE ROOT/PARENT FOLDER JADI NOT EMPTY
            driver_state.dir_table_buf.table[0].user_attribute = UATTR_NOT_EMPTY;

            // WRITE FILE TO CLUSTER
            int prevClusterIndex = -1;
            while (totalSize > 0)
            {
                if (prevClusterIndex != -1)
                {
                    driver_state.fat_table.cluster_map[prevClusterIndex] = clusterIndex;
                    prevClusterIndex = -1;
                }

                driver_state.fat_table.cluster_map[clusterIndex] = FAT32_FAT_END_OF_FILE;

                write_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
                write_clusters(request.buf + fragment_ctr * CLUSTER_SIZE, clusterIndex, 1);
                totalSize -= CLUSTER_SIZE;
                fragment_ctr++;
                if (totalSize > 0)
                {
                    prevClusterIndex = clusterIndex;
                    clusterIndex = findEmptyCluster();
                }
            }

            write_clusters((void *)&driver_state.dir_table_buf, currentParentClusterNumber, 1);
        }

        return 0;
    }
    else
    {
        return 2;
    }

    return -1;
}

int8_t read(struct FAT32DriverRequest request)
{

    // read parent cluster
    read_clusters((void *)&driver_state.dir_table_buf, request.parent_cluster_number, 1);

    // the parent cluster should be a directory cluster
    if (request.buffer_size < sizeof(struct FAT32DirectoryTable) || driver_state.dir_table_buf.table[0].attribute != ATTR_SUBDIRECTORY)
    {
        return -1;
    }

    int8_t ret = 3; // initialize ret to not found
    uint32_t parent_cluster_number = request.parent_cluster_number;
    bool is_last_dir_table = driver_state.fat_table.cluster_map[parent_cluster_number] == FAT32_FAT_END_OF_FILE;
    // traverse the parent directory from i = 1 bc i = 0 stores the information about the parent dir
    do
    {
        for (int i = 1; i < (int)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++)
        {
            // current dir entry has the same name as req
            if (memcmp((void *)&driver_state.dir_table_buf.table[i].name, request.name, 8) == 0 && driver_state.dir_table_buf.table[i].undelete)
            {
                if (driver_state.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY)
                {
                    ret = 1;
                }
                //  current dir entry is the searched file
                else if (memcmp((void *)&driver_state.dir_table_buf.table[i].ext, request.ext, 3) == 0)
                {

                    if (request.buffer_size < driver_state.dir_table_buf.table[i].filesize)
                    {
                        return 2; // buffer size is not enough
                    }

                    // read the fat
                    read_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);

                    int fragment_ctr = 0;
                    uint32_t dir_cluster_number;

                    ret = 0;
                    time_t current_time;
                    struct tm *time_info;
                    time(&current_time);
                    time_info = localtime(&current_time);
                    driver_state.dir_table_buf.table[i].access_date = (time_info->tm_year - 100) << 9 | (time_info->tm_mon + 1) << 5 | time_info->tm_mday;
                    write_clusters((void *)&driver_state.dir_table_buf, request.parent_cluster_number, 1);

                    dir_cluster_number = (driver_state.dir_table_buf.table[i].cluster_high << 16) | driver_state.dir_table_buf.table[i].cluster_low;

                    do
                    {
                        read_clusters(request.buf + fragment_ctr * sizeof(struct FAT32DirectoryTable), dir_cluster_number, 1);
                        dir_cluster_number = driver_state.fat_table.cluster_map[dir_cluster_number];
                        fragment_ctr++;
                        request.buffer_size -= sizeof(struct FAT32DirectoryTable);
                    } while (dir_cluster_number != FAT32_FAT_END_OF_FILE && request.buffer_size >= sizeof(struct FAT32DirectoryTable));
                    break;
                }
            }
        }
        parent_cluster_number = driver_state.fat_table.cluster_map[parent_cluster_number];
        is_last_dir_table = parent_cluster_number == FAT32_FAT_END_OF_FILE;
        if (!is_last_dir_table)
        {
            read_clusters((void *)&driver_state.dir_table_buf, parent_cluster_number, 1);
        }
    } while (!is_last_dir_table);
    return ret;
}

int8_t delete(struct FAT32DriverRequest request)
{
    const int DELETE_SUCCESS_RETURN = 0;
    const int REQUEST_NAME_NOT_FOUND_RETURN = 1;
    const int FOLDER_IS_NOT_EMPTY_RETURN = 2;
    const int UNKOWN_REQUEST_TYPE_RETURN = -1;
    // read parent cluster
    read_clusters((void *)&driver_state.dir_table_buf, request.parent_cluster_number, 1);
    // read FAT
    read_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);

    // the parent cluster should be a directory cluster
    bool parent_is_not_directory = driver_state.dir_table_buf.table[0].attribute != ATTR_SUBDIRECTORY;
    if (parent_is_not_directory)
    {
        return UNKOWN_REQUEST_TYPE_RETURN;
    }

    bool done = 0;
    int currentClusterNumber = request.parent_cluster_number;

    while (!done)
    {
        // traverse the parent directory from i = 1 bc i = 0 stores the information about the parent dir
        for (int i = 1; i < (int)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++)
        {
            bool current_entry_has_same_name = memcmp((void *)&driver_state.dir_table_buf.table[i].name, request.name, 8) == 0;
            bool current_entry_has_same_extension = memcmp((void *)&driver_state.dir_table_buf.table[i].ext, request.ext, 3) == 0;
            bool current_entry_is_undelete = driver_state.dir_table_buf.table[i].undelete;

            if (current_entry_has_same_extension && current_entry_has_same_name && current_entry_is_undelete)
            {
                bool current_entry_is_folder = driver_state.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY;

                if (current_entry_is_folder)
                {
                    struct FAT32DirectoryTable searched_dir;
                    uint32_t dir_cluster_number = (driver_state.dir_table_buf.table[i].cluster_high << 16) | driver_state.dir_table_buf.table[i].cluster_low;
                    read_clusters(&searched_dir, dir_cluster_number, 1);
                    if (searched_dir.table[0].user_attribute == UATTR_NOT_EMPTY)
                    {
                        return FOLDER_IS_NOT_EMPTY_RETURN;
                    }
                }
                // If the entry is an empty folder or a file, proceed to delete
                // find the clusters of the data in FAT
                uint32_t dir_cluster_number = (driver_state.dir_table_buf.table[i].cluster_high << 16) | driver_state.dir_table_buf.table[i].cluster_low;
                uint32_t next_cluster_number = driver_state.fat_table.cluster_map[dir_cluster_number];

                driver_state.fat_table.cluster_map[dir_cluster_number] = 0;

                while (next_cluster_number != FAT32_FAT_END_OF_FILE)
                {
                    dir_cluster_number = next_cluster_number;
                    next_cluster_number = driver_state.fat_table.cluster_map[dir_cluster_number];
                    driver_state.fat_table.cluster_map[dir_cluster_number] = 0;
                }

                // set to null in parent's directory table
                struct FAT32DirectoryEntry new_entry = {0};
                driver_state.dir_table_buf.table[i] = new_entry;

                // check if there are any objects in parent
                int dir_table_length = (int)CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry);
                int currentClusterNumber1 = request.parent_cluster_number;
                bool check_all_dir_table = 0;
                bool not_empty = 0;
                while (!check_all_dir_table)
                {
                    for (int i = 1; i < dir_table_length; i++)
                    {
                        // found a file
                        if (driver_state.dir_table_buf.table[i].undelete)
                        {
                            check_all_dir_table = 1;
                            not_empty = 1;
                            break;
                        }
                    }
                    if (driver_state.fat_table.cluster_map[currentClusterNumber1] != FAT32_FAT_END_OF_FILE && !not_empty)
                    {
                        read_clusters((void *)&driver_state.dir_table_buf, driver_state.fat_table.cluster_map[currentClusterNumber1], 1);
                        currentClusterNumber1 = driver_state.fat_table.cluster_map[currentClusterNumber1];
                        check_all_dir_table = 0;
                    }
                    else if (driver_state.fat_table.cluster_map[currentClusterNumber1] == FAT32_FAT_END_OF_FILE)
                    {
                        check_all_dir_table = 1;
                    }
                }

                if (!not_empty)
                {
                    driver_state.dir_table_buf.table[0].user_attribute = 0;
                }
                // else
                // {
                //     driver_state.dir_table_buf.table[i].user_attribute = 0;
                // }

                // write to parent's directory table and FAT
                write_clusters((void *)&driver_state.dir_table_buf, request.parent_cluster_number, 1);
                write_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);

                return DELETE_SUCCESS_RETURN;
            }
        }

        done = 1;

        // IF CURRENT DIR_TABLE HAS MORE THAN 1 CLUSTER, CHECK IT!
        if (driver_state.fat_table.cluster_map[currentClusterNumber] != FAT32_FAT_END_OF_FILE)
        {
            read_clusters((void *)&driver_state.dir_table_buf, driver_state.fat_table.cluster_map[currentClusterNumber], 1);
            currentClusterNumber = driver_state.fat_table.cluster_map[currentClusterNumber];
            done = 0;
        }
    }
    return REQUEST_NAME_NOT_FOUND_RETURN;
}