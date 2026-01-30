#ifndef SNAKE_INTERFACE_H
#define SNAKE_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Update button states from hardware GPIO
     * @param up    true if UP button is pressed (PD4)
     * @param down  true if DOWN button is pressed (PD5)
     * @param left  true if LEFT button is pressed (PD6)
     * @param right true if RIGHT button is pressed (PD7)
     *
     * This function should be called periodically from main.c to update
     * the button states in the TouchGFX Model.
     */
    void Snake_UpdateButtonStates(int up, int down, int left, int right);

    /**
     * @brief Play buzzer sound
     * @param durationMs Duration in milliseconds (100=food, 300=bigfood, 1000=gameover)
     *
     * This function triggers the buzzer for the specified duration.
     * Called from TouchGFX when sound events occur.
     */
    void Snake_PlayBuzzer(int durationMs);

    /**
     * @brief Play music from ISD1820 audio module
     *
     * This function triggers the ISD1820 module to play the recorded audio.
     * Called from TouchGFX when game over occurs.
     *
     * WIRING: P-L pin of ISD1820 → PD12 on STM32
     */
    void Snake_PlayMusic(void);

    /* Recording functions removed - audio is already recorded to ISD1820 module
     * Use the REC button on ISD1820 module to record audio manually.
     */

    /**
     * @brief Get current system time in milliseconds
     * @return Current time in milliseconds since system start
     *
     * This provides real-time clock for BigFood timer.
     */
    uint32_t Snake_GetTickMs(void);

    /**
     * @brief Test ISD1820 playback (debug function)
     *
     * This function tests ISD1820 by playing recorded audio.
     * Use for hardware debugging and verification.
     */
    void Snake_TestISD1820Play(void);

    /* Snake_TestISD1820Record() removed - use REC button on module instead */

    /**
     * @brief Load high score from Flash storage
     * @return High score value (0 if not found or corrupted)
     *
     * This function loads the high score from internal Flash memory.
     * Data persists across resets and power cycles.
     */
    uint16_t Snake_LoadHighScore(void);

    /**
     * @brief Save high score to Flash storage
     * @param score: High score value to save
     *
     * This function saves the high score to internal Flash memory.
     * Only saves if the new score is higher than the stored one.
     */
    void Snake_SaveHighScore(uint16_t score);

    /**
     * @brief Initialize Flash storage system
     *
     * This should be called once at startup to initialize
     * the Flash storage module.
     */
    void Snake_InitStorage(void);

#ifdef __cplusplus
}
#endif

#endif // SNAKE_INTERFACE_H
