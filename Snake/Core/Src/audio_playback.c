/**
 ******************************************************************************
 * @file           : audio_playback.c
 * @brief          : WAV audio playback driver for STM32F429
 *
 * This module plays WAV audio data through the ISD1820 module or DAC
 *
 * @author  Snake Game Team
 * @date    2026
 ******************************************************************************
 */

#include "audio_playback.h"
#include "audio_data.h"

/**
 * @brief Initialize audio playback
 *
 * Sets up GPIO and timers for audio output
 */
void AudioPlayback_Init(void)
{
    /* GPIO already initialized in main.c for ISD1820 */
    /* Add additional initialization if using DAC */
}

/**
 * @brief Play WAV audio through ISD1820
 *
 * This function triggers ISD1820 to play pre-recorded audio.
 * Use with audio recorded directly to ISD1820 via REC pin.
 */
void AudioPlayback_PlayISD1820(void)
{
    /* This is now handled by Snake_PlayMusic() in main.c */
    extern void Snake_PlayMusic(void);
    Snake_PlayMusic();
}

/**
 * @brief Get audio data pointer and size
 *
 * @param audio_data Pointer to audio data array
 * @param size Size of audio data in bytes
 * @return 0 if audio data available, -1 if empty
 */
int AudioPlayback_GetAudioData(const uint8_t **audio_data, uint32_t *size)
{
    /* Check if audio data is placeholder */
    if (gameOverAudioSize <= 4)
    {
        /* Audio data is just placeholder, use ISD1820 instead */
        return -1;
    }

    *audio_data = gameOverAudioData;
    *size = gameOverAudioSize;
    return 0;
}

/**
 * @brief Get audio format information
 *
 * @param format Pointer to audio format structure
 */
void AudioPlayback_GetAudioFormat(AudioFormat_t *format)
{
    format->channels = gameOverAudioChannels;
    format->bits_per_sample = gameOverAudioBitsPerSample;
    format->sample_rate = gameOverAudioSampleRate;
    format->duration_ms = (gameOverAudioSize * 8) /
                          (gameOverAudioBitsPerSample * gameOverAudioChannels *
                           gameOverAudioSampleRate / 1000);
}

/**
 * @brief Print audio information (for debugging)
 */
void AudioPlayback_PrintInfo(void)
{
    AudioFormat_t fmt;
    AudioPlayback_GetAudioFormat(&fmt);

#ifdef DEBUG
    printf("Game Over Audio Format:\n");
    printf("  Channels: %d\n", fmt.channels);
    printf("  Bits per Sample: %d\n", fmt.bits_per_sample);
    printf("  Sample Rate: %lu Hz\n", fmt.sample_rate);
    printf("  Data Size: %lu bytes\n", gameOverAudioSize);
    printf("  Duration: %lu ms\n", fmt.duration_ms);
#endif
}
