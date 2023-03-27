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
    return cluster * CLUSTER_BLOCK_COUNT;
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

    // read parent cluster
    read_clusters((void *)&driver_state.dir_table_buf, request.parent_cluster_number, 1);
    // buffer size should be equal to size of FAT32DirectoryTable
    // the cluster should be a directory cluster
    if (request.buffer_size < sizeof(struct FAT32DirectoryTable) || driver_state.dir_table_buf.table[0].attribute != ATTR_SUBDIRECTORY)
    {
        return -1;
    }

    int8_t ret = 2;
    // traverse the parent directory from i = 1 bc i = 0 stores the information about the parent dir
    for (int i = 1; i < (int)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++)
    {
        // current dir entry has the same name as req
        if (memcmp((void *)&driver_state.dir_table_buf.table[i].name, request.name, 8) == 0)
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

                // check if the dir is fragmented

                // read the fat
                read_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);

                do
                {
                    read_clusters(request.buf + fragment_ctr * sizeof(struct FAT32DirectoryTable), dir_cluster_number, 1);
                    if (fragment_ctr == 0)
                    {
                        dir_cluster_number = (driver_state.dir_table_buf.table[i].cluster_high << 16) | driver_state.dir_table_buf.table[i].cluster_low;
                    }
                    else
                    {
                        dir_cluster_number = driver_state.fat_table.cluster_map[dir_cluster_number];
                    }
                    fragment_ctr++;
                    request.buffer_size -= sizeof(struct FAT32DirectoryTable);
                } while (dir_cluster_number != FAT32_FAT_END_OF_FILE && request.buffer_size >= sizeof(struct FAT32DirectoryTable));

                // to do : CHECK IF THE BUFFER SIZE IS ENOUGH???

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
    int currentParentClusterNumber = request.parent_cluster_number;
    read_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
    read_clusters((void *)&driver_state.dir_table_buf, currentParentClusterNumber, 1);

    // CHECK IF REQUEST IS FOLDER OR FILE
    bool isFolder = request.buffer_size == 0;

    // check if parent valid
    bool parent_valid = driver_state.dir_table_buf.table[0].attribute == ATTR_SUBDIRECTORY;

    if (parent_valid){
        // LOOP THROUGH CLUSTER MAP, FIND CLUSTER TO FILL FILE/TABLE
        uint32_t i = 3;
        while (driver_state.fat_table.cluster_map[i] != 0 && i < CLUSTER_MAP_SIZE){
            i++;
        }

        uint32_t entry = 0;
        while (entry == 0){
            // FIND WHICH ENTRY TO INSERT
            while (driver_state.dir_table_buf.table[entry].name[0] != '\0' && entry < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry))){
                if (isFolder && (memcmp((void *)driver_state.dir_table_buf.table[entry].name,request.name, 8) == 0) && (driver_state.dir_table_buf.table[entry].attribute == ATTR_SUBDIRECTORY)){
                    return 1;
                }
                if (!isFolder && memcmp((void *)driver_state.dir_table_buf.table[entry].name,request.name, 8) == 0 &&  memcmp((void *)driver_state.dir_table_buf.table[entry].ext,request.ext, 3) == 0)
                {
                    return 1;
                }

                entry++;
            }

            // IF CURRENT DIR_TABLE HAS MORE THAN 1 CLUSTER, CHECK IT!
            if (driver_state.fat_table.cluster_map[currentParentClusterNumber] != FAT32_FAT_END_OF_FILE && entry != (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry))){
                read_clusters((void *)&driver_state.dir_table_buf, driver_state.fat_table.cluster_map[currentParentClusterNumber], 1);
                entry = 0;
            }
        }
       
        if (isFolder){
                
            // create sub directory table from parent
            struct FAT32DirectoryTable new_table;

            // init new_table
            init_directory_table(&new_table, request.name, request.parent_cluster_number);

            // write the new_table to cluster
            write_clusters(&new_table, i, 1);
            
            // add directory entry to parent
            struct FAT32DirectoryEntry new_entry = new_table.table[0];
            new_entry.cluster_low = (uint16_t)(i & 0xFFFF); // points to parent dir
            new_entry.access_date = (uint16_t)(i >> 16);

            // GANTI USER ATTRIBUTE ROOT/PARENT FOLDER JADI NOT EMPTY
            driver_state.dir_table_buf.table[0].user_attribute = UATTR_NOT_EMPTY;

            // update driver_state
            driver_state.dir_table_buf.table[entry] = new_entry;
            driver_state.fat_table.cluster_map[i] = FAT32_FAT_END_OF_FILE;

            // write the driver_state
            write_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
            write_clusters((void *)&driver_state.dir_table_buf, currentParentClusterNumber, 1);
        }
        else
        {
            int totalSize = request.buffer_size; 

            while (totalSize > 0){
                // fill the entry
                struct FAT32DirectoryEntry *new_entry = (void *)&(driver_state.dir_table_buf.table[entry]);
                memcpy(new_entry->name, request.name, 8);
                memcpy(new_entry->ext, "   ", 3);
                new_entry->user_attribute = UATTR_NOT_EMPTY;
                new_entry->cluster_high = (uint16_t)(request.parent_cluster_number >> 16);
                new_entry->cluster_low = (uint16_t)(request.parent_cluster_number & 0xFFFF);
                new_entry->filesize = totalSize % 512;

                totalSize -= 512;
            }
        }

        return 0;
    }
    else {
        return 2;
    }

    return -1;

}

int8_t read(struct FAT32DriverRequest request)
{

    // read parent cluster
    read_clusters((void *)&driver_state.dir_table_buf, request.parent_cluster_number, 1);

    // the parent cluster should be a directory cluster
    if (driver_state.dir_table_buf.table[0].attribute == ATTR_SUBDIRECTORY)
    {
        return -1;
    }

    int8_t ret = 3; // initialize ret to not found

    // traverse the parent directory from i = 1 bc i = 0 stores the information about the parent dir
    for (int i = 1; i < (int)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++)
    {
        // current dir entry has the same name as req
        if (memcmp((void *)&driver_state.dir_table_buf.table[i].name, request.name, 8) == 0)
        {
            if (driver_state.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY)
            {
                ret = 1;
            }
            //  current dir entry is the searched file
            else if (driver_state.dir_table_buf.table[i].ext == request.ext)
            {

                if (request.buffer_size < driver_state.dir_table_buf.table[i].filesize)
                {
                    return 2; // buffer size is not enough
                }

                // read the fat
                read_clusters((void *)&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);

                int fragment_ctr = 0;
                uint32_t dir_cluster_number;
                // struct ClusterBuffer *dir_table = (struct ClusterBuffer *)request.buf;
                ret = 0;

                do
                {
                    read_clusters(request.buf + fragment_ctr * CLUSTER_SIZE, dir_cluster_number, 1);
                    if (fragment_ctr == 0)
                    {
                        dir_cluster_number = (driver_state.dir_table_buf.table[i].cluster_high << 16) | driver_state.dir_table_buf.table[i].cluster_low;
                    }
                    else
                    {
                        dir_cluster_number = driver_state.fat_table.cluster_map[dir_cluster_number];
                    }
                    fragment_ctr++;
                    request.buffer_size -= sizeof(struct ClusterBuffer);
                } while (dir_cluster_number != FAT32_FAT_END_OF_FILE);

                break;
            }
        }
    }
    return ret;
}