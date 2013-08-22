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
	int16_t* samples;
	int size;
	int i, j;
} delay_buffer;

typedef struct {
	uint8_t dry, wet;
	uint8_t feedback;
	int delay_time;
	delay_buffer l, r;
	int sample_rate;
} delay_data;

static void delay_process(nocta_unit* self, int16_t* buffer, size_t len);
static inline int delay_run(delay_data* data, delay_buffer* b, int x);
static void delay_free(nocta_unit* self);

void nocta_delay_init(nocta_unit* self) {
	self->name = "delay";
	self->process = delay_process;
	self->free = delay_free;
	self->params = delay_params;
	self->num_params = NOCTA_DELAY_NUM_PARAMS;
	
	int sample_rate = self->engine->sample_rate;
	
	delay_data* data = malloc(sizeof(delay_data));
	*data = (delay_data) {
		.dry = 255,
		.wet = 127,
		.feedback = 100,
		.l = (delay_buffer) {.size = 2*MAX_TIME*sample_rate},
		.r = (delay_buffer) {.size = 2*MAX_TIME*sample_rate},
		.sample_rate = sample_rate
	};
	data->l.samples = malloc(sizeof(int16_t) * data->l.size);
	data->r.samples = malloc(sizeof(int16_t) * data->r.size);
	self->data = data;
	
	set_time(self, 127);
}

static void delay_free(nocta_unit* self) {
	delay_data* data = self->data;
	free(data->l.samples);
	free(data->r.samples);
}

static void delay_process(nocta_unit* self, int16_t* buffer, size_t len) {
	delay_data* data = self->data;
	
	int16_t* sample = buffer;
	
	for (int i=len/2; i>0; i--) {
		int val = *sample;
		*sample = delay_run(data, &data->l, val);
		sample++;
		
		val = *sample;
		*sample = delay_run(data, &data->r, val);
		sample++;
	}
}

/*
if( i >= BufferSize )
    i = 0;

j = i - (fDlyTime * MaxDlyTime);

if( j < 0 )
    j += BufferSize;

Output = DlyBuffer[ i ] = Input + (DlyBuffer[ j ] * fFeedback);

i++;
*/

static inline int delay_run(delay_data* data, delay_buffer* b, int x) {
	if (b->i >= b->size) b->i = 0;
	b->j = b->i - (data->delay_time * data->sample_rate >> 8);
	if (b->j < 0) b->j += b->size;
	
	x += data->feedback * b->samples[b->j] >> 8;
	b->samples[b->i] = x;
	b->i++;
	return x;
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