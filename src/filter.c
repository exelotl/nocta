#include "common.h"

// Butterworth-style filter, adapted from the algorithm
// posted to the musicdsp archive by Patrice Tarrabia
// http://www.musicdsp.org/showone.php?id=38

/*
r  = rez amount, from sqrt(2) to ~ 0.1
f  = cutoff frequency
(from ~0 Hz to SampleRate/2 - though many
synths seem to filter only  up to SampleRate/4)

The filter algo:
out(n) = a1 * in + a2 * in(n-1) + a3 * in(n-2) 
       - b1*out(n-1) - b2*out(n-2)

Lowpass:
      c = 1.0 / tan(pi * f / sample_rate);

      a1 = 1.0 / ( 1.0 + r * c + c * c);
      a2 = 2* a1;
      a3 = a1;
      b1 = 2.0 * ( 1.0 - c*c) * a1;
      b2 = ( 1.0 - r * c + c * c) * a1;

Hipass:
      c = tan(pi * f / sample_rate);

      a1 = 1.0 / ( 1.0 + r * c + c * c);
      a2 = -2*a1;
      a3 = a1;
      b1 = 2.0 * ( c*c - 1.0) * a1;
      b2 = ( 1.0 - r * c + c * c) * a1;
*/

#define NUM_PASSES 2
#define NUM_PASSES_SQ (NUM_PASSES*NUM_PASSES)

typedef struct {
	int64_t in1, in2;   // values of the previous 2 input samples
	int64_t out1, out2; // values of the previous 2 output samples
} filter_channel;

typedef struct {
	int64_t a1, a2, a3;
	int64_t b1, b2;
	filter_channel l[NUM_PASSES], r[NUM_PASSES];
} filter_instance;

typedef struct {
	uint8_t vol;
	int mode;
	int freq;
	uint8_t res;
	uint8_t bandwidth;
	filter_instance lopass, hipass;
} filter_data;

// to update the filter coefficients when the parameters are changed:
static void update_current(nocta_unit* self);
static void update_lopass(filter_instance* f, int sample_rate, int64_t freq, int64_t res);
static void update_hipass(filter_instance* f, int sample_rate, int64_t freq, int64_t res);

// get the next sample
static int filter_run(filter_instance* filter, filter_channel* channel, int64_t input);
static int filter_run_current_l(filter_data* data, int x);
static int filter_run_current_r(filter_data* data, int x);

void nocta_filter_init(nocta_unit* self) {
	self->name = "filter";
	self->process = nocta_process_filter;
	
	filter_data* data = malloc(sizeof(filter_data));
	*data = (filter_data) {
		.vol = 128,
		.mode = NOCTA_FILTER_LOPASS,
		.freq = 22050,
		.res = 0,
		.lopass = (filter_instance){},
		.hipass = (filter_instance){}
	};
	self->data = data;
}

void nocta_process_filter(nocta_unit* self, int16_t* buffer, size_t length) {
	filter_data* data = self->data;
	
	int16_t* sample = buffer;
	
	for (int i=length/2; i>0; i--) {
		int val = *sample;
		val = (val * data->vol) >> 8;
		*sample = filter_run_current_l(data, val);
		sample++;
		
		val = *sample;
		val = (val * data->vol) >> 8;
		*sample = filter_run_current_r(data, val);
		sample++;
	}
}

static int filter_run_current_l(filter_data* data, int x) {
	for (int i=0; i<NUM_PASSES; i++) {
		if (data->mode == NOCTA_FILTER_LOPASS) {
			x = filter_run(&data->lopass, &data->lopass.l[i], x);
		} else if (data->mode == NOCTA_FILTER_HIPASS) {
			x = filter_run(&data->hipass, &data->hipass.l[i], x);
		} else if (data->mode == NOCTA_FILTER_BANDPASS) {
			x = filter_run(&data->lopass, &data->lopass.l[i], x);
			x = filter_run(&data->hipass, &data->hipass.l[i], x);
		}
	}
	return x;
}
static int filter_run_current_r(filter_data* data, int x) {
	for (int i=0; i<NUM_PASSES; i++) {
		if (data->mode == NOCTA_FILTER_LOPASS) {
			x = filter_run(&data->lopass, &data->lopass.r[i], x);
		} else if (data->mode == NOCTA_FILTER_HIPASS) {
			x = filter_run(&data->hipass, &data->hipass.r[i], x);
		} else if (data->mode == NOCTA_FILTER_BANDPASS) {
			x = filter_run(&data->lopass, &data->lopass.r[i], x);
			x = filter_run(&data->hipass, &data->hipass.r[i], x);
		}
	}
	return x;
}

