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
	NOCTA_FILTER_LOWPASS,
	NOCTA_FILTER_HIGHPASS,
	NOCTA_FILTER_BANDPASS,
	NOCTA_FILTER_NOTCH,
	
	NOCTA_NUM_FILTER_MODES
};

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


struct nocta_param {
	char* name;
	int min, max;
	int (*get)(struct nocta_unit* unit);
	void (*set)(struct nocta_unit* unit, int val);
};

/**
 * A sound processing object
 */
struct nocta_unit {
	nocta_engine* engine;
	
	// custom data/functions for each type of unit:
	char* name;
	void* data;
	void (*process)(struct nocta_unit* self, int16_t* buffer, size_t len);
	void (*free)(struct nocta_unit* self);
	uint8_t* (*serialize)(void* data);
	
	// list of subclasses:
	struct nocta_unit* sources[NOCTA_MAX_SOURCES];
	int num_sources;
};

nocta_engine* nocta_engine_new(int sample_rate);
void nocta_engine_init(nocta_engine* self, int sample_rate);
void nocta_engine_free(nocta_engine* self);

nocta_unit* nocta_unit_new(nocta_engine* engine);
void nocta_unit_init(nocta_unit* self, nocta_engine* engine);
void nocta_unit_free(nocta_unit* self);
void nocta_unit_process(nocta_unit* self, int16_t* buffer, size_t length);
void nocta_unit_add(nocta_unit* self, nocta_unit* source);
void nocta_unit_remove(nocta_unit* self, nocta_unit* source);
bool nocta_unit_has_source(nocta_unit* self, nocta_unit* source);

typedef bool (*nocta_unit_iterator)(nocta_unit* unit);
void nocta_unit_source_each(nocta_unit* self, nocta_unit_iterator iterator);

void nocta_output_init(nocta_unit* self);

void    nocta_gainer_init(nocta_unit* self);
uint8_t nocta_gainer_vol(nocta_unit* self);
int8_t  nocta_gainer_pan(nocta_unit* self);
void    nocta_gainer_set_vol(nocta_unit* self, uint8_t vol);
void    nocta_gainer_set_pan(nocta_unit* self, int8_t pan);

// Biquad Filter:
// better stability than the state variable filter
// can be used at any sample rate
// cutoff frequency ranges from 100 to 22050 Hz
void    nocta_bqfilter_init(nocta_unit* self);
uint8_t nocta_bqfilter_vol(nocta_unit* self);
int     nocta_bqfilter_mode(nocta_unit* self);
int     nocta_bqfilter_freq(nocta_unit* self);
uint8_t nocta_bqfilter_res(nocta_unit* self);
void    nocta_bqfilter_set_vol(nocta_unit* self, uint8_t vol);
void    nocta_bqfilter_set_mode(nocta_unit* self, int mode);
void    nocta_bqfilter_set_freq(nocta_unit* self, int freq);
void    nocta_bqfilter_set_res(nocta_unit* self, uint8_t res);

// State Variable Filter:
// cleaner, more pleasant sound than the biquad
// becomes unstable at 1/3 of the sample-rate
// cutoff frequency is capped at 10000 Hz
void    nocta_svfilter_init(nocta_unit* self);
uint8_t nocta_svfilter_vol(nocta_unit* self);
int     nocta_svfilter_mode(nocta_unit* self);
int     nocta_svfilter_freq(nocta_unit* self);
uint8_t nocta_svfilter_res(nocta_unit* self);
void    nocta_svfilter_set_vol(nocta_unit* self, uint8_t vol);
void    nocta_svfilter_set_mode(nocta_unit* self, int mode);
void    nocta_svfilter_set_freq(nocta_unit* self, int freq);
void    nocta_svfilter_set_res(nocta_unit* self, uint8_t res);

// Delay/echo:
void    nocta_delay_init(nocta_unit* self);
uint8_t nocta_delay_dry(nocta_unit* self);
uint8_t nocta_delay_wet(nocta_unit* self);
uint8_t nocta_delay_feedback(nocta_unit* self);
int     nocta_delay_time(nocta_unit* self);
void    nocta_delay_set_dry(nocta_unit* self, uint8_t dry);
void    nocta_delay_set_wet(nocta_unit* self, uint8_t wet);
void    nocta_delay_set_feedback(nocta_unit* self, uint8_t feedback);
void    nocta_delay_set_time(nocta_unit* self, int time);

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
// Dxx - Note delay
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
