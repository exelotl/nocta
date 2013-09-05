#include "common.h"

/*
//Input/Output
    I - input sample
    L - lowpass output sample
    B - bandpass output sample
    H - highpass output sample
    N - notch output sample
    F1 - Frequency control parameter
    Q1 - Q control parameter
    
// parameters:
    Q1 = 1/Q   // where Q1 goes from 2 to 0, ie Q goes from .5 to infinity
    
    // frequency tuning:
    F1 = 2 * sin( pi * F / Fs )

// algorithm
    // loop
    L = L + F1 * B
    H = I - L - Q1*B
    B = F1 * H + B
    N = H + L
*/

static int get_vol(nocta_unit* self);
static int get_mode(nocta_unit* self);
static int get_freq(nocta_unit* self);
static int get_res(nocta_unit* self);
static void set_vol(nocta_unit* self, int vol);
static void set_mode(nocta_unit* self, int mode);
static void set_freq(nocta_unit* self, int freq);
static void set_res(nocta_unit* self, int res);

static nocta_param svfilter_params[] = {
	{"volume", 0, 255, get_vol, set_vol},
	{"mode", 0, NOCTA_FILTER_NUM_MODES, get_mode, set_mode},
	{"frequency", 0, 10000, get_freq, set_freq},
	{"resoncance", 0, 255, get_res, set_res}
};

typedef struct {
	int lp, hp, bp, n;
	int* out;
} filter_state;

typedef struct {
	uint8_t vol;
	int mode;
	int freq;
	uint8_t res;
	int tuned_freq;
	int tuned_res;
	filter_state l, r;
} filter_data;

// get the next sample
static int svfilter_run(filter_data* data, filter_state* state, int input);
static int svfilter_l(nocta_unit* self, int x);
static int svfilter_r(nocta_unit* self, int x);

nocta_unit* nocta_svfilter(nocta_context* context) {
	
	filter_data* data = malloc(sizeof(filter_data));
	*data = (filter_data) {
		.vol = 255,
		.freq = 7000,
		.res = 0,
	};
	
	nocta_unit* self = nocta_create(context, 
		.name = "svfilter",
		.data = data,
		.process_l = svfilter_l,
		.process_r = svfilter_r,
		.params = svfilter_params,
		.num_params = NOCTA_FILTER_NUM_PARAMS);
	
	set_mode(self, NOCTA_FILTER_MODE_LOWPASS);
	return self;
}

static int svfilter_l(nocta_unit* self, int x) {
	filter_data* data = self->data;
	return svfilter_run(data, &data->l, x);
}
static int svfilter_r(nocta_unit* self, int x) {
	filter_data* data = self->data;
	return svfilter_run(data, &data->r, x);
}

static int svfilter_run(filter_data* data, filter_state* s, int input) {
	int output = 0;
	for (int i=0; i<2; i++) {
		s->lp = s->lp + fix_mul(data->tuned_freq, s->bp);
		s->hp = input - s->lp - fix_mul(data->tuned_res, s->bp);
		s->bp = fix_mul(data->tuned_freq, s->hp) + s->bp;
		s->n = s->hp + s->lp;
		output += *s->out / 2;
	}
	return (output * data->vol) >> 8;
}

// getters and setters:

int get_vol(nocta_unit* self) {
	filter_data* data = self->data;
	return data->vol;
}
void set_vol(nocta_unit* self, int vol) {
	filter_data* data = self->data;
	data->vol = vol;
}

int get_mode(nocta_unit* self) {
	filter_data* data = self->data;
	return data->mode;
}
void set_mode(nocta_unit* self, int mode) {
	filter_data* data = self->data;
	filter_state* l = &data->l;
	filter_state* r = &data->r;
	data->mode = mode;
	
	switch (mode) {
		case NOCTA_FILTER_MODE_LOWPASS:
			l->out = &l->lp;
			r->out = &r->lp;
			break;
		case NOCTA_FILTER_MODE_HIGHPASS:
			l->out = &l->hp;
			r->out = &r->hp;
			break;
		case NOCTA_FILTER_MODE_BANDPASS:
			l->out = &l->bp;
			r->out = &r->bp;
			break;
		case NOCTA_FILTER_MODE_NOTCH:
			l->out = &l->n;
			r->out = &r->n;
			break;
	}
}

int get_freq(nocta_unit* self) {
	filter_data* data = self->data;
	return data->freq;
}
void set_freq(nocta_unit* self, int freq) {
	filter_data* data = self->data;
	data->freq = freq;
	data->tuned_freq = 2 * fix_sin(FIX_PI * freq / (self->context->sample_rate*2));
}

int get_res(nocta_unit* self) {
	filter_data* data = self->data;
	return data->res;
}
void set_res(nocta_unit* self, int res) {
	filter_data* data = self->data;
	data->res = res;
	
	res -= res/8; // max resonance is too harsh
    data->tuned_res = 2*u8_to_fix(255 - res);
}
