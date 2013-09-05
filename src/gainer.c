#include "common.h"

static int get_vol(nocta_unit* self);
static void set_vol(nocta_unit* self, int vol);
static int get_pan(nocta_unit* self);
static void set_pan(nocta_unit* self, int pan);

static nocta_param gainer_params[] = {
	{"vol",    0, 255, get_vol, set_vol},
	{"pan", -127, 127, get_pan, set_pan}
};

typedef struct {
	uint8_t vol;
	int8_t pan;
} gainer_data;

static int gainer_process_l(nocta_unit* self, int in);
static int gainer_process_r(nocta_unit* self, int in);

nocta_unit* nocta_gainer(nocta_context* context) {
	
	gainer_data* data = malloc(sizeof(gainer_data));
	data->vol = 128;
	data->pan = 0;
	
	return nocta_create(context,
		.name = "gainer",
		.data = data,
		.process_l = gainer_process_l,
		.process_r = gainer_process_r,
		.params = gainer_params,
		.num_params = NOCTA_GAINER_NUM_PARAMS);
}


static int gainer_process_l(nocta_unit* self, int in) {
	gainer_data* data = self->data;
	int amp = 255;
	if (data->pan > 0) amp -= 2 * data->pan;
	amp = amp * data->vol >> 8;
	return in * amp >> 7;
}

static int gainer_process_r(nocta_unit* self, int in) {
	gainer_data* data = self->data;
	int amp = 255;
	if (data->pan < 0) amp += 2 * data->pan;
	amp = amp * data->vol >> 8;
	return in * amp >> 7;
}


// getters and setters

static int get_vol(nocta_unit* self) {
	gainer_data* data = self->data;
	return data->vol;
}
static void set_vol(nocta_unit* self, int vol) {
	gainer_data* data = self->data;
	data->vol = vol;
}

static int get_pan(nocta_unit* self) {
	gainer_data* data = self->data;
	return data->pan;
}
static void set_pan(nocta_unit* self, int pan) {
	gainer_data* data = self->data;
	data->pan = pan;
}
