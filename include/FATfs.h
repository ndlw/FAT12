#pragma once
#ifndef FATFS_H
#define FATFS_H
/*******************************************************************************
 * Inclusion
 ******************************************************************************/
#include "HAL.h"
 /*******************************************************************************
  * Macro
  ******************************************************************************/
#define SIZE_OF_STRUCT_BOOTSECTOR 36

#define SIZE_JUMP_CODE (3)           // uint32_t
#define SIZE_OEM_NAME (8)            // uint64_t
#define SIZE_BYTES_PER_SECTOR (2)    // uint16_t
#define SIZE_SECTORS_PER_CLUSTER (1) // uint8_t
#define SIZE_RESERVED_SECTOR (2)     // uint16_t
#define SIZE_FAT_COUNT (1)           // uint8_t
#define SIZE_MAX_ROOT_ENTRY (2)      // uint16_t
#define SIZE_TOTAL_SECTOR_16 (2)     // uint16_t
#define SIZE_MEDIA_DESCRIPTOR (1)    // uint8_t
#define SIZE_SECTORS_PER_FAT (2)     // uint16_t
#define SIZE_SECTORS_PER_TRACK (2)   // uint16_t
#define SIZE_NUM_OF_HEADS (2)        // uint16_t
#define SIZE_HIDDEN_SECTORS (4)      // uint32_t
#define SIZE_TOTAL_SECTOR_32 (4)     // uint32_t

#define SIZE_FILE_NAME (8)            // char[8]
#define SIZE_FILE_NAME_EXT (3)        // char[3]
#define SIZE_FILE_ATTRIBUTES (1)      // uint8_t
#define SIZE_RESERVED (1)             // uint8_t
#define SIZE_CREATE_TENTHS (1)        // uint8_t
#define SIZE_CREATE_TIME (2)          // uint16_t
#define SIZE_CREATE_DATE (2)          // uint16_t
#define SIZE_LAST_ACCESS_DATE (2)     // uint16_t
#define SIZE_EA_INDEX (2)             // uint16_t
#define SIZE_LAST_MOD_TIME (2)        // uint16_t
#define SIZE_LAST_MOD_DATE (2)        // uint16_t
#define SIZE_FIRST_CLUSTER (2)        // uint16_t
#define SIZE_FILE_SIZE (4)            // uint32_t


  /*******************************************************************************
   * Enum
   ******************************************************************************/
typedef enum
{
    FATFS_SUCCESS,                /* Operation succeeded */
    FATFS_FILE_OPEN_FAIL,          /* Failed to open file */
    FATFS_FILE_READ_SECTOR_FAIL,   /* Failed to read from file */
    FATFS_FILE_CLOSE_FAIL,         /* Failed to close file */
    FATFS_READ_BOOTSECTOR_FAIL,
    FATFS_UPDATE_SECTOR_SIZE_FAIL, /* Failed to close file */
    FATFS_READ_FAT_TABLE_FAIL,     /* Failed to close file */
    FATFS_INIT_FAIL,
    FATFS_LFN_FAIL,
    FATFS_ROOT_ENTRY_FAIL,
    FATFS_READ_TIME_FAIL,
    FATFS_READ_DATE_FAIL,
    FATFS_READ_DIR_FAIL,
    FATFS_FIND_FILE,
    FATFS_READ_FILE_FAIL,
    FATFS_READ_FILE,
    FATFS_FILE_ERROR,              /* General failure */
} FATfs_status_t;

/*******************************************************************************
 * Struct
 ******************************************************************************/
typedef struct
{
    uint32_t jump_code;          /* Jump code */
    char oem_name[8];        /* EOM name */
    uint16_t bytes_per_sector;   /* Numbers of bytes per sector */
    uint8_t sectors_per_cluster; /* Numbers of sectors per cluster */
    uint16_t reserved_sector;    /* Sector reserved */
    uint8_t fat_count;           /* Numbers of FAT tables */
    uint16_t max_root_entry;     /* Maximum number of root directory entries */
    uint16_t total_sector_16;    /* Total sectors if num of sector smaller than 65535*/
    uint8_t media_descriptor;    /* Media descriptor */
    uint16_t sectors_per_fat;    /* Numbers of sectors per FAT */
    uint16_t sectors_per_track;  /* Numbers of sectors per track */
    uint16_t num_of_heads;       /* Numbers of heads */
    uint32_t hidden_sectors;     /* Numbers of hidden sectors */
    uint32_t total_sector_32;    /* Total sectors if num of sector greater than 65535*/
} fatfs_bootsector_struct;

typedef struct
{
    char file_name[8];            /* Name of file */
    char file_name_ext[3];        /* Extension of name file*/
    uint8_t file_attributes;      /* File attributes*/
    uint8_t reserved;             /* Reserved */
    uint8_t create_tenths;        /* Create time, fine resolution: 10ms units, values from 0 to 199 */
    uint16_t create_time;         /* Create time. The hour, minute and second */
    uint16_t create_date;         /* Create date. The year, month and day */
    uint16_t last_access_date;    /* Last access date */
    uint16_t ea_index;            /* High 2 bytes of first cluster number in FAT32 */
    uint16_t last_mod_time;       /* Last modified time */
    uint16_t last_mod_date;       /* Last modified date */
    uint16_t first_cluster;       /* First cluster in FAT12 and FAT16 */
    uint32_t file_size;           /* File size in bytes. */
    /*unsigned char checksum;
    char long_name[256];*/
} fatfs_entry_struct;

/*******************************************************************************
 * API
 ******************************************************************************/
FATfs_status_t fatfs_init(char* file_name, uint8_t** buff);

//FATfs_status_t fatfs_read_dir(uint8_t *buff);
FATfs_status_t fatfs_read_dir(int entry_index, fatfs_entry_struct** get_dir, int* file_counter);


FATfs_status_t fatfs_read_file( fatfs_entry_struct entry, uint8_t** buff_data);
#endif /* FATFS_H */

FATfs_status_t fatfs_deinitFile(void);