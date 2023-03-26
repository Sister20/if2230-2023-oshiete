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

/* -- Driver Interfaces -- */

/**
 * Convert cluster number to logical block address
 *
 * @param cluster Cluster number to convert
 * @return uint32_t Logical Block Address
 */
uint32_t cluster_to_lba(uint32_t cluster)
{
    return cluster * CLUSTER_BLOCK_COUNT + 1;
}

/**
 * Checking whether filesystem signature is missing or not in boot sector
 *
 * @return True if memcmp(boot_sector, fs_signature) returning inequality
 */
bool is_empty_storage(void)
{
    uint8_t boot_sector[BLOCK_SIZE];
    read_blocks(boot_sector, 0, 1);
    return memcmp(boot_sector, fs_signature, sizeof(fs_signature));
};

/**
 * Initialize DirectoryTable value with parent DirectoryEntry and directory name
 *
 * @param dir_table          Pointer to directory table
 * @param name               8-byte char for directory name
 * @param parent_dir_cluster Parent directory cluster number
 */
void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster)
{
    struct FAT32DirectoryEntry *entry = &(dir_table->table[0]);
    memcpy(entry->name, name, 8);
    memcpy(entry->ext, "   ", 3);
    entry->cluster_low = (uint16_t)(parent_dir_cluster & 0xFFFF);
    entry->cluster_high = (uint16_t)(parent_dir_cluster >> 16);
    entry->attribute = ATTR_SUBDIRECTORY;
    entry->filesize = 0;
};

/**
 * Create new FAT32 file system. Will write fs_signature into boot sector and
 * proper FileAllocationTable (contain CLUSTER_0_VALUE, CLUSTER_1_VALUE,
 * and initialized root directory) into cluster number 1
 */
void create_fat32(void)
{
    write_blocks(fs_signature, BOOT_SECTOR, 1); // write fs_signature into boot sector

    struct FAT32FileAllocationTable fat;

    struct FAT32DirectoryTable root_dir_table;
    init_directory_table(&root_dir_table, "root\0\0\0\0", ROOT_CLUSTER_NUMBER);

    fat.cluster_map[0] = (uint32_t)CLUSTER_0_VALUE;
    fat.cluster_map[1] = (uint32_t)CLUSTER_1_VALUE;
    fat.cluster_map[2] = (uint32_t)FAT32_FAT_END_OF_FILE;

    write_blocks(&fat, cluster_to_lba(1), CLUSTER_BLOCK_COUNT);
    write_blocks(&root_dir_table, cluster_to_lba(2), CLUSTER_BLOCK_COUNT);
}
/**
 * Initialize file system driver state, if is_empty_storage() then create_fat32()
 * Else, read and cache entire FileAllocationTable (located at cluster number 1) into driver state
 */
void initialize_filesystem_fat32(void)
{
    if (is_empty_storage())
    {
        create_fat32();
    }
    struct FAT32FileAllocationTable fat;
    read_blocks(&fat, cluster_to_lba(1), CLUSTER_BLOCK_COUNT);
    driver_state.fat_table = fat;
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