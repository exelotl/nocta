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

typedef struct {
	double in1, in2;   // values of the previous 2 input samples
	double out1, out2; // values of the previous 2 output samples
} filter_channel;

typedef struct {
	double a1, a2, a3;
	double b1, b2;
	double c;
	filter_channel l, r;
} filter_instance;

typedef struct {
	uint8_t vol;
	int mode;
	uint16_t freq;
	uint8_t res;
	filter_instance lopass, hipass;
} filter_data;

// to update the filter's internal values when the parameters are changed:
static void update_lopass(filter_data* data, double sample_rate);
static void update_hipass(filter_data* data, double sample_rate);
static void update_current(filter_data* data, double sample_rate);

static double filter_run(filter_instance* filter, filter_channel* channel, double input);

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
		// process the left channel:
		double dval = (double) val / INT16_MAX;
		dval = filter_run(&(data->lopass), &(data->lopass.l), dval);
		val = dval * INT16_MAX;
		// end
		val = (val * data->vol) >> 7;
		*sample = (int16_t) val;
		sample++;
		
		val = *sample;
		// process the right channel:
		dval = (double) val / INT16_MAX;
		dval = filter_run(&(data->lopass), &(data->lopass.r), dval);
		val = dval * INT16_MAX;
		// end
		val = (val * data->vol) >> 7;
		*sample = (int16_t) val;
		sample++;
	}
}
//out(n) = a1 * in + a2 * in(n-1) + a3 * in(n-2) 
//      - b1*out(n-1) - b2*out(n-2)
static double filter_run(filter_instance* filter, filter_channel* channel, double input) {
	double output = filter->a1 * input
	              + filter->a2 * channel->in1
	              + filter->a3 * channel->in2
	              - filter->b1 * channel->out1
	              - filter->b2 * channel->out2;
	channel->in2 = channel->in1;
	channel->in1 = input;
	channel->out2 = channel->out1;
	channel->out1 = output;
	return output;
}


static void update_lopass(filter_data* data, double sample_rate) {
	
	filter_instance* f = &(data->lopass);
	
	// resonance is between sqrt(2) (lowest) and 0.1 (highest)
	double r = 1.0 - (double)data->res / UINT8_MAX;
	r *= 1.4142 - 0.1;
	r += 0.1;
	
	double c = 1.0 / tan(M_PI * data->freq / sample_rate);
	f->c = c;
	
    f->a1 = 1.0 / (1.0 + r*c + c*c);
    f->a2 = 2 * f->a1;
    f->a3 = f->a1;
    f->b1 = 2.0 * (1.0 - c*c) * f->a1;
    f->b2 = (1.0 - r*c + c*c) * f->a1;
}

static void update_hipass(filter_data* data, double sample_rate) {
	
	filter_instance* f = &(data->hipass);
	
	// resonance is between sqrt(2) (lowest) and 0.1 (highest)
	double r = 1.0 - (double)data->res / UINT8_MAX;
	r *= 1.4142 - 0.1;
	r += 0.1;
	
	double c = tan(M_PI * data->freq / sample_rate);
	f->c = c;
	
	f->a1 = 1.0 / ( 1.0 + r*c + c*c);
	f->a2 = -2 * f->a1;
	f->a3 = f->a1;
	f->b1 = 2.0 * ( c*c - 1.0) * f->a1;
	f->b2 = (1.0 - r*c + c*c) * f->a1;
}

static void update_current(filter_data* data, double sample_rate) {
	switch (data->mode) {
		case NOCTA_FILTER_LOPASS:
			update_lopass(data, sample_rate);
			break;
		case NOCTA_FILTER_HIPASS:
			update_hipass(data, sample_rate);
			break;
		case NOCTA_FILTER_BANDPASS:
			update_lopass(data, sample_rate);
			update_hipass(data, sample_rate);
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
	
	update_current(data, self->engine->sample_rate);
}

uint16_t nocta_filter_freq(nocta_unit* self) {
	filter_data* data = self->data;
	return data->freq;
}
void nocta_filter_set_freq(nocta_unit* self, uint16_t freq) {
	filter_data* data = self->data;
	
	if (freq > 22050) freq = 22050;
	data->freq = freq;
	update_current(data, self->engine->sample_rate);
}

uint8_t nocta_filter_res(nocta_unit* self) {
	filter_data* data = self->data;
	return data->res;
}
void nocta_filter_set_res(nocta_unit* self, uint8_t res) {
	filter_data* data = self->data;
	data->res = res;
	update_current(data, self->engine->sample_rate);
}
