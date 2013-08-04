#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

//typedef void (*nocta_process)(int16_t* buffer, size_t length);

#define NOCTA_MAX_SOURCES 16

enum {
	NOCTA_FILTER_LOPASS,
	NOCTA_FILTER_HIPASS,
	NOCTA_FILTER_BANDPASS,
	
	NOCTA_FILTER_NUM_MODES
};


typedef struct nocta_engine {
	int sample_rate;
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

void     nocta_filter_init(nocta_unit* self);
uint8_t  nocta_filter_vol(nocta_unit* self);
int      nocta_filter_mode(nocta_unit* self);
uint16_t nocta_filter_freq(nocta_unit* self);
uint8_t  nocta_filter_res(nocta_unit* self);
void     nocta_filter_set_vol(nocta_unit* self, uint8_t vol);
void     nocta_filter_set_mode(nocta_unit* self, int mode);
void     nocta_filter_set_freq(nocta_unit* self, uint16_t freq);
void     nocta_filter_set_res(nocta_unit* self, uint8_t res);

//void nocta_generator_init(nocta_unit* self);

void nocta_process_gainer(nocta_unit* self, int16_t* buffer, size_t length);
void nocta_process_filter(nocta_unit* self, int16_t* buffer, size_t length);

