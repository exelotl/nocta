#include "common.h"

// max delay time in seconds:
#define MAX_TIME 4

int get_dry(nocta_unit* self);
void set_dry(nocta_unit* self, int dry);
int get_wet(nocta_unit* self);
void set_wet(nocta_unit* self, int wet);
int get_feedback(nocta_unit* self);
void set_feedback(nocta_unit* self, int feedback);
int get_time(nocta_unit* self);
void set_time(nocta_unit* self, int t);

static nocta_param delay_params[] = {
	{"dry", 0, 255, get_dry, set_dry},
	{"wet", 0, 255, get_wet, set_wet},
	{"feedback", 0, 255, get_feedback, set_feedback},
	{"time", 0, 255, get_time, set_time}
};

typedef struct {
	int16_t* pre_samples; // initial delay
	int16_t* samples;     // feedback delay
	int size;
	int i, j;             // i = current position, j = future position
} delay_buffer;

typedef struct {
	uint8_t dry, wet;
	uint8_t feedback;
	int delay_time;
	delay_buffer l, r;
	int sample_rate;
} delay_data;

static inline int delay_run(delay_data* data, delay_buffer* b, int x);
static int delay_l(nocta_unit* self, int x);
static int delay_r(nocta_unit* self, int x);
static void delay_free(nocta_unit* self);

nocta_unit* nocta_delay(nocta_context* context) {
	
	int buffer_size = 2 * MAX_TIME * context->sample_rate;
	int alloc_size = sizeof(int16_t) * buffer_size;
	
	delay_data* data = ialloc(delay_data,
		.dry = 255,
		.wet = 127,
		.feedback = 100,
		.l = (delay_buffer){ malloc(alloc_size), malloc(alloc_size), buffer_size },
		.r = (delay_buffer){ malloc(alloc_size), malloc(alloc_size), buffer_size },
		.sample_rate = context->sample_rate
	);
	
	nocta_unit* self = nocta_create(context, 
		.name = "delay",
		.data = data,
		.process_l = delay_l,
		.process_r = delay_r,
		.free = delay_free,
		.params = delay_params,
		.num_params = NOCTA_DELAY_NUM_PARAMS);
	
	set_time(self, 127);
	return self;
}

static void delay_free(nocta_unit* self) {
	delay_data* data = self->data;
	free(data->l.samples);
	free(data->r.samples);
}

static int delay_l(nocta_unit* self, int x) {
	delay_data* data = self->data;
	return delay_run(data, &data->l, x);
}

static int delay_r(nocta_unit* self, int x) {
	delay_data* data = self->data;
	return delay_run(data, &data->r, x);
}

static inline int delay_run(delay_data* data, delay_buffer* b, int in) {
	if (b->i >= b->size) b->i = 0;
	b->j = b->i - (data->delay_time * data->sample_rate >> 8);
	if (b->j < 0) b->j += b->size;
	
	b->pre_samples[b->i] = in;
	
	int out = b->pre_samples[b->j];
	out += data->feedback * b->samples[b->j] >> 8;
	
	b->samples[b->i] = out;
	b->i++;
	
	return (in * data->dry >> 8)
	     + (out * data->wet >> 8);
}


// getters and setters:

int get_dry(nocta_unit* self) {
	delay_data* data = self->data;
	return data->dry;
}
void set_dry(nocta_unit* self, int dry) {
	delay_data* data = self->data;
	data->dry = dry;
}

int get_wet(nocta_unit* self) {
	delay_data* data = self->data;
	return data->wet;
}
void set_wet(nocta_unit* self, int wet) {
	delay_data* data = self->data;
	data->wet = wet;
}

int get_feedback(nocta_unit* self) {
	delay_data* data = self->data;
	return data->feedback;
}
void set_feedback(nocta_unit* self, int feedback) {
	delay_data* data = self->data;
	data->feedback = feedback;
}

int get_time(nocta_unit* self) {
	delay_data* data = self->data;
	return data->delay_time;
}
void set_time(nocta_unit* self, int t) {
	delay_data* data = self->data;
	data->delay_time = CLAMP(t, 1, MAX_TIME*256 - 1);
}