/*******************************************************************************
 * Inclusion
 ******************************************************************************/
#include "FATfs.h"
 /*******************************************************************************
  * Variables
  ******************************************************************************/
static fatfs_bootsector_struct s_FAT12Infor;

static uint16_t* fat_table_entry = NULL;

static uint8_t* fatfs_buff = NULL;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
 /**
  * @brief
  * @param
  * @return
  */
static FATfs_status_t fatfs_initFile(char* file_name);

/**
 * @brief
 * @param
 * @return
 */
static uint32_t fatst_to_little_endian(uint8_t* data, size_t length);

/**
 * @brief
 * @param
 * @return
 */
static FATfs_status_t fatfs_bootsector_get_value(uint8_t* buff);

/**
 * @brief
 * @param
 * @return
 */
static FATfs_status_t fat_table_entry_handler(uint8_t* buff, uint16_t* fat_table_entry);
/**
 * @brief
 * @param
 * @return
 */
static FATfs_status_t fatfs_read_fatTable(void);

/**
 * @brief
 * @param
 * @return
 */
//static FATfs_status_t LFN_handler( uint8_t index);

/**
 * @brief
 * @param
 * @return
 */
static FATfs_status_t entry_handler(uint8_t* buff, fatfs_entry_struct* root_directory_entry, int *file_counter);

/*******************************************************************************
 * Code
 ******************************************************************************/
 /**
  * @brief
  * @param
  * @return
  */
static FATfs_status_t fatfs_initFile(char* file_name)
{
    FATfs_status_t status;
    if (HAL_SUCCESS == kmc_initFile(file_name))
    {
        status = FATFS_SUCCESS;
    }
    else
    {
        status = FATFS_FILE_OPEN_FAIL;
    }

    return status;
}

/**
 * @brief
 * @param
 * @return
 */
static uint32_t fatst_to_little_endian(uint8_t* data, size_t length)
{
    uint32_t result = 0; 
    for (size_t i = 0; i < length; i++)
    {
        result |= ((uint32_t)data[i] << (8 * i)); 
    }

    return result;
}

/**
 * @brief
 * @param
 * @return
 */
static FATfs_status_t fatfs_bootsector_get_value(uint8_t* buff)
{
    FATfs_status_t status = FATFS_SUCCESS;

    if (NULL != buff)
    {
        int counter = 0;
        kmc_read_sector(0, buff);
        s_FAT12Infor.jump_code = (uint32_t)fatst_to_little_endian(buff, SIZE_JUMP_CODE);
        counter += SIZE_JUMP_CODE;

        for (int idx = 0; idx < SIZE_OEM_NAME; idx++)
        {
            s_FAT12Infor.oem_name[idx] = buff[SIZE_JUMP_CODE + idx];
        }
        counter += SIZE_OEM_NAME;

        s_FAT12Infor.bytes_per_sector = (uint16_t)fatst_to_little_endian(buff + counter, SIZE_BYTES_PER_SECTOR);
        counter += SIZE_BYTES_PER_SECTOR;

        s_FAT12Infor.sectors_per_cluster = (uint8_t)fatst_to_little_endian(buff + counter, SIZE_SECTORS_PER_CLUSTER);
        counter += SIZE_SECTORS_PER_CLUSTER;

        s_FAT12Infor.reserved_sector = (uint16_t)fatst_to_little_endian(buff + counter, SIZE_RESERVED_SECTOR);
        counter += SIZE_RESERVED_SECTOR;

        s_FAT12Infor.fat_count = (uint8_t)fatst_to_little_endian(buff + counter, SIZE_FAT_COUNT);
        counter += SIZE_FAT_COUNT;

        s_FAT12Infor.max_root_entry = (uint16_t)fatst_to_little_endian(buff + counter, SIZE_MAX_ROOT_ENTRY);
        counter += SIZE_MAX_ROOT_ENTRY;

        s_FAT12Infor.total_sector_16 = (uint16_t)fatst_to_little_endian(buff + counter, SIZE_TOTAL_SECTOR_16);
        counter += SIZE_TOTAL_SECTOR_16;

        s_FAT12Infor.media_descriptor = (uint8_t)fatst_to_little_endian(buff + counter, SIZE_MEDIA_DESCRIPTOR);
        counter += SIZE_MEDIA_DESCRIPTOR;

        s_FAT12Infor.sectors_per_fat = (uint16_t)fatst_to_little_endian(buff + counter, SIZE_SECTORS_PER_FAT);
        counter += SIZE_SECTORS_PER_FAT;

        s_FAT12Infor.sectors_per_track = (uint16_t)fatst_to_little_endian(buff + counter, SIZE_SECTORS_PER_TRACK);
        counter += SIZE_SECTORS_PER_TRACK;

        s_FAT12Infor.num_of_heads = (uint16_t)fatst_to_little_endian(buff + counter, SIZE_NUM_OF_HEADS);
        counter += SIZE_NUM_OF_HEADS;

        s_FAT12Infor.hidden_sectors = (uint32_t)fatst_to_little_endian(buff + counter, SIZE_HIDDEN_SECTORS);
        counter += SIZE_HIDDEN_SECTORS;

        s_FAT12Infor.total_sector_32 = (uint32_t)fatst_to_little_endian(buff + counter, SIZE_TOTAL_SECTOR_32);
    }
    else
    {
        status = FATFS_READ_BOOTSECTOR_FAIL;
    }

    return status;
}

