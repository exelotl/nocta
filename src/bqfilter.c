#include "common.h"

// Biquad filter, based on the algorithm presented in
// the Audio EQ Cookbook by Robert Bristow-Johnson.

// http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

static int get_vol(nocta_unit* self);
static int get_mode(nocta_unit* self);
static int get_freq(nocta_unit* self);
static int get_res(nocta_unit* self);
static void set_vol(nocta_unit* self, int vol);
static void set_mode(nocta_unit* self, int mode);
static void set_freq(nocta_unit* self, int freq);
static void set_res(nocta_unit* self, int res);

static nocta_param bqfilter_params[] = {
	{"volume", 0, 255, get_vol, set_vol},
	{"mode", 0, NOCTA_FILTER_NUM_MODES, get_mode, set_mode},
	{"frequency", 100, 22050, get_freq, set_freq},
	{"resoncance", 0, 255, get_res, set_res}
};

#define NUM_PASSES 2

typedef struct {
	int in1, in2;   // values of the previous 2 input samples
	int out1, out2; // values of the previous 2 output samples
} filter_state;

typedef struct {
	// properties:
	uint8_t vol;
	int mode;
	int freq;
	int amp;
	uint8_t res;
	
	// coefficients:
	int a0, a1, a2;
	int b0, b1, b2;
	
	filter_state l[NUM_PASSES], r[NUM_PASSES];
} filter_data;

// calculate the coefficients when frequency, resonance, etc are changed
static void update_coefficients(nocta_unit* self);

// get the next sample
static int bqfilter_run(filter_data* data, filter_state* state, int input);
static int bqfilter_l(nocta_unit* self, int x);
static int bqfilter_r(nocta_unit* self, int x);

nocta_unit* nocta_bqfilter(nocta_context* context) {
	
	filter_data* data = malloc(sizeof(filter_data));
	*data = (filter_data) {
		.vol = 255,
		.mode = NOCTA_FILTER_MODE_LOWPASS,
		.freq = 22050,
		.res = 0
	};
	
	return nocta_create(context,
		.name = "bqfilter",
		.data = data,
		.process_l = bqfilter_l,
		.process_r = bqfilter_r,
		.params = bqfilter_params,
		.num_params = NOCTA_FILTER_NUM_PARAMS);
}

static int bqfilter_l(nocta_unit* self, int x) {
	filter_data* data = self->data;
	x = x*data->amp >> 8;
	for (int i=0; i<NUM_PASSES; i++) {
		x = bqfilter_run(data, &data->l[i], x);
	}
	return x*data->vol >> 8;
}
static int bqfilter_r(nocta_unit* self, int x) {
	filter_data* data = self->data;
	x = x*data->amp >> 8;
	for (int i=0; i<NUM_PASSES; i++) {
		x = bqfilter_run(data, &data->r[i], x);
	}
	return x*data->vol >> 8;
}

static int bqfilter_run(filter_data* data, filter_state* state, int input) {
	int output = fix_mul(data->b0, input);
	output += fix_mul(data->b1, state->in1);
	output += fix_mul(data->b2, state->in2);
	output -= fix_mul(data->a1, state->out1);
	output -= fix_mul(data->a2, state->out2);
	state->in2 = state->in1;
	state->in1 = input;
	state->out2 = state->out1;
	state->out1 = output;
	return output;
}

static void update_coefficients(nocta_unit* self) {
	filter_data* data = self->data;
	
	int sample_rate = self->context->sample_rate;
	int w0 = (2 * FIX_PI * data->freq) / sample_rate;
	int cos_w0 = fix_cos(w0);
	int sin_w0 = fix_sin(w0);
	int res = 2*u8_to_fix(data->res) + FIX_1/10;
	int alpha = fix_div(sin_w0, res);
	
	switch (data->mode) {
		case NOCTA_FILTER_MODE_LOWPASS:
			data->amp = 200;
			data->b0 = (FIX_1 - cos_w0) / 2;
			data->b1 = FIX_1 - cos_w0;
			data->b2 = (FIX_1 - cos_w0) / 2;
			data->a0 = FIX_1 + alpha;
			data->a1 = -2 * cos_w0;
			data->a2 = FIX_1 - alpha;
			break;
		case NOCTA_FILTER_MODE_HIGHPASS:
			data->amp = 200 + (data->freq >> 5);
			data->b0 = (FIX_1 + cos_w0) / 2;
			data->b1 = -(FIX_1 + cos_w0);
			data->b2 = (FIX_1 + cos_w0) / 2;
			data->a0 = FIX_1 + alpha;
			data->a1 = -2 * cos_w0;
			data->a2 = FIX_1 - alpha;
			break;
		case NOCTA_FILTER_MODE_BANDPASS:
			data->amp = 255 + (data->freq >> 5);
			data->b0 = sin_w0/2;
			data->b1 = 0;
			data->b2 = -sin_w0/2;
			data->a0 = FIX_1 + alpha;
			data->a1 = -2 * cos_w0;
			data->a2 = FIX_1 - alpha;
			break;
		case NOCTA_FILTER_MODE_NOTCH:
			data->amp = 200;
			data->b0 = FIX_1;
			data->b1 = -2 * cos_w0;
			data->b2 = FIX_1;
			data->a0 = FIX_1 + alpha;
			data->a1 = -2 * cos_w0;
			data->a2 = FIX_1 - alpha;
			break;
	}
	
	// optimisation: divide the coefficients in advance, so it doesn't need to be done per-sample
	data->b0 = fix_div(data->b0, data->a0);
	data->b1 = fix_div(data->b1, data->a0);
	data->b2 = fix_div(data->b2, data->a0);
	data->a1 = fix_div(data->a1, data->a0);
	data->a2 = fix_div(data->a2, data->a0);
}

// getters and setters:

static int get_vol(nocta_unit* self) {
	filter_data* data = self->data;
	return data->vol;
}
static void set_vol(nocta_unit* self, int vol) {
	filter_data* data = self->data;
	data->vol = vol;
}

static int get_mode(nocta_unit* self) {
	filter_data* data = self->data;
	return data->mode;
}
static void set_mode(nocta_unit* self, int mode) {
	filter_data* data = self->data;
	data->mode = mode;
	update_coefficients(self);
}

static int get_freq(nocta_unit* self) {
	filter_data* data = self->data;
	return data->freq;
}
static void set_freq(nocta_unit* self, int freq) {
	filter_data* data = self->data;
	data->freq = freq;
	update_coefficients(self);
}

static int get_res(nocta_unit* self) {
	filter_data* data = self->data;
	return data->res;
}
static void set_res(nocta_unit* self, int res) {
	filter_data* data = self->data;
	data->res = res;
	update_coefficients(self);
}
