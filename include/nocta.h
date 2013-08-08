#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

//typedef void (*nocta_process)(int16_t* buffer, size_t length);

#define NOCTA_MAX_SOURCES 16

enum {
	NOCTA_FILTER_LOWPASS,
	NOCTA_FILTER_HIGHPASS,
	NOCTA_FILTER_BANDPASS,
	NOCTA_FILTER_NOTCH,
	
	NOCTA_FILTER_NUM_MODES
};


typedef struct nocta_engine {
	int sample_rate;
	int time_scale;
} nocta_engine;

/**
 * A sound processing object
 */
typedef struct nocta_unit {
	char* name;
	nocta_engine* engine;
	void (*process)(struct nocta_unit* self, int16_t* buffer, size_t len);
	struct nocta_unit* sources[NOCTA_MAX_SOURCES];
	int num_sources;
	void* data;
	uint8_t* (*serialize)(void* data);
} nocta_unit;

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
uint8_t nocta_delay_mix(nocta_unit* self);
uint8_t nocta_delay_feedback(nocta_unit* self);
int     nocta_delay_time(nocta_unit* self);
void    nocta_delay_set_mix(nocta_unit* self, uint8_t mix);
void    nocta_delay_set_feedback(nocta_unit* self, uint8_t feedback);
void    nocta_delay_set_time(nocta_unit* self, int time);

//void nocta_generator_init(nocta_unit* self);
