/**
 ******************************************************************************
 * @file           : flash_storage.h
 * @brief          : Flash storage for persistent data (highscore)
 ******************************************************************************
 * @attention
 *
 * This module provides non-volatile storage for game data like highscore.
 * Data persists across resets and power cycles.
 *
 * STM32F429 Flash Memory Layout:
 * - Sector 23 (last sector): 0x081E0000 - 0x081FFFFF (128KB)
 * - Used for storing highscore and other persistent game data
 *
 ******************************************************************************
 */

#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* Flash storage configuration for STM32F429 (2MB Flash) */
/* Using Sector 23 (last sector) to avoid conflicts with program code */
#define FLASH_STORAGE_SECTOR FLASH_SECTOR_23
#define FLASH_STORAGE_START_ADDR ((uint32_t)0x081E0000)
#define FLASH_STORAGE_END_ADDR ((uint32_t)0x081FFFFF)
#define FLASH_STORAGE_SIZE ((uint32_t)0x20000) /* 128KB */

/* Magic number to validate stored data */
#define FLASH_STORAGE_MAGIC ((uint32_t)0x534E414B) /* "SNAK" */

    /* Data structure stored in Flash */
    typedef struct
    {
        uint32_t magic;     /* Magic number for validation */
        uint16_t highScore; /* High score value */
        uint16_t reserved1; /* Reserved for alignment */
        uint32_t playCount; /* Number of games played */
        uint32_t checksum;  /* Simple checksum for data integrity */
    } FlashStorageData_t;

    /**
     * @brief Initialize flash storage module
     * @retval HAL_OK if successful, HAL_ERROR otherwise
     */
    HAL_StatusTypeDef FlashStorage_Init(void);

    /**
     * @brief Load high score from flash
     * @retval High score value (0 if not found or corrupted)
     */
    uint16_t FlashStorage_LoadHighScore(void);

    /**
     * @brief Save high score to flash
     * @param score: High score value to save
     * @retval HAL_OK if successful, HAL_ERROR otherwise
     */
    HAL_StatusTypeDef FlashStorage_SaveHighScore(uint16_t score);

    /**
     * @brief Load play count from flash
     * @retval Play count value (0 if not found)
     */
    uint32_t FlashStorage_LoadPlayCount(void);

    /**
     * @brief Increment and save play count
     * @retval HAL_OK if successful, HAL_ERROR otherwise
     */
    HAL_StatusTypeDef FlashStorage_IncrementPlayCount(void);

    /**
     * @brief Erase all stored data (reset to factory)
     * @retval HAL_OK if successful, HAL_ERROR otherwise
     */
    HAL_StatusTypeDef FlashStorage_EraseAll(void);

    /**
     * @brief Check if flash storage has valid data
     * @retval 1 if valid data exists, 0 otherwise
     */
    uint8_t FlashStorage_IsValid(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_STORAGE_H */
