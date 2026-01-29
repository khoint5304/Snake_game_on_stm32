/**
 ******************************************************************************
 * @file           : audio_playback.h
 * @brief          : Header file for audio playback module
 ******************************************************************************
 */

#ifndef AUDIO_PLAYBACK_H
#define AUDIO_PLAYBACK_H

#include <stdint.h>

/**
 * @brief Audio format information structure
 */
typedef struct
{
    uint8_t channels;        /*!< 1 = Mono, 2 = Stereo */
    uint8_t bits_per_sample; /*!< 8 or 16 bits */
    uint32_t sample_rate;    /*!< Sample rate in Hz */
    uint32_t duration_ms;    /*!< Duration in milliseconds */
} AudioFormat_t;

/**
 * @brief Initialize audio playback module
 */
void AudioPlayback_Init(void);

/**
 * @brief Play audio through ISD1820
 *
 * Triggers the ISD1820 module to play pre-recorded audio
 */
void AudioPlayback_PlayISD1820(void);

/**
 * @brief Get audio data pointer and size
 *
 * @param audio_data Pointer to audio data array (output)
 * @param size Size of audio data in bytes (output)
 * @return 0 if audio data available, -1 if not available
 */
int AudioPlayback_GetAudioData(const uint8_t **audio_data, uint32_t *size);

/**
 * @brief Get audio format information
 *
 * @param format Pointer to AudioFormat_t structure (output)
 */
void AudioPlayback_GetAudioFormat(AudioFormat_t *format);

/**
 * @brief Print audio information (debugging)
 */
void AudioPlayback_PrintInfo(void);

#endif /* AUDIO_PLAYBACK_H */
