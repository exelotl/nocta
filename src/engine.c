#include "common.h"

static int samples_per_beat(int sample_rate, int bpm);
static int samples_per_line(int sample_rate, int bpm);

nocta_engine* nocta_engine_new(int sample_rate) {
	nocta_engine* self = malloc(sizeof(nocta_engine));
	*self = (nocta_engine) {
		.sample_rate = sample_rate,
		.tempo = 120,
		.ticks_per_line = 12,
		.samples_per_tick = samples_per_line(sample_rate, 120) / 12,
		.units = malloc(sizeof(nocta_unit*) * NOCTA_MAX_UNITS),
		.num_units = 0
	};
	return self;
}

bool nocta_engine_add(nocta_engine* self, nocta_unit* unit) {
	
}

bool nocta_engine_remove(nocta_engine* self, nocta_unit* unit) {
	
}

void nocta_engine_run(nocta_engine* self, int16_t* buffer, size_t len) {
	
}

void nocta_engine_free(nocta_engine* self) {
	free(self->units);
	free(self);
}


static int samples_per_beat(int sample_rate, int bpm) {
	return (sample_rate*60) / bpm;
}
static int samples_per_line(int sample_rate, int bpm) {
	// assume 4 lines per beat
	return samples_per_beat(sample_rate, bpm) / 4;
}
