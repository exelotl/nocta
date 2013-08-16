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
static void svfilter_process(nocta_unit* self, int16_t* buffer, size_t length);

void nocta_svfilter_init(nocta_unit* self) {
	self->name = "filter";
	self->process = svfilter_process;
	
	filter_data* data = malloc(sizeof(filter_data));
	*data = (filter_data) {
		.vol = 255,
		.freq = 7000,
		.res = 0,
	};
	self->data = data;
	nocta_svfilter_set_mode(self, NOCTA_FILTER_LOWPASS);
}

static void svfilter_process(nocta_unit* self, int16_t* buffer, size_t length) {
	filter_data* data = self->data;
	
	int16_t* sample = buffer;
	
	for (int i=length/2; i>0; i--) {
		int val = *sample;
		val = (val * data->vol) >> 8;
		val = svfilter_run(data, &data->l, val);
		*sample = clip(val);
		sample++;
		
		val = *sample;
		val = (val * data->vol) >> 8;
		val = svfilter_run(data, &data->r, val);
		*sample = clip(val);
		sample++;
	}
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
	return output;
}

// getters and setters:

uint8_t nocta_svfilter_vol(nocta_unit* self) {
	filter_data* data = self->data;
	return data->vol;
}
void nocta_svfilter_set_vol(nocta_unit* self, uint8_t vol) {
	filter_data* data = self->data;
	data->vol = vol;
}

int nocta_svfilter_mode(nocta_unit* self) {
	filter_data* data = self->data;
	return data->mode;
}
void nocta_svfilter_set_mode(nocta_unit* self, int mode) {
	filter_data* data = self->data;
	filter_state* l = &data->l;
	filter_state* r = &data->r;
	
	if (mode < 0 || mode >= NOCTA_NUM_FILTER_MODES)
		mode = NOCTA_FILTER_LOWPASS;
	data->mode = mode;
	
	switch (mode) {
		case NOCTA_FILTER_LOWPASS:
			l->out = &l->lp;
			r->out = &r->lp;
			break;
		case NOCTA_FILTER_HIGHPASS:
			l->out = &l->hp;
			r->out = &r->hp;
			break;
		case NOCTA_FILTER_BANDPASS:
			l->out = &l->bp;
			r->out = &r->bp;
			break;
		case NOCTA_FILTER_NOTCH:
			l->out = &l->n;
			r->out = &r->n;
			break;
	}
}

int nocta_svfilter_freq(nocta_unit* self) {
	filter_data* data = self->data;
	return data->freq;
}
void nocta_svfilter_set_freq(nocta_unit* self, int freq) {
	filter_data* data = self->data;
	freq = CLAMP(freq, 0, 10000);
	data->freq = freq;
	
	data->tuned_freq = 2 * fix_sin(FIX_PI * freq / (self->engine->sample_rate*2));
}

uint8_t nocta_svfilter_res(nocta_unit* self) {
	filter_data* data = self->data;
	return data->res;
}
void nocta_svfilter_set_res(nocta_unit* self, uint8_t res) {
	filter_data* data = self->data;
	data->res = res;
	
	res -= res/8; // max resonance is too harsh
    data->tuned_res = 2*u8_to_fix(255 - res);
}
