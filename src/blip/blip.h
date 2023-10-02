#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint32_t length;     // Length in ticks
	uint32_t phase_step; // Phase step - phase state is incremented by this value on each sample
} blip_note_t;

typedef struct
{
	uint8_t bpm;               // Current BPM, set by 't' command
	uint8_t octave;            // Current octave, set by 'o' command
	uint8_t volume;            // Current volume, set by 'v' command
	uint32_t default_length;   // Default note length, set by 'l' command
	blip_note_t note;          // Current note
	bool add_length;           // Add next note length to the current note length, set by '&' command
	uint32_t phase;            // Waveform phase, 5.27 fixed point
	uint32_t ticks_per_16th;   // Ticks per 16th note, calculated from BPM and sample rate
	uint32_t ticks_per_sample; // How many ticks are in one sample, calculated from BPM and sample rate
} blip_state_t;

typedef struct
{
	const char* notes;    // Song string
	uint32_t sample_rate; // Sample rate
	blip_state_t state;   // Current song state (do not modify, unless you know what you're doing)
} blip_song_t;

// Initialize song state
bool blip_init_song( blip_song_t* song, const char* notes, size_t sample_rate );

int blip_mix_samples_f32( blip_song_t* song, float* samples, size_t samples_length );
int blip_mix_samples_i16( blip_song_t* song, int16_t* samples, size_t samples_length );

#ifdef __cplusplus
}
#endif
