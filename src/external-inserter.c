#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
// #include "lib-header/stdtype.h"

// Usual gcc fixed width integer type
// typedef u_int32_t uint32_t;
// typedef u_int8_t  uint8_t;

// Manual import from fat32.h, disk.h, & stdmem.h due some issue with size_t
#define BLOCK_SIZE 512

/* -- IF2230 File System constants -- */
#define BOOT_SECTOR 0
#define CLUSTER_BLOCK_COUNT 4
#define CLUSTER_SIZE (BLOCK_SIZE * CLUSTER_BLOCK_COUNT)
#define CLUSTER_MAP_SIZE 512

/* -- FAT32 FileAllocationTable constants -- */
// FAT reserved value for cluster 0 and 1 in FileAllocationTable
#define CLUSTER_0_VALUE 0x0FFFFFF0
#define CLUSTER_1_VALUE 0x0FFFFFFF

// EOF also double as valid cluster / "this is last valid cluster in the chain"
#define FAT32_FAT_END_OF_FILE 0x0FFFFFFF
#define FAT32_FAT_EMPTY_ENTRY 0x00000000

#define FAT_CLUSTER_NUMBER 1
#define ROOT_CLUSTER_NUMBER 2

/* -- FAT32 DirectoryEntry constants -- */
#define ATTR_SUBDIRECTORY 0b00010000
#define UATTR_NOT_EMPTY 0b10101010

// Boot sector signature for this file system "FAT32 - IF2230 edition"
extern const uint8_t fs_signature[BLOCK_SIZE];

// Cluster buffer data type - @param buf Byte buffer with size of CLUSTER_SIZE
struct ClusterBuffer
{
    uint8_t buf[CLUSTER_SIZE];
} __attribute__((packed));

/* -- FAT32 Data Structures -- */

/**
 * FAT32 FileAllocationTable, for more information about this, check guidebook
 *
 * @param cluster_map Containing cluster map of FAT32
 */
struct FAT32FileAllocationTable
{
    uint32_t cluster_map[CLUSTER_MAP_SIZE];
} __attribute__((packed));

/**
 * FAT32 standard 8.3 format - 32 bytes DirectoryEntry, Some detail can be found at:
 * https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#Directory_entry, and click show table.
 *
 * @param name           Entry name
 * @param ext            File extension
 * @param attribute      Will be used exclusively for subdirectory flag / determining this entry is file or folder
 * @param user_attribute If this attribute equal with UATTR_NOT_EMPTY then entry is not empty
 *
 * @param undelete       Unused / optional
 * @param create_time    Unused / optional
 * @param create_date    Unused / optional
 * @param access_time    Unused / optional
 * @param cluster_high   Upper 16-bit of cluster number
 *
 * @param modified_time  Unused / optional
 * @param modified_date  Unused / optional
 * @param cluster_low    Lower 16-bit of cluster number
 * @param filesize       Filesize of this file, if this is directory / folder, filesize is 0
 */
struct FAT32DirectoryEntry
{
    char name[8];
    char ext[3];
    uint8_t attribute;
    uint8_t user_attribute;

    bool undelete;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t cluster_high;

    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t cluster_low;
    uint32_t filesize;
} __attribute__((packed));

// FAT32 DirectoryTable, containing directory entry table - @param table Table of DirectoryEntry that span within 1 cluster
struct FAT32DirectoryTable
{
    struct FAT32DirectoryEntry table[CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)];
} __attribute__((packed));

/* -- FAT32 Driver -- */

/**
 * FAT32DriverState - Contain all driver states
 *
 * @param fat_table     FAT of the system, will be loaded during initialize_filesystem_fat32()
 * @param dir_table_buf Buffer for directory table
 * @param cluster_buf   Buffer for cluster
 */
struct FAT32DriverState
{
    struct FAT32FileAllocationTable fat_table;
    struct FAT32DirectoryTable dir_table_buf;
    struct ClusterBuffer cluster_buf;
} __attribute__((packed));