static FATfs_status_t fat_table_entry_handler(uint8_t* buff, uint16_t* fat_table_entry)
{
    FATfs_status_t status = FATFS_SUCCESS;
    if (NULL == buff || NULL == fat_table_entry)
    {
        status = FATFS_READ_FAT_TABLE_FAIL;
    }
    else
    {
        *fat_table_entry = (*buff | (*(buff + 1) << 8)) & 0xFFF;
        *(fat_table_entry + 1) = (*(buff + 1) >> 4 | (*(buff + 2) << 4)) & 0xFFF;
    }
    return status;
}

/**
 * @brief
 * @param
 * @return
 */
static FATfs_status_t fatfs_read_fatTable(void)
{
    FATfs_status_t status = FATFS_SUCCESS;
    uint32_t fatfsTable_index = s_FAT12Infor.hidden_sectors + s_FAT12Infor.reserved_sector;
    uint16_t max_fat_table_entry = ((s_FAT12Infor.sectors_per_fat * s_FAT12Infor.bytes_per_sector * 2) / 3);
    if (NULL != fat_table_entry)
    {
        free(fat_table_entry);
    }
    fat_table_entry = (uint16_t*)malloc(max_fat_table_entry * sizeof(uint16_t));

    if (NULL != fatfs_buff && 0 != fatfsTable_index)
    {
        uint32_t fatTable_adr = fatfsTable_index * s_FAT12Infor.bytes_per_sector;
        uint16_t fat_table_size = s_FAT12Infor.sectors_per_fat * s_FAT12Infor.bytes_per_sector;
        for (int i = 0; i < fat_table_size; i += 3)
        {
            fat_table_entry_handler(&fatfs_buff[fatTable_adr + i], fat_table_entry + (i / 3) * 2);
        }
    }
    else
    {
        status = FATFS_READ_FAT_TABLE_FAIL;
    }
    return status;
}

/**
 * @brief
 * @param
 * @return
 */
FATfs_status_t fatfs_init(char* file_name, uint8_t** buff)
{
    FATfs_status_t status = FATFS_SUCCESS;
    if (NULL != *buff)
    {
        free(*buff);
    }
    *buff = (uint8_t*)malloc(512 * sizeof(uint8_t));
    // fatfs_bootsector_struct boot_sector;
    if (NULL != file_name && NULL != *buff)
    {
        /* Open file */
        if (FATFS_FILE_OPEN_FAIL == fatfs_initFile(file_name))
        {
            status = FATFS_INIT_FAIL;
        }

        /* assign value for bootsector structure */
        if (FATFS_READ_BOOTSECTOR_FAIL == fatfs_bootsector_get_value(*buff))
        {
            status = FATFS_INIT_FAIL;
        }
        //cấp phát vùng nhớ cho fatsr_buff
        if (NULL != fatfs_buff)
        {
            free(fatfs_buff);
        }
        fatfs_buff = (uint8_t*)malloc(s_FAT12Infor.bytes_per_sector*s_FAT12Infor.total_sector_16 * sizeof(uint8_t));
        /* read all sector to buff */
        kmc_read_multi_sector(0, s_FAT12Infor.total_sector_16, fatfs_buff);
        
        /*update sector size to HAL*/
        if (FATFS_UPDATE_SECTOR_SIZE_FAIL == kmc_update_sector_size(s_FAT12Infor.bytes_per_sector))
        {
            status = FATFS_INIT_FAIL;
        }

        /* read FAT table and save in buff */
        if (FATFS_READ_FAT_TABLE_FAIL == fatfs_read_fatTable())
        {
            status = FATFS_INIT_FAIL;
        }
    }
    else
    {
        status = FATFS_INIT_FAIL;
    }

    free(*buff);
    return status;
}

