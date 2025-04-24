#pragma once
#ifndef HAL_H
#define HAL_H
/*******************************************************************************
 * Inclusion
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

 /*******************************************************************************
  * Macro
  ******************************************************************************/
#define KMC_DEFAULT_SECTOR_SIZE 512 /* Default size of sector */

  /*******************************************************************************
   * Enum
   ******************************************************************************/
typedef enum
{
    HAL_SUCCESS = 0x00,                 /* Operation succeeded */
    HAL_FILE_OPEN_FAIL = 0x01,          /* Failed to open file */
    HAL_FILE_READ_SECTOR_FAIL = 0x02,   /* Failed to read from file */
    HAL_FILE_CLOSE_FAIL = 0x03,         /* Failed to close file */
    HAL_UPDATE_SECTOR_SIZE_FAIL = 0x04, /* Failed to update sector file file */
    HAL_FILE_ERROR = 0xFF,              /* General failure */
} hal_status_t;

/*******************************************************************************
 * API
 ******************************************************************************/
 /**
  * @brief     open a file named file_name.
  * @param     file_name    [Input] name of file.
  * @return    error code.
  */
hal_status_t kmc_initFile(char* file_name);

/**
 * @brief     update sector size.
 * @param     new_sector_size    [Input] new sector size.
 */
hal_status_t kmc_update_sector_size(uint16_t new_sector_size);

/**
 * @brief     read one sector with index is one of param of function.
 * @param     index    [Input]         index of object.
 * @param     buff     [Input][Output] store sectors after reading.
 * @return    bytes read.
 */
int32_t kmc_read_sector(uint32_t index, uint8_t* buff);

/**
 * @brief     read multiple sector with index is one of param of function.
 * @param     index    [Input]         index of byte.
 * @param     num      [Input]         number of sectors to read.
 * @param     buff     [Input][Output] store sectors after reading.
 * @return    bytes read.
 */
int32_t kmc_read_multi_sector(uint32_t index, uint32_t num, uint8_t* buff);

/**
 * @brief     close a file named file_name.
 * @param     file_name    [Input] name of file.
 * @return    error code.
 */
hal_status_t kmc_deinitFile(void);

#endif /* HAL_H */
