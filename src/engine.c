#include "common.h"

nocta_engine* nocta_engine_new(int sample_rate) {
	nocta_engine* self = malloc(sizeof(nocta_engine));
	nocta_engine_init(self, sample_rate);
	return self;
}

void nocta_engine_init(nocta_engine* self, int sample_rate) {
	*self = (nocta_engine) {
		.sample_rate = sample_rate
	};
}

void nocta_engine_free(nocta_engine* self) {
	free(self);
}
