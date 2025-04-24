#include "HAL.h"
#include "FATfs.h"
static void printHex(uint8_t *buff, int numBytes)
{

    for (int i = 0; i < numBytes; i++)
    {
        printf("%.2X ", buff[i]);
        if (0 == ((i + 1) % 16))
        {
            printf("\n");
        }
    }
}
static void display(int file_counter, fatfs_entry_struct* list_entry);
/**
 * @brief
 * @param
 * @return
 */
static FATfs_status_t get_time(uint16_t time);

/**
 * @brief
 * @param
 * @return
 */
static FATfs_status_t get_date(uint16_t date);

int main(void)
{
    /*******************************************************************************
     * NEW TEST
     ******************************************************************************/

    uint8_t* buff = NULL;
    uint8_t* buff_data = NULL;
    fatfs_entry_struct* list_entry = NULL;
    int file_counter = 0;
    int click_num = -1;
    //fatfs_entry_struct root_entry;
    // int32_t numBytes = 0;
    if (FATFS_SUCCESS == fatfs_init("C:/Users/LENOVO/Documents/FPT_INTERNSHIP/MOCK_C/FRF_EMB23_01_MockCBasic_MockProject01/floppy/floppy.img", &buff))
    {
        //printf("ok1\n");
    }

    do
    {
        if (FATFS_SUCCESS == fatfs_read_dir( click_num, &list_entry, &file_counter))
        {
            system("cls");
            display(file_counter, list_entry);
            
        }
        else if(FATFS_FIND_FILE == fatfs_read_dir( click_num, &list_entry, &file_counter))
        {
            fatfs_read_file(list_entry[click_num - 1], &buff_data);
            system("cls");
            printf("%s\n\n", buff_data);
            display(file_counter, list_entry);

        }
        printf("Enter the folder number or file you want to open.\n");
        printf("Enter the sequence number of \"..\" to go back to the previous folder.\n");
        printf("Enter 0 to end the program.\n");
        printf("==> ");
        scanf_s("%d", &click_num);

    } while (click_num != 0);
    
   
    
}

static FATfs_status_t get_time(uint16_t time)
{
    FATfs_status_t status = FATFS_SUCCESS;

    if (0 >= time)
    {
        status = FATFS_READ_TIME_FAIL;
    }
    else
    {
        uint8_t hour = (uint8_t)((time >> 11) & 0x1F);
        uint8_t minute = (uint8_t)((time >> 5) & 0x1F);
        uint8_t second = (uint8_t)(time & 0x1F) * 2;
        printf(" %.2d:%.2d:%.2d \t\t|  ", hour, minute, second);
    }
    return status;
}

static FATfs_status_t get_date(uint16_t date)
{
    FATfs_status_t status = FATFS_SUCCESS;
    if (0 >= date)
    {
        status = FATFS_READ_DATE_FAIL;
    }
    else
    {
        uint16_t year = (uint16_t)((date >> 9) & 0x7F) + 1980;
        uint8_t month = (uint8_t)((date >> 5) & 0x0F);
        uint8_t day = (uint8_t)(date & 0x1F);
        printf(" %.2d/%.2d/%.2d\t\t| ", month, day, year);
    }
    return status;
}

static void display(int file_counter, fatfs_entry_struct* list_entry)
{
    printf("\n###########################################-CURRENT-FOLDER-##############################################\n");
    printf("\n=========================================================================================================\n");
    printf("| No | Name File\t\t| Last Mod Date\t\t| Type\t\t\t| Size (bytes)\t\t|\n");
    printf("---------------------------------------------------------------------------------------------------------\n");

    // Vòng lặp để hiển thị danh sách file
    for (int i = 0; i < file_counter && list_entry[i].first_cluster >= 0; i++)
    {
        // Số thứ tự
        printf("| %2d | ", i + 1);

        // Tên file
        printf("%.8s%.3s\t\t|", list_entry[i].file_name, list_entry[i].file_name_ext);
        // Ngày sửa đổi cuối cùng
        if (FATFS_READ_DATE_FAIL == get_date(list_entry[i].last_mod_date)) {
            printf("ERROR          ");
        }

        if (0x0 == list_entry[i].file_attributes)
        {
            printf("File\t\t\t| ");
        }
        else if (0x10 == list_entry[i].file_attributes)
        {
            printf("Directory\t\t| ");
        }
        // Kích thước file
        if (0x0 == list_entry[i].file_attributes)
        {
            printf(" %.d bytes\t\t", list_entry[i].file_size);
            printf("|\n");
        }
        else if (0x10 == list_entry[i].file_attributes)
        {
            printf("\t\t\t");
            printf("|\n");
        }
    }
    printf("=========================================================================================================\n");
}