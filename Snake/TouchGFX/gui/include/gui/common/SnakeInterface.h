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
     */
    void Snake_PlayMusic(void);

    /**
     * @brief Start recording audio to ISD1820
     *
     * This function pulls the REC pin low to start recording audio.
     * Call Snake_StopRecording() to stop recording.
     */
    void Snake_StartRecording(void);

    /**
     * @brief Stop recording audio to ISD1820
     *
     * This function releases the REC pin to stop recording audio.
     */
    void Snake_StopRecording(void);

    /**
     * @brief Get current system time in milliseconds
     * @return Current time in milliseconds since system start
     *
     * This provides real-time clock for BigFood timer.
     */
    uint32_t Snake_GetTickMs(void);

#ifdef __cplusplus
}
#endif

#endif // SNAKE_INTERFACE_H
