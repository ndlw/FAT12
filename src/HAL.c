/*******************************************************************************
 * Inclusion
 ******************************************************************************/
#include "HAL.h"

 /*******************************************************************************
  * Variables
  ******************************************************************************/
static FILE* fptr = NULL;
static uint16_t s_sectorSize = KMC_DEFAULT_SECTOR_SIZE;

/*******************************************************************************
 * Code
 ******************************************************************************/
 /**
  * @brief     open a file named file_name.
  * @param     file_name    [Input] name of file.
  * @return    error code.
  */
hal_status_t kmc_initFile(char* file_name)
{
    hal_status_t status;
    s_sectorSize = KMC_DEFAULT_SECTOR_SIZE;
    fopen_s(&fptr,file_name, "rb");

    if (NULL != fptr)
    {
        status = HAL_SUCCESS;
    }
    else
    {
        status = 0;
    }

    return status;
}
/**
 * @brief     update sector size.
 * @param     new_sector_size    [Input] new sector size.
 */
hal_status_t kmc_update_sector_size(uint16_t new_sector_size)
{
    hal_status_t status;
    if ((0 != new_sector_size) && (0 == (new_sector_size % 512)))
    {
        status = HAL_SUCCESS;
        s_sectorSize = new_sector_size;
    }
    else
    {
        status = HAL_UPDATE_SECTOR_SIZE_FAIL;
    }

    return status;
}

/**
 * @brief     read one sector with index is one of param of function.
 * @param     index    [Input]         index of sector.
 * @param     buff     [Input][Output] store sectors after reading.
 * @return    bytes read.
 */
int32_t kmc_read_sector(uint32_t index, uint8_t* buff)
{

    int32_t bytes_read = 0;

    if ((0 <= index) && (NULL != buff))
    {
        if (fptr != NULL && NULL != buff)
        {
            fseek(fptr, index * s_sectorSize, SEEK_SET);
            bytes_read = (int32_t)fread(buff, 1, s_sectorSize, fptr);
        }
    }

    return bytes_read;
}

/**
 * @brief     read multiple sector with index is one of param of function.
 * @param     index    [Input]         index of sector.
 * @param     num      [Input]         number of sectors to read.
 * @param     buff     [Input][Output] store sectors after reading.
 * @return    bytes read.
 */
int32_t kmc_read_multi_sector(uint32_t index, uint32_t num, uint8_t* buff)
{
    int32_t bytes_read = 0;
    if (0 <= index && 0 <= num && NULL != buff)
    {
        for (uint32_t time = 0; (time < num) && (kmc_read_sector((index + time), buff + (time * s_sectorSize)) != 0); time++)
        {
            bytes_read += kmc_read_sector((index + time), buff + (time * s_sectorSize)); /* di chuyển con trỏ buff để ghi dữ liệu từng lần đọc */
        }
    }

    return bytes_read;
}

/**
 * @brief     close a file named file_name.
 * @param     file_name    [Input] name of file.
 * @return    error code.
 */
hal_status_t kmc_deinitFile(void)
{
    hal_status_t status;
    if (NULL != fptr && 0 == fclose(fptr))
    {
        status = HAL_SUCCESS;
    }
    else
    {
        status = HAL_FILE_CLOSE_FAIL;
    }
    fptr = NULL;
    return status;
}