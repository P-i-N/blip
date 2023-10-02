#include "blip.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define CLAMPED( x, minimum, maximum ) \
	( ( ( x ) < ( minimum ) ) ? ( minimum ) : ( ( x ) > ( maximum ) ) ? ( maximum ) : ( x ) )

#define GET_CURRENT_CH \
	while ( *( song->notes ) && ( *( song->notes ) ) <= 32 ) \
		( ++( song->notes ) ); \
	ch = tolower( *( song->notes ) )

#define CONSUME_CH ( ++( song->notes ) )

#define CONSUME_AND_GET_NEXT_CH \
	( ++( song->notes ) ); \
	GET_CURRENT_CH;

//----------------------------------------------------------------------------------------------------------------------
static bool decode_u8( blip_song_t* song, uint8_t* value, uint8_t max_value )
{
	char* end = NULL;
	*value = ( uint8_t )strtoul( song->notes, &end, 10 );
	if ( end == NULL || ( *value ) > max_value )
		return false;

	song->notes = end;
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
static uint32_t decode_length( blip_song_t* song, uint32_t ticks_per_16th )
{
	uint32_t result = song->state.default_length;

	char ch = 0;
	GET_CURRENT_CH;

	if ( ch >= '0' && ch <= '9' )
	{
		uint8_t len = 0;
		if ( !decode_u8( song, &len, 128 ) || len == 0 )
			return 0;

		result = ( ticks_per_16th * 16 ) / len;
		GET_CURRENT_CH;
	}

	uint32_t dot_length = result;
	while ( ch == '.' )
	{
		CONSUME_AND_GET_NEXT_CH;
		result += ( dot_length >>= 1 );
	}

	return result;
}

//----------------------------------------------------------------------------------------------------------------------
static void change_tempo( blip_state_t* s, uint8_t tempo, uint32_t sample_rate )
{
	if ( tempo == 0 )
		tempo = 120;
	else if ( tempo < 32 )
		tempo = 32;

	// Chosen by educated guessing, so tick counters can fit in 32-bit integers
	static const uint32_t tick_scale = 4096;

	// When changing tempo, existing default note length should be scaled accordingly
	uint32_t rescale_coef = ( s->ticks_per_16th / tick_scale );
	uint32_t old_default_length = rescale_coef ? ( s->default_length / rescale_coef ) : 0;
	uint32_t old_note_length = rescale_coef ? ( s->note.length / rescale_coef ) : 0;

	s->bpm = tempo;
	s->ticks_per_16th = tick_scale * ( ( 60 * sample_rate ) / s->bpm );
	s->ticks_per_sample = ( s->bpm * tick_scale ) / ( sample_rate / 1600 );

	rescale_coef = s->ticks_per_16th / tick_scale;
	s->default_length = old_default_length * rescale_coef;
	s->note.length = old_note_length * rescale_coef;
}

//----------------------------------------------------------------------------------------------------------------------
static bool decode_note( blip_song_t* song, blip_note_t* note )
{
	char ch = 0;
	GET_CURRENT_CH;

	int8_t note_value = 0;

	if ( ch == 'n' || ch == 'N' )
	{
		CONSUME_AND_GET_NEXT_CH;
		if ( !decode_u8( song, ( uint8_t* )( &note_value ), 95 ) )
			return false;
	}
	else
	{
		static const char* note_names = "cdefgabh";
		static const uint8_t semitones[] = { 0, 2, 4, 5, 7, 9, 11, 11 };
		const char* note_char = strchr( note_names, ch );
		if ( note_char == NULL )
			return false;

		CONSUME_AND_GET_NEXT_CH;
		uint8_t semitone = semitones[( uint8_t )( note_char - note_names )];
		note_value = ( 12 * song->state.octave ) + semitone;
	}

	while ( ch == '#' || ch == '+' || ch == '-' )
	{
		note_value += ( ch == '-' ) ? -1 : 1;

		CONSUME_AND_GET_NEXT_CH;
		note_value = CLAMPED( note_value, 0, 95 );
	}

	note->length = decode_length( song, song->state.ticks_per_16th );
	if ( note->length == 0 )
		return false;

	// 5.27 fixed point frequency table for notes of first octave (C0 to B0), equal temperament tuning
	static const uint32_t freq_table[12] = {
		0x82d01286, 0x8a976073, 0x92d5171d, 0x9b904100, 0xa4d053c8, 0xae9d36b0, //
		0xb8ff493e, 0xc3ff6a72, 0xcfa70054, 0xdc000000, 0xe914f623, 0xf6f11004,
	};

	note->phase_step = ( freq_table[note_value % 12] / song->sample_rate ) << ( note_value / 12 );
	return true;
}

// Sample mixing function typedef - used to mix single in16_t sample into a buffer of some other type
typedef void ( *mix_func_t )( void**, int16_t );

//----------------------------------------------------------------------------------------------------------------------
static void mix_f32( void** ptr, int16_t sample )
{
	float* ptr_f32 = *( ( float** )ptr );
	float value = ( *ptr_f32 ) + ( float )( sample / 32768.0f );
	*ptr_f32++ = CLAMPED( value, -1.0f, 1.0f );
	*ptr = ptr_f32;
}

//----------------------------------------------------------------------------------------------------------------------
static void mix_i16( void** ptr, int16_t sample )
{
	int16_t* ptr_i16 = *( ( int16_t** )ptr );
	int32_t value = ( int32_t )( *ptr_i16 ) + sample;
	*ptr_i16++ = ( int16_t )CLAMPED( value, -32768, 32767 );
	*ptr = ptr_i16;
}

//----------------------------------------------------------------------------------------------------------------------
static size_t generate_samples( blip_state_t* S, void** samples, size_t samples_length, const mix_func_t mix )
{
	size_t generated_samples = ( S->note.length + S->ticks_per_sample - 1 ) / S->ticks_per_sample;
	if ( generated_samples > samples_length )
		generated_samples = samples_length;

	uint32_t length = ( uint32_t )( generated_samples * S->ticks_per_sample );
	S->note.length -= ( length > S->note.length ) ? S->note.length : length;

	uint8_t volume = S->volume;

	samples_length = generated_samples;
	while ( samples_length-- )
	{
		uint8_t sample = ( S->phase >> 19 ) & 0xFF;
		mix( samples, ( ( sample - 128l ) * volume ) >> 4 );

		S->phase = ( S->phase + S->note.phase_step ) & 0x07FFFFFF;
	}

	return generated_samples;
}

//----------------------------------------------------------------------------------------------------------------------
int blip_mix_samples( blip_song_t* song, void* samples, size_t samples_length, const mix_func_t mix )
{
	if ( song == NULL )
		return -1;

	if ( samples == NULL )
		return 0;

	size_t generated_samples = 0;
	blip_state_t* S = &( song->state );

	while ( samples_length )
	{
		char ch = 0;
		GET_CURRENT_CH;

		if ( ch == '&' )
		{
			S->add_length = true;
			CONSUME_CH;
			continue;
		}

		if ( S->add_length == false )
		{
			size_t num_samples = generate_samples( S, &samples, samples_length, mix );
			samples_length -= num_samples;
			generated_samples += num_samples;

			if ( !samples_length )
				break;
		}

		// End of song or channel
		if ( S->note.length == 0 && ( ch == '\0' || ch == ',' || ch == ';' ) )
			break;

		CONSUME_CH;
		switch ( ch )
		{
			// Tempo
			case 't':
				if ( !decode_u8( song, &( S->bpm ), 255 ) || S->bpm == 0 )
					return -2;

				change_tempo( S, S->bpm, song->sample_rate );
				break;

			// Volume
			case 'v':
				if ( !decode_u8( song, &( S->volume ), 127 ) )
					return -2;
				break;

			// Octave
			case 'o':
				if ( !decode_u8( song, &( S->octave ), 7 ) )
					return -2;
				break;

			// Octave down
			case '<':
				if ( S->octave > 0 )
					S->octave -= 1;
				break;

			// Octave up
			case '>':
				if ( S->octave < 7 )
					S->octave += 1;
				break;

			// Length
			case 'l':
				S->default_length = decode_length( song, S->ticks_per_16th );
				if ( S->default_length == 0 )
					return -2;
				break;

			case 'r':
			case 'p': {
				uint32_t length = decode_length( song, S->ticks_per_16th );
				if ( length == 0 )
					return -2;

				if ( S->add_length )
				{
					S->note.length += length;
					S->add_length = false;
				}
				else
				{
					S->note.length = length;
					S->note.phase_step = 0;
				}
			}
			break;

			default: {
				--( song->notes ); // Rewind previously consumed note

				blip_note_t note;
				if ( decode_note( song, &note ) == false )
					return -2;

				if ( S->add_length )
				{
					S->note.length += note.length;
					S->add_length = false;
				}
				else
				{
					S->note = note;
					S->phase = 0;
				}
			}
			break;
		}
	}

	return ( int )generated_samples;
}

//----------------------------------------------------------------------------------------------------------------------
bool blip_init_song( blip_song_t* song, const char* notes, size_t sample_rate )
{
	if ( song == NULL || notes == NULL || sample_rate == 0 )
		return false;

	memset( song, 0, sizeof( blip_song_t ) );
	song->notes = notes;
	song->sample_rate = ( uint32_t )sample_rate;

	blip_state_t* S = &( song->state );

	// Set defaults: 120 BPM, 4th octave, full volume, 8th note default length
	change_tempo( S, 120, song->sample_rate );
	S->octave = 4;
	S->volume = 127;
	S->default_length = ( S->ticks_per_16th * 16 ) / 4;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
int blip_mix_samples_f32( blip_song_t* song, float* samples, size_t samples_length )
{
	return blip_mix_samples( song, samples, samples_length, mix_f32 );
}

//----------------------------------------------------------------------------------------------------------------------
int blip_mix_samples_i16( blip_song_t* song, int16_t* samples, size_t samples_length )
{
	return blip_mix_samples( song, samples, samples_length, mix_i16 );
}
