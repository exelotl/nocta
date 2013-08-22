#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#define NOCTA_MAX_SOURCES 16

struct nocta_unit;
struct nocta_param;
typedef struct nocta_unit nocta_unit;
typedef struct nocta_param nocta_param;

enum {
	NOCTA_WAVE_SAW,
	NOCTA_WAVE_SINE,
	NOCTA_WAVE_SQUARE,
	NOCTA_WAVE_TRIANGLE,
	NOCTA_WAVE_NOISE,
	
	NOCTA_NUM_WAVES	
};

typedef struct {
	int sample_rate;
	int tempo;
	int ticks_per_line;
	int samples_per_tick;
} nocta_engine;

typedef struct {
	uint8_t* data;
	size_t size;
} nocta_serialized;

/**
 * A sound processing object
 */
struct nocta_unit {
	nocta_engine* engine;
	
	// custom data/functions for each type of unit:
	char* name;
	void* data;
	void (*process)(nocta_unit* self, int16_t* buffer, size_t len);
	void (*free)(nocta_unit* self);
	uint8_t* (*serialize)(void* data);
	
	nocta_param* params;
	int num_params;
	
	// list of units to recieve sound data from:
	nocta_unit* sources[NOCTA_MAX_SOURCES];
	int num_sources;
};


struct nocta_param {
	char* name;
	int min, max;
	int (*get)(nocta_unit* unit);
	void (*set)(nocta_unit* unit, int val);
};


nocta_engine* nocta_engine_new(int sample_rate);
void nocta_engine_init(nocta_engine* self, int sample_rate);
void nocta_engine_free(nocta_engine* self);

nocta_unit*  nocta_unit_new(nocta_engine* engine);
void nocta_unit_init(nocta_unit* self, nocta_engine* engine);
void nocta_unit_free(nocta_unit* self);
void nocta_unit_process(nocta_unit* self, int16_t* buffer, size_t length);
void nocta_unit_add(nocta_unit* self, nocta_unit* source);
void nocta_unit_remove(nocta_unit* self, nocta_unit* source);
bool nocta_unit_has_source(nocta_unit* self, nocta_unit* source);
int nocta_unit_get(nocta_unit* self, int param_id);
void nocta_unit_set(nocta_unit* self, int param_id, int val);
nocta_param* nocta_unit_get_param(nocta_unit* self, int param_id);


typedef bool (*nocta_unit_iterator)(nocta_unit* unit);
void nocta_unit_source_each(nocta_unit* self, nocta_unit_iterator iterator);

void nocta_output_init(nocta_unit* self);


// Gainer:
// amplifies or attenuates a sound signal
// also used as a panning control
void nocta_gainer_init(nocta_unit* self);

enum {
	NOCTA_GAINER_VOL,       // amplitude from 0 to 255, where 128 = 100%
	NOCTA_GAINER_PAN,       // stereo panning, from -127 (hard left) to 127 (hard right)
	NOCTA_GAINER_NUM_PARAMS
};

// Biquad Filter:
// better stability than the state variable filter
// can be used at any sample rate
// cutoff frequency ranges from 100 to 22050 Hz
void nocta_bqfilter_init(nocta_unit* self);

// State Variable Filter:
// cleaner, more pleasant sound than the biquad
// becomes unstable at 1/3 of the sample-rate
// cutoff frequency is capped at 10000 Hz
void nocta_svfilter_init(nocta_unit* self);

enum {
	NOCTA_FILTER_VOL,       // amplitude from 0 to 255, where 128 = 100%
	NOCTA_FILTER_MODE,      // type of filter (see the filter modes below)
	NOCTA_FILTER_FREQ,      // cutoff frequency
	NOCTA_FILTER_RES,       // resonance, from 0 (none) to 255 (maximum)
	NOCTA_FILTER_NUM_PARAMS
};

// filter modes:
enum {
	NOCTA_FILTER_MODE_LOWPASS,
	NOCTA_FILTER_MODE_HIGHPASS,
	NOCTA_FILTER_MODE_BANDPASS,
	NOCTA_FILTER_MODE_NOTCH,
	NOCTA_FILTER_NUM_MODES
};

// Delay/echo:
void    nocta_delay_init(nocta_unit* self);

enum {
	NOCTA_DELAY_DRY,
	NOCTA_DELAY_WET,
	NOCTA_DELAY_FEEDBACK,
	NOCTA_DELAY_TIME,
	NOCTA_DELAY_NUM_PARAMS
};

void    nocta_osc_init(nocta_unit* self);
int     nocta_osc_unused_voice(nocta_unit* self);
void    nocta_osc_note_on(nocta_unit* self, int voice, uint8_t note, uint8_t vol);
void    nocta_osc_note_off(nocta_unit* self, int voice);

/*
// Potential commands:
// Axy - Arpeggio (x semitones, y semitones)
// Uxx - Portamento up
// Dxx - Portamento down
// Pxx - Portamento to note
// Rxx - Retrigger
// Ixx - Volume fade in
// Oxx - Volume fade out
// Cxx - Note cut
// Qxx - Note delay/quantize
void    nocta_osc_command(nocta_unit* self, int voice, char command, uint8_t value);
int     nocta_osc_wave(nocta_unit* self);
void    nocta_osc_adsr(nocta_unit* self, int env, int* a, int* d, int* s, int* r);
int     nocta_osc_attack(nocta_unit* self, int env);
int     nocta_osc_decay(nocta_unit* self, int env);
int     nocta_osc_sustain(nocta_unit* self, int env);
int     nocta_osc_release(nocta_unit* self, int env);
void    nocta_osc_set_wave(nocta_unit* self, int wave);
void    nocta_osc_set_adsr(nocta_unit* self, int a, int d, int s, int r);
void    nocta_osc_set_attack(nocta_unit* self, int val);
void    nocta_osc_set_decay(nocta_unit* self, int val);
void    nocta_osc_set_sustain(nocta_unit* self, int val);
void    nocta_osc_set_release(nocta_unit* self, int val);
*/