static FATfs_status_t entry_handler(uint8_t* buff, fatfs_entry_struct* list_entry, int* file_counter)
{
    FATfs_status_t status = FATFS_SUCCESS;
    int buff_idx = 0;
    int LFN_idx = 0;
    uint32_t root_dir_idx = ((s_FAT12Infor.hidden_sectors + s_FAT12Infor.reserved_sector) + s_FAT12Infor.sectors_per_fat * s_FAT12Infor.fat_count);
    uint32_t root_dir_adr = root_dir_idx * s_FAT12Infor.bytes_per_sector;
    //uint8_t buff = fatfs_buff + root_dir_adr
    (*file_counter) = 0;
    if (NULL == buff || NULL == list_entry)
    {
        status = FATFS_ROOT_ENTRY_FAIL;
    }
    else
    {
        int idx = 0;
        int entry_idx = 0;
        while (((buff + buff_idx)[0] != 0x00))
        {
            int byte_counter = idx * 32;
            if ((buff + byte_counter)[0] != 0xE5 && (buff + byte_counter)[11] != 0x0F)
            {
                int size_counter = 0;
                for (uint8_t name_idx = 0; name_idx < SIZE_FILE_NAME; name_idx++)
                {
                    list_entry[entry_idx].file_name[name_idx] = (buff + byte_counter)[size_counter + name_idx];
                }
                size_counter += SIZE_FILE_NAME;

                for (uint8_t name_ext_idx = 0; name_ext_idx < SIZE_FILE_NAME_EXT; name_ext_idx++)
                {
                    list_entry[entry_idx].file_name_ext[name_ext_idx] = (buff + byte_counter)[size_counter + name_ext_idx];
                }
                size_counter += SIZE_FILE_NAME_EXT;

                list_entry[entry_idx].file_attributes = (uint8_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_FILE_ATTRIBUTES);
                size_counter += SIZE_FILE_ATTRIBUTES;

                list_entry[entry_idx].reserved = (uint8_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_RESERVED);
                size_counter += SIZE_RESERVED;

                list_entry[entry_idx].create_tenths = (uint8_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_CREATE_TENTHS);
                size_counter += SIZE_CREATE_TENTHS;

                list_entry[entry_idx].create_time = (uint16_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_CREATE_TIME);
                size_counter += SIZE_CREATE_TIME;

                list_entry[entry_idx].create_date = (uint16_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_CREATE_DATE);
                size_counter += SIZE_CREATE_DATE;

                list_entry[entry_idx].last_access_date = (uint16_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_LAST_ACCESS_DATE);
                size_counter += SIZE_LAST_ACCESS_DATE;

                list_entry[entry_idx].ea_index = (uint16_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_EA_INDEX);
                size_counter += SIZE_EA_INDEX;

                list_entry[entry_idx].last_mod_time = (uint16_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_LAST_MOD_TIME);
                size_counter += SIZE_LAST_MOD_TIME;

                list_entry[entry_idx].last_mod_date = (uint16_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_LAST_MOD_DATE);
                size_counter += SIZE_LAST_MOD_DATE;

                list_entry[entry_idx].first_cluster = (uint16_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_FIRST_CLUSTER);
                size_counter += SIZE_FIRST_CLUSTER;

                list_entry[entry_idx].file_size = (uint32_t)fatst_to_little_endian((buff + byte_counter) + size_counter, SIZE_FILE_SIZE);
                size_counter += SIZE_FILE_SIZE;

                buff_idx = byte_counter + size_counter;
                (*file_counter)++;
                entry_idx++;
            }
            idx++;
        }
    }
    return status;
}

