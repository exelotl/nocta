#include "common.h"

static int get_active(nocta_unit* self);
static void set_active(nocta_unit* self, int active);
static int get_vol(nocta_unit* self);
static void set_vol(nocta_unit* self, int vol);
static int get_freq(nocta_unit* self);
static void set_freq(nocta_unit* self, int freq);
static int get_wave(nocta_unit* self);
static void set_wave(nocta_unit* self, int wave);

static nocta_param osc_params[] = {
	{"active", 0, 1, get_active, set_active},
	{"vol", 0, 255, get_vol, set_vol},
	{"freq", 50, 20000, get_freq, set_freq},
	{"wave", 0, NOCTA_NUM_WAVES-1, get_wave, set_wave}
};

struct osc_data;
typedef struct osc_data osc_data;
typedef int (*wave_cb)(int);

struct osc_data {
	bool active;
	uint8_t vol;
	int freq;
	int wave;
	wave_cb callback;
	int pos;
};

static int saw(int t);
static int sine(int t);
static int square(int t);
static int triangle(int t);
static int noise(int t);

static wave_cb waves[] = { saw, sine, square, triangle, noise };

static int osc_process_l(nocta_unit* self, int in);
static int osc_process_r(nocta_unit* self, int in);

nocta_unit* nocta_osc(nocta_context* context) {
	
	return nocta_create(
		.context = context,
		.name = "osc",
		.data = ialloc(osc_data, 
			.active = false,
			.vol = 128,
			.freq = 440,
			.wave = 0,
			.callback = waves[0],
			.pos = 0
		),
		.process_l = osc_process_l,
		.process_r = osc_process_r,
		.params = osc_params,
		.num_params = NOCTA_OSC_NUM_PARAMS
	);
}


static int osc_process_l(nocta_unit* self, int x) {
	osc_data* data = self->data;
	int out = data->callback(data->pos);
	data->pos += normalize_hz(data->freq, self->context->sample_rate);
	int amp = u8_to_fix(data->vol);
	return fix_to_int((x + out) * amp);
}

// doesn't advance the phase of the oscillator
static int osc_process_r(nocta_unit* self, int x) {
	osc_data* data = self->data;
	int out  = data->callback(data->pos);
	int amp = u8_to_fix(data->vol);
	return fix_to_int((x + out) * amp);
}


static int saw(int t) {
	return (t % FIX_1 - FIX_1/2) * 2;
}
static int sine(int t) {
	return 0;
}
static int square(int t) {
	return (t % FIX_1 > FIX_1/2) ? FIX_1 : -FIX_1;
}
static int triangle(int t) {
	return (abs(t % FIX_1 - FIX_1/2) - FIX_1/4) * 4;
}
static int noise(int t) {
	return rand();
}


// getters and setters


static int get_active(nocta_unit* self) {
	osc_data* data = self->data;
	return data->active;
}
static void set_active(nocta_unit* self, int active) {
	osc_data* data = self->data;
	data->active = active;
}

static int get_vol(nocta_unit* self) {
	osc_data* data = self->data;
	return data->vol;
}
static void set_vol(nocta_unit* self, int vol) {
	osc_data* data = self->data;
	data->vol = vol;
}

static int get_freq(nocta_unit* self) {
	osc_data* data = self->data;
	return data->freq;
}
static void set_freq(nocta_unit* self, int freq) {
	osc_data* data = self->data;
	data->freq = freq;
}

static int get_wave(nocta_unit* self) {
	osc_data* data = self->data;
	return data->wave;
}
static void set_wave(nocta_unit* self, int wave) {
	osc_data* data = self->data;
	data->wave = wave;
	data->callback = waves[wave];
}