static int filter_run(filter_instance* filter, filter_channel* channel, int64_t input) {
	input <<= FIX_P - 13;
	int64_t output = fix_mul(filter->a1, input);
	output += fix_mul(filter->a2, channel->in1);
	output += fix_mul(filter->a3, channel->in2);
	output -= fix_mul(filter->b1, channel->out1);
	output -= fix_mul(filter->b2, channel->out2);
	channel->in2 = channel->in1;
	channel->in1 = input;
	channel->out2 = channel->out1;
	channel->out1 = output;
	return output >> FIX_P - 13;
}

static void update_lopass(filter_instance* f, int sample_rate, int64_t freq, int64_t res) {
	freq = int_to_fix(freq);
	
	int64_t c = fix_div(FIX_1, fix_tan(fix_mul(FIX_PI, freq / sample_rate)));
	int64_t cc = fix_mul(c, c);
	int64_t rc = fix_mul(res, c);

	f->a1 = fix_div(FIX_1, FIX_1 + rc + cc);
	f->a2 = 2 * f->a1;
	f->a3 = f->a1;
	f->b1 = 2 * fix_mul(FIX_1 - cc, f->a1);
	f->b2 = fix_mul(FIX_1 - rc + cc, f->a1);
}

static void update_hipass(filter_instance* f, int sample_rate, int64_t freq, int64_t res) {
	freq = int_to_fix(freq);
	
	int64_t c = fix_tan(fix_mul(FIX_PI, freq / sample_rate));
	int64_t cc = fix_mul(c, c);
	int64_t rc = fix_mul(res, c);
	
	f->a1 = fix_div(FIX_1, FIX_1 + rc + cc);
	f->a2 = -2 * f->a1;
	f->a3 = f->a1;
	f->b1 = 2 * fix_mul(cc - FIX_1, f->a1);
	f->b2 = fix_mul(FIX_1 - rc + cc, f->a1);
}

static void update_current(nocta_unit* self) {
	int rate = self->engine->sample_rate;
	filter_data* data = self->data;
	
	// resonance is between sqrt(2) (lowest) and 0.1 (highest)
	int64_t res = (255 - data->res) << FIX_P-8;
	res = fix_mul(res, FIX_SQRT2 - NUM_PASSES*FIX_1/6);
	res += NUM_PASSES * FIX_1/6;
	
	switch (data->mode) {
		case NOCTA_FILTER_LOPASS:
			update_lopass(&data->lopass, rate, data->freq, res);
			break;
		case NOCTA_FILTER_HIPASS:
			update_hipass(&data->hipass, rate, data->freq, res);
			break;
		case NOCTA_FILTER_BANDPASS:
			update_lopass(&data->lopass, rate, fix_mul(data->freq, FIX_1-FIX_1/6), res/(NUM_PASSES_SQ*2));
			update_hipass(&data->hipass, rate, fix_mul(data->freq, FIX_1+FIX_1/6), res/(NUM_PASSES_SQ*2));
			break;
	}
}

// getters and setters:

uint8_t nocta_filter_vol(nocta_unit* self) {
	filter_data* data = self->data;
	return data->vol;
}
void nocta_filter_set_vol(nocta_unit* self, uint8_t vol) {
	filter_data* data = self->data;
	data->vol = vol;
}

int nocta_filter_mode(nocta_unit* self) {
	filter_data* data = self->data;
	return data->mode;
}
void nocta_filter_set_mode(nocta_unit* self, int mode) {
	filter_data* data = self->data;
	
	if (mode < 0 || mode >= NOCTA_FILTER_NUM_MODES)
		mode = NOCTA_FILTER_LOPASS;
	data->mode = mode;
	update_current(self);
}

uint16_t nocta_filter_freq(nocta_unit* self) {
	filter_data* data = self->data;
	return data->freq;
}
void nocta_filter_set_freq(nocta_unit* self, uint16_t freq) {
	filter_data* data = self->data;
	
	if (freq < 100) freq = 100;
	if (freq > 22050) freq = 22050;
	data->freq = freq;
	update_current(self);
}

uint8_t nocta_filter_res(nocta_unit* self) {
	filter_data* data = self->data;
	return data->res;
}
void nocta_filter_set_res(nocta_unit* self, uint8_t res) {
	filter_data* data = self->data;
	data->res = res;
	update_current(self);
}
