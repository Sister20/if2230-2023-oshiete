#include "lib-header/stdtype.h"
#include "lib-header/fat32.h"
#include "lib-header/disk.h"
#include "lib-header/stdmem.h"

// FAT32 - IF2230 edition"
const uint8_t fs_signature[BLOCK_SIZE] = {
    'F', 'A', 'T', '3', '2', ' ', '-', ' ', 'I', 'F', '2', '2', '3', '0', ' ', 'e',
    'd', 'i', 't', 'i', 'o', 'n', '\n',
    // [BLOCK_SIZE-2] = 'O',
    // [BLOCK_SIZE-1] = 'k',
};

uint32_t cluster_to_lba(uint32_t cluster)
{
    return cluster * CLUSTER_BLOCK_COUNT + 1;
}

bool is_empty_storage(void)
{
    uint8_t boot_sector[BLOCK_SIZE];
    read_blocks(boot_sector, 0, 1);
    return memcmp(boot_sector, fs_signature, sizeof(fs_signature));
};

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster)
{
    struct FAT32DirectoryEntry *entry = &(dir_table->table[0]); // updates index 0
    memcpy(entry->name, name, 8);
    memcpy(entry->ext, "   ", 3);
    entry->cluster_low = (uint16_t)(parent_dir_cluster & 0xFFFF); // points to parent dir
    entry->cluster_high = (uint16_t)(parent_dir_cluster >> 16);
    entry->attribute = ATTR_SUBDIRECTORY;
    entry->filesize = 0;
};

void create_fat32(void)
{
    write_blocks(fs_signature, BOOT_SECTOR, 1); // write fs_signature into boot sector

    struct FAT32FileAllocationTable fat;

    // initialize root
    struct FAT32DirectoryTable root_dir_table;
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
    struct FAT32FileAllocationTable fat;
    read_clusters(&fat, FAT_CLUSTER_NUMBER, 1);
    driver_state.fat_table = fat;

    // load root directory table to driver state
    struct FAT32DirectoryTable root_dir_table;
    read_clusters(&root_dir_table, ROOT_CLUSTER_NUMBER, 1);
    driver_state.dir_table_buf = root_dir_table;
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count)
{
    write_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
};

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count)
{
    read_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
};

int8_t read_directory(struct FAT32DriverRequest request)
{
    // struct ClusterBuffer cluster_data;
    // buffer size should be equal to size of FAT32DirectoryTable
    if (request.buffer_size != sizeof(struct FAT32DirectoryTable) || request.parent_cluster_number < ROOT_CLUSTER_NUMBER)
    {
        return -1;
    }

    // cast void pointer to FAT32DirectoryTable pointer
    struct FAT32DirectoryTable parent_dir_table;

    // read parent cluster
    read_clusters(&parent_dir_table, request.parent_cluster_number, 1);

    int8_t ret = 2;
    // traverse the parent directory
    for (int i = 0; i < (int)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++)
    {
        // current dir entry has the same name as req
        if (memcmp(&parent_dir_table.table[i].name, request.name, 8) == 0)
        {
            ret = 1;
            //  current dir entry is a subdir
            if (parent_dir_table.table[i].attribute == ATTR_SUBDIRECTORY)
            {
                // copies the dir table to buf
                struct FAT32DirectoryTable *dir_table = (struct FAT32DirectoryTable *)request.buf;
                uint32_t dir_cluster_number = (parent_dir_table.table[i].cluster_high << 16) | parent_dir_table.table[i].cluster_low;
                read_clusters(dir_table, dir_cluster_number, 1);
                ret = 0;
                break;
            }
        }
    }

    return ret;
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
    struct FAT32FileAllocationTable fat = driver_state.fat_table;

    // CHECK IF PARENT_CLUSTER_NUMBER IS VALID
    bool parentClusterValid = fat.cluster_map[request.parent_cluster_number] != 0;

    if (parentClusterValid){
        // CHECK IF FILE OR FOLDER ALREADY EXIST
        bool hasExisted = 0;

        // OPEN FOLDER
        //struct FAT32DirectoryTable dir_table = driver_state.dir_table_buf;

        if (!hasExisted){
            // LOOP THROUGH CLUSTER MAP, FIND CLUSTER TO FILL FILE/TABLE
            int i = 3;
            while (fat.cluster_map[i] != 0){
                i++;
            }

            // check whether request is file or folder
            if (request.buffer_size == 0){
                // create sub directory from parent
                struct FAT32DirectoryTable new_table;

                struct FAT32DirectoryEntry *new_entry = &(new_table.table[0]);
                memcpy(new_entry->name, request.name, 8);
                memcpy(new_entry->ext, "   ", 3);
                new_entry->cluster_low = (uint16_t)(request.parent_cluster_number & 0xFFFF);
                new_entry->cluster_high = (uint16_t)(request.parent_cluster_number >> 16);
                new_entry->attribute = ATTR_SUBDIRECTORY;
                new_entry->filesize = 0;

                // update parent directory attribute

                // write new table
                write_blocks(&new_table, cluster_to_lba(i), CLUSTER_BLOCK_COUNT);

            } else {
                // add file to cluster

            }

            driver_state.fat_table = fat;

            return 0;
        } 
        else 
        {
            return 1;
        }

    } 
    else 
    {
        return 2;
    }

    return -1;
}