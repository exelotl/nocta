#include "common.h"

nocta_unit* nocta_unit_new(nocta_engine* engine) {
	nocta_unit* self = malloc(sizeof(nocta_unit));
	nocta_unit_init(self, engine);
	return self;
}

void nocta_unit_init(nocta_unit* self, nocta_engine* engine) {
	*self = (nocta_unit) {
		.name = "unit",
		.engine = engine,
		.process = NULL,
		.free = NULL,
		.num_sources = 0,
		.data = NULL
	};
}

void nocta_unit_free(nocta_unit* self) {
	if (self->free) self->free(self); // custom free routine
	if (self->data) free(self->data); // custom data
	free(self);
}

void nocta_unit_process(nocta_unit* self, int16_t* buffer, size_t length) {
	for (int i=0; i < self->num_sources; i++) {
		nocta_unit_process(self->sources[i], buffer, length);
	}
	if (self->process)
		self->process(self, buffer, length);
}

void nocta_unit_add(nocta_unit* self, nocta_unit* source) {
	if (self->num_sources >= NOCTA_MAX_SOURCES) return;
	if (self == source) return;
	if (nocta_unit_has_source(self, source)) return;
	self->sources[self->num_sources++] = source;
}

void nocta_unit_remove(nocta_unit* self, nocta_unit* source) {
	nocta_unit** sources = self->sources;

	for (int i=0; i < self->num_sources; i++) {
		if (sources[i] == source) {
			sources[i] = NULL;
			self->num_sources--;
		}
		if (sources[i] == NULL && i < NOCTA_MAX_SOURCES-1 && sources[i+1]) {
			sources[i] = sources[i+1];
			sources[i+1] = NULL;
		}
	}
}

bool nocta_unit_has_source(nocta_unit* self, nocta_unit* source) {
	for (int i=0; i < self->num_sources; i++) {
		if (self->sources[i] == source)
			return true;
	}
	return false;
}

void nocta_unit_source_each(nocta_unit* self, nocta_unit_iterator iterator) {
	for (int i=0; i < self->num_sources; i++) {
		if (!iterator(self))
			return;
	}
}

int nocta_unit_get(nocta_unit* self, int param_id) {
	if (param_id >= self->num_params)
		return 0;
	nocta_param* param = &self->params[param_id];
	return param->get(self);
}

void nocta_unit_set(nocta_unit* self, int param_id, int val) {
	if (param_id >= self->num_params)
		return;
	nocta_param* param = &self->params[param_id];
	param->set(self, val);
}

nocta_param* nocta_unit_get_param(nocta_unit* self, int param_id) {
	return param_id < self->num_params ? &self->params[param_id] : NULL;
}
