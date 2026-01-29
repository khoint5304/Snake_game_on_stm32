#!/usr/bin/env python3
"""
WAV to C Array Converter for STM32 Audio Playback
Converts WAV files to C header files with PCM data arrays

Usage:
    python3 wav_to_c_array.py input.wav output.h [variable_name]

Example:
    python3 wav_to_c_array.py gameover.wav gameover_audio.h gameOverAudio
"""

import sys
import struct
import wave
import os

def read_wav_file(filename):
    """Read WAV file and return PCM data"""
    try:
        with wave.open(filename, 'rb') as wav_file:
            # Get WAV parameters
            n_channels = wav_file.getnchannels()
            sample_width = wav_file.getsampwidth()
            frame_rate = wav_file.getframerate()
            n_frames = wav_file.getnframes()
            
            # Read audio data
            audio_data = wav_file.readframes(n_frames)
            
            # Verify WAV format
            print(f"WAV File Info:")
            print(f"  Channels: {n_channels}")
            print(f"  Sample Width: {sample_width} bytes")
            print(f"  Sample Rate: {frame_rate} Hz")
            print(f"  Total Frames: {n_frames}")
            print(f"  Duration: {n_frames / frame_rate:.2f} seconds")
            print(f"  Data Size: {len(audio_data)} bytes")
            
            # Check format compatibility
            if n_channels not in [1, 2]:
                raise ValueError(f"Only mono (1) or stereo (2) channels supported, got {n_channels}")
            if sample_width not in [1, 2]:
                raise ValueError(f"Only 8-bit or 16-bit samples supported, got {sample_width} bytes")
            if frame_rate not in [8000, 12000, 16000, 22050, 44100, 48000]:
                print(f"Warning: Sample rate {frame_rate} Hz may not be optimal for embedded systems")
            
            return audio_data, n_channels, sample_width, frame_rate, n_frames
            
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found!")
        return None, 0, 0, 0, 0
    except wave.Error as e:
        print(f"Error: Failed to read WAV file: {e}")
        return None, 0, 0, 0, 0

def bytes_to_hex_string(data, bytes_per_line=16):
    """Convert bytes to hexadecimal string with proper formatting"""
    hex_lines = []
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i+bytes_per_line]
        hex_values = [f"0x{byte:02x}" for byte in chunk]
        hex_lines.append(", ".join(hex_values))
    return ",\n    ".join(hex_lines)

def generate_c_header(audio_data, n_channels, sample_width, frame_rate, n_frames, variable_name):
    """Generate C header file content"""
    header_guard = f"{variable_name.upper()}_H"
    
    hex_data = bytes_to_hex_string(audio_data)
    data_size = len(audio_data)
    
    c_code = f"""/* Auto-generated WAV audio data */
#ifndef {header_guard}
#define {header_guard}

#include <stdint.h>

/**
 * @brief WAV Audio Data - {variable_name}
 * 
 * Audio Format:
 *   Channels: {n_channels} {'(Mono)' if n_channels == 1 else '(Stereo)'}
 *   Sample Rate: {frame_rate} Hz
 *   Sample Width: {sample_width * 8}-bit
 *   Total Frames: {n_frames}
 *   Duration: {n_frames / frame_rate:.2f} seconds
 *   Data Size: {data_size} bytes
 * 
 * For DAC Output: Convert to 16-bit stereo at 16kHz if needed
 */
const uint8_t {variable_name}Data[] = {{
    {hex_data}
}};

const uint32_t {variable_name}Size = sizeof({variable_name}Data);
const uint8_t {variable_name}Channels = {n_channels};
const uint8_t {variable_name}BitsPerSample = {sample_width * 8};
const uint32_t {variable_name}SampleRate = {frame_rate};

#endif /* {header_guard} */
"""
    return c_code

def main():
    if len(sys.argv) < 3:
        print("WAV to C Array Converter")
        print("")
        print("Usage: python3 wav_to_c_array.py <input.wav> <output.h> [variable_name]")
        print("")
        print("Arguments:")
        print("  input.wav      - Input WAV file to convert")
        print("  output.h       - Output C header file")
        print("  variable_name  - Variable name prefix (optional, default: 'audio')")
        print("")
        print("Example:")
        print("  python3 wav_to_c_array.py gameover.wav gameover_audio.h gameOverAudio")
        print("")
        print("Supported Formats:")
        print("  - Channels: Mono (1) or Stereo (2)")
        print("  - Sample Width: 8-bit or 16-bit")
        print("  - Sample Rates: 8kHz, 12kHz, 16kHz, 22.05kHz, 44.1kHz, 48kHz")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    variable_name = sys.argv[3] if len(sys.argv) > 3 else "audio"
    
    # Read WAV file
    audio_data, n_channels, sample_width, frame_rate, n_frames = read_wav_file(input_file)
    
    if audio_data is None:
        sys.exit(1)
    
    # Generate C header
    c_code = generate_c_header(audio_data, n_channels, sample_width, frame_rate, n_frames, variable_name)
    
    # Write to output file
    try:
        with open(output_file, 'w') as f:
            f.write(c_code)
        print(f"\n✓ Successfully created: {output_file}")
        print(f"  Variable name: {variable_name}Data")
        print(f"  Data size: {len(audio_data)} bytes")
    except IOError as e:
        print(f"Error: Failed to write output file: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