static volatile struct FAT32DriverState driver_state;
/**
 * FAT32DriverRequest - Request for Driver CRUD operation
 *
 * @param buf                   Pointer pointing to buffer
 * @param name                  Name for directory entry
 * @param ext                   Extension for file
 * @param parent_cluster_number Parent directory cluster number, for updating metadata
 * @param buffer_size           Buffer size, CRUD operation will have different behaviour with this attribute
 */
// struct FAT32DriverRequest
// {
//     void *buf;
//     char name[8];
//     char ext[3];
//     uint32_t parent_cluster_number;
//     uint32_t buffer_size;
// } __attribute__((packed));

struct FAT32DriverRequest
{
    void *buf;
    char name[8];
    char ext[3];
    uint32_t parent_cluster_number;
    uint32_t buffer_size;
} __attribute__((packed));

void *memcpy(void *restrict dest, const void *restrict src, size_t n);

void initialize_filesystem_fat32(void);
int8_t read(struct FAT32DriverRequest request);
int8_t read_directory(struct FAT32DriverRequest request);
int8_t write(struct FAT32DriverRequest request);
int8_t delete(struct FAT32DriverRequest request);

// Global variable
uint8_t *image_storage;
uint8_t *file_buffer;

uint32_t cluster_to_lba(uint32_t cluster)
{
    return cluster * CLUSTER_BLOCK_COUNT;
}

void read_blocks(void *ptr, uint32_t logical_block_address, uint8_t block_count)
{
    for (int i = 0; i < block_count; i++)
        memcpy((uint8_t *)ptr + BLOCK_SIZE * i, image_storage + BLOCK_SIZE * (logical_block_address + i), BLOCK_SIZE);
}

void write_blocks(const void *ptr, uint32_t logical_block_address, uint8_t block_count)
{
    for (int i = 0; i < block_count; i++)
        memcpy(image_storage + BLOCK_SIZE * (logical_block_address + i), (uint8_t *)ptr + BLOCK_SIZE * i, BLOCK_SIZE);
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count)
{
    write_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
};

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count)
{
    read_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
};

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "inserter: ./inserter <file to insert> <parent cluster index> <storage>\n");
        exit(1);
    }

    // Read storage into memory, requiring 4 MB memory
    image_storage = malloc(4 * 1024 * 1024);
    file_buffer = malloc(4 * 1024 * 1024);
    FILE *fptr = fopen(argv[3], "r");
    fread(image_storage, 4 * 1024 * 1024, 1, fptr);
    fclose(fptr);

    // Read target file, assuming file is less than 4 MiB
    FILE *fptr_target = fopen(argv[1], "r");
    size_t filesize = 0;
    if (fptr_target == NULL)
        filesize = 0;
    else
    {
        filesize = ftell(fptr_target);
        fread(file_buffer, 4 * 1024 * 1024, 1, fptr_target);
        fseek(fptr_target, 0, SEEK_END);
        fclose(fptr_target);
    }

    printf("Filename : %s\n", argv[1]);
    printf("Filesize : %ld bytes\n", filesize);

    // FAT32 operations
    initialize_filesystem_fat32();
    struct FAT32DriverRequest request = {
        .buf = file_buffer,
        .ext = "\0\0\0",
        .buffer_size = filesize,
    };
    sscanf(argv[2], "%u", &request.parent_cluster_number);
    sscanf(argv[1], "%8s", request.name);
    int retcode = write(request);
    if (retcode == 0)
        puts("Write success");
    else if (retcode == 1)
        puts("Error: File/folder name already exist");
    else if (retcode == 2)
        puts("Error: Invalid parent cluster");
    else
        puts("Error: Unknown error");

    // Write image in memory into original, overwrite them
    fptr = fopen(argv[3], "w");
    fwrite(image_storage, 4 * 1024 * 1024, 1, fptr);
    fclose(fptr);

    return 0;
}