FATfs_status_t fatfs_read_dir(int entry_index, fatfs_entry_struct** list_entry, int* file_counter)
{
    FATfs_status_t status = FATFS_SUCCESS;
    entry_index = entry_index - 1;
    if ((*list_entry == NULL) || (*list_entry != NULL && (*list_entry)[entry_index].first_cluster == 0x00))
    {
        *list_entry = (fatfs_entry_struct*)malloc(s_FAT12Infor.max_root_entry * sizeof(fatfs_entry_struct));
        if (NULL != fatfs_buff)
        {
            uint32_t num_sector_dir = (s_FAT12Infor.max_root_entry * 32) / 512;
            uint32_t root_dir_idx = ((s_FAT12Infor.hidden_sectors + s_FAT12Infor.reserved_sector) + s_FAT12Infor.sectors_per_fat * s_FAT12Infor.fat_count);
            uint32_t root_dir_adr = root_dir_idx * s_FAT12Infor.bytes_per_sector;
            entry_handler(fatfs_buff + root_dir_adr, *list_entry, file_counter);
        }
    }
    else if ((*list_entry)[entry_index].file_attributes == 0x10)
    {
        int data_adr = 33 * s_FAT12Infor.bytes_per_sector;
        int sub_dir_adr = data_adr + ((*list_entry)[entry_index].first_cluster - 2)*s_FAT12Infor.bytes_per_sector;
        free(*list_entry);
        *list_entry = (fatfs_entry_struct*)malloc(s_FAT12Infor.max_root_entry * sizeof(fatfs_entry_struct));
        entry_handler(fatfs_buff + sub_dir_adr, *list_entry, file_counter);
    }
    else if ((*list_entry)[entry_index].file_attributes == 0x0)
    {
        status = FATFS_FIND_FILE;
     
    }
    return status;
}



FATfs_status_t fatfs_read_file( fatfs_entry_struct entry, uint8_t** buff_data)
{
    FATfs_status_t status = FATFS_SUCCESS;

    // Tính toán địa chỉ vùng dữ liệu
    int data_area_adr = s_FAT12Infor.bytes_per_sector *
        (1 + s_FAT12Infor.fat_count * s_FAT12Infor.sectors_per_fat) +
        s_FAT12Infor.max_root_entry * 32;

    int current_cluster = entry.first_cluster;

    // Giải phóng bộ nhớ nếu cần
    if (buff_data && *buff_data)
    {
        free(*buff_data);
        *buff_data = NULL; // Đặt NULL để tránh dangling pointer
    }

    // Cấp phát bộ nhớ mới cho buff_data
    *buff_data = (unsigned char*)malloc((entry.file_size + 1) * sizeof(uint8_t));
    if (*buff_data == NULL)
    {
        return 0; // Trả về lỗi nếu cấp phát thất bại
    }

    if (fatfs_buff == NULL)
    {
        free(*buff_data); // Giải phóng nếu `buff` không hợp lệ
        *buff_data = NULL;
        return 0;
    }

    uint32_t write_index = 0;
    int while_flat = 0;
    do
    {
        
        int for_flat = 0;
        for (int i = 0;
            i < s_FAT12Infor.sectors_per_cluster * s_FAT12Infor.bytes_per_sector &&
            write_index < entry.file_size &&
            for_flat != 1;
            i++)
        {
            uint8_t byte = (fatfs_buff + data_area_adr +
                (current_cluster - 2) * s_FAT12Infor.sectors_per_cluster *
                s_FAT12Infor.bytes_per_sector)[i];

            if (byte == 0x00) // Điều kiện dừng khi gặp byte 0x00
            {
                for_flat = 1;
            }
            else
            {
                (*buff_data)[write_index++] = byte;
            }
        }

        if (fat_table_entry[current_cluster] == 0xFFF)
        {
            while_flat = 1;
        }
        else
        {
            current_cluster = fat_table_entry[current_cluster];
        }

    } while (write_index < entry.file_size && while_flat != 1 );

    // Đảm bảo dữ liệu kết thúc bằng '\0'
    if (write_index <= entry.file_size)
    {
        (*buff_data)[write_index] = '\0';
    }

    //if(write_index == entry.file_size)

    return status;
}

/**
 * @brief
 * @param
 * @return
 */
FATfs_status_t fatfs_deinitFile(void)
{
    FATfs_status_t status;

    /* Open file */
    if (HAL_SUCCESS == kmc_deinitFile())
    {
        status = FATFS_SUCCESS;
    }
    else
    {
        status = FATFS_FILE_CLOSE_FAIL;
    }

    return status;
}
