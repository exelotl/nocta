#include "common.h"

nocta_engine* nocta_engine_new(int sample_rate) {
	nocta_engine* self = malloc(sizeof(nocta_engine));
	nocta_engine_init(self, sample_rate);
	return self;
}

static int samples_per_beat(int sample_rate, int bpm);
static int samples_per_line(int sample_rate, int bpm);

void nocta_engine_init(nocta_engine* self, int sample_rate) {
	*self = (nocta_engine) {
		.sample_rate = sample_rate,
		.tempo = 120,
		.ticks_per_line = 6,
		.samples_per_tick = samples_per_line(sample_rate, 120) / 6
	};
}

void nocta_engine_free(nocta_engine* self) {
	free(self);
}


static int samples_per_beat(int sample_rate, int bpm) {
	return (sample_rate*60) / bpm;
}
static int samples_per_line(int sample_rate, int bpm) {
	// assume 4 lines per beat
	return samples_per_beat(sample_rate, bpm) / 4;
}
