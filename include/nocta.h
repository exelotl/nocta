#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

// Every sound unit refers to an instance of this
typedef struct {
	int sample_rate;
} nocta_context;

// A sound processing object
struct nocta_unit;
typedef struct nocta_unit nocta_unit;

// Defines the getters, setters, minimum and maximum values for a parameter
struct nocta_param;
typedef struct nocta_param nocta_param;


struct nocta_unit {
	nocta_context* context;
	
	// custom data/functions for each type of unit:
	char* name;
	void* data;
	int (*process_l)(nocta_unit* self, int l);
	int (*process_r)(nocta_unit* self, int r);
	void (*free)(nocta_unit* self);
	
	nocta_param* params;
	int num_params;
};

struct nocta_param {
	char* name;
	int min, max;
	int (*get)(nocta_unit* unit);
	void (*set)(nocta_unit* unit, int val);
};

// Create a new sound unit
#define nocta_create(...) nocta_create_impl((nocta_unit) { __VA_ARGS__ })
nocta_unit* nocta_create_impl(nocta_unit base);

void nocta_free(nocta_unit* self);

// Process two samples (one in each channel)
void nocta_process(nocta_unit* self, int16_t* l, int16_t* r);

// Process a block of interleaved stereo samples
void nocta_process_buffer(nocta_unit* self, int16_t* buffer, size_t length);

// Get/set the value of a parameter
int nocta_get(nocta_unit* self, int param_id);
void nocta_set(nocta_unit* self, int param_id, int val);

// Get a parameter's definition
nocta_param* nocta_get_param(nocta_unit* self, int param_id);


// Gainer:
// amplifies or attenuates a sound signal
// also used as a panning control
nocta_unit* nocta_gainer(nocta_context* context);

enum {
	NOCTA_GAINER_VOL,       // amplitude from 0 to 255, where 128 = 100%
	NOCTA_GAINER_PAN,       // stereo panning, from -127 (hard left) to 127 (hard right)
	NOCTA_GAINER_NUM_PARAMS
};

// Biquad Filter:
// better stability than the state variable filter
// can be used at any sample rate
// cutoff frequency ranges from 100 to 22050 Hz
nocta_unit* nocta_bqfilter(nocta_context* context);

// State Variable Filter:
// cleaner, more pleasant sound than the biquad
// becomes unstable at 1/3 of the sample rate
// cutoff frequency is capped at 10000 Hz
nocta_unit* nocta_svfilter(nocta_context* context);

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
nocta_unit* nocta_delay(nocta_context* context);

enum {
	NOCTA_DELAY_DRY,
	NOCTA_DELAY_WET,
	NOCTA_DELAY_FEEDBACK,
	NOCTA_DELAY_TIME,
	NOCTA_DELAY_NUM_PARAMS
};

nocta_unit* nocta_osc(nocta_context* context);
void nocta_osc_on(nocta_unit* osc, uint8_t note);
void nocta_osc_off(nocta_unit* osc);
enum {
	NOCTA_OSC_VOL,
	NOCTA_OSC_WAVE
};

enum {
	NOCTA_WAVE_SAW,
	NOCTA_WAVE_SINE,
	NOCTA_WAVE_SQUARE,
	NOCTA_WAVE_TRIANGLE,
	NOCTA_WAVE_NOISE,
	
	NOCTA_NUM_WAVES	
};
