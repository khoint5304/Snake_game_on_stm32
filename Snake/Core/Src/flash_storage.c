/**
 ******************************************************************************
 * @file           : flash_storage.c
 * @brief          : Flash storage implementation for persistent data
 ******************************************************************************
 * @attention
 *
 * This module uses the last sector of STM32F429 internal flash (Sector 23)
 * to store persistent game data like highscore.
 *
 * The flash is organized as follows:
 * - Data is written at the beginning of Sector 23 (0x081E0000)
 * - Each save operation erases the sector and rewrites all data
 * - A magic number and checksum verify data integrity
 *
 ******************************************************************************
 */

#include "flash_storage.h"
#include <string.h>

/* Private variables */
static FlashStorageData_t cachedData = {0};
static uint8_t cacheValid = 0;

/* Private function prototypes */
static uint32_t CalculateChecksum(const FlashStorageData_t *data);
static HAL_StatusTypeDef WriteDataToFlash(const FlashStorageData_t *data);
static void ReadDataFromFlash(FlashStorageData_t *data);

/**
 * @brief Calculate simple checksum for data integrity
 */
static uint32_t CalculateChecksum(const FlashStorageData_t *data)
{
    uint32_t checksum = 0;
    checksum += data->magic;
    checksum += data->highScore;
    checksum += data->reserved1;
    checksum += data->playCount;
    return checksum ^ 0xA5A5A5A5;
}

/**
 * @brief Read data structure from flash memory
 */
static void ReadDataFromFlash(FlashStorageData_t *data)
{
    /* Read data directly from flash address */
    memcpy(data, (void *)FLASH_STORAGE_START_ADDR, sizeof(FlashStorageData_t));
}

/**
 * @brief Write data structure to flash memory
 * @note This erases the sector first, then writes data
 */
static HAL_StatusTypeDef WriteDataToFlash(const FlashStorageData_t *data)
{
    HAL_StatusTypeDef status = HAL_OK;
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError = 0;
    uint32_t *srcPtr;
    uint32_t destAddr;
    uint32_t numWords;
    uint32_t i;

    /* Unlock flash for writing */
    status = HAL_FLASH_Unlock();
    if (status != HAL_OK)
    {
        return status;
    }

    /* Clear any pending flash flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    /* Configure sector erase */
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3; /* 2.7V - 3.6V */
    eraseInit.Sector = FLASH_STORAGE_SECTOR;
    eraseInit.NbSectors = 1;

    /* Erase the sector */
    status = HAL_FLASHEx_Erase(&eraseInit, &sectorError);
    if (status != HAL_OK)
    {
        HAL_FLASH_Lock();
        return status;
    }

    /* Write data word by word (32-bit) */
    srcPtr = (uint32_t *)data;
    destAddr = FLASH_STORAGE_START_ADDR;
    numWords = (sizeof(FlashStorageData_t) + 3) / 4; /* Round up to word boundary */

    for (i = 0; i < numWords; i++)
    {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destAddr, srcPtr[i]);
        if (status != HAL_OK)
        {
            HAL_FLASH_Lock();
            return status;
        }
        destAddr += 4;
    }

    /* Lock flash after writing */
    HAL_FLASH_Lock();

    return HAL_OK;
}

/**
 * @brief Initialize flash storage module
 */
HAL_StatusTypeDef FlashStorage_Init(void)
{
    FlashStorageData_t tempData;

    /* Read current data from flash */
    ReadDataFromFlash(&tempData);

    /* Validate data */
    if (tempData.magic == FLASH_STORAGE_MAGIC)
    {
        uint32_t expectedChecksum = CalculateChecksum(&tempData);
        if (tempData.checksum == expectedChecksum)
        {
            /* Data is valid, cache it */
            memcpy(&cachedData, &tempData, sizeof(FlashStorageData_t));
            cacheValid = 1;
            return HAL_OK;
        }
    }

    /* Data is invalid or not present, initialize with defaults */
    cachedData.magic = FLASH_STORAGE_MAGIC;
    cachedData.highScore = 0;
    cachedData.reserved1 = 0;
    cachedData.playCount = 0;
    cachedData.checksum = CalculateChecksum(&cachedData);
    cacheValid = 1;

    /* Don't write to flash yet, will write when first score is saved */
    return HAL_OK;
}

/**
 * @brief Load high score from flash
 */
uint16_t FlashStorage_LoadHighScore(void)
{
    if (!cacheValid)
    {
        FlashStorage_Init();
    }
    return cachedData.highScore;
}

/**
 * @brief Save high score to flash
 */
HAL_StatusTypeDef FlashStorage_SaveHighScore(uint16_t score)
{
    HAL_StatusTypeDef status;

    if (!cacheValid)
    {
        FlashStorage_Init();
    }

    /* Only save if score is higher than current high score */
    if (score <= cachedData.highScore)
    {
        return HAL_OK; /* No need to save, current is higher */
    }

    /* Update cached data */
    cachedData.highScore = score;
    cachedData.checksum = CalculateChecksum(&cachedData);

    /* Write to flash */
    status = WriteDataToFlash(&cachedData);

    return status;
}

/**
 * @brief Load play count from flash
 */
uint32_t FlashStorage_LoadPlayCount(void)
{
    if (!cacheValid)
    {
        FlashStorage_Init();
    }
    return cachedData.playCount;
}

/**
 * @brief Increment and save play count
 */
HAL_StatusTypeDef FlashStorage_IncrementPlayCount(void)
{
    HAL_StatusTypeDef status;

    if (!cacheValid)
    {
        FlashStorage_Init();
    }

    /* Increment play count */
    cachedData.playCount++;
    cachedData.checksum = CalculateChecksum(&cachedData);

    /* Write to flash */
    status = WriteDataToFlash(&cachedData);

    return status;
}

/**
 * @brief Erase all stored data
 */
HAL_StatusTypeDef FlashStorage_EraseAll(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError = 0;

    /* Unlock flash */
    status = HAL_FLASH_Unlock();
    if (status != HAL_OK)
    {
        return status;
    }

    /* Clear pending flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    /* Erase sector */
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector = FLASH_STORAGE_SECTOR;
    eraseInit.NbSectors = 1;

    status = HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    HAL_FLASH_Lock();

    /* Reset cache */
    memset(&cachedData, 0, sizeof(cachedData));
    cacheValid = 0;

    return status;
}

/**
 * @brief Check if flash storage has valid data
 */
uint8_t FlashStorage_IsValid(void)
{
    if (!cacheValid)
    {
        FlashStorage_Init();
    }
    return (cachedData.magic == FLASH_STORAGE_MAGIC);
}
