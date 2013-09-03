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

static void gainer_process(nocta_unit* self, int16_t* buffer, size_t length);

nocta_unit* nocta_gainer(nocta_context* context) {
	
	gainer_data* data = malloc(sizeof(gainer_data));
	data->vol = 128;
	data->pan = 0;
	
	return nocta_create(context,
		.name = "gainer",
		.data = data,
		.process = gainer_process,
		.params = gainer_params,
		.num_params = NOCTA_GAINER_NUM_PARAMS);
}

static void gainer_process(nocta_unit* self, int16_t* buffer, size_t length) {
	gainer_data* data = self->data;
	int amount_l = 255, amount_r = 255;
	
	if (data->pan < 0) amount_r += 2*data->pan;
	if (data->pan > 0) amount_l -= 2*data->pan;
	amount_l = (amount_l * data->vol) >> 8;
	amount_r = (amount_r * data->vol) >> 8;
	
	int16_t* sample = buffer;
	
	for (int i=length/2; i>0; i--) {
		*sample = clip(((int)(*sample) * amount_l) >> 7);
		sample++;
		*sample = clip(((int)(*sample) * amount_r) >> 7);
		sample++;
	}
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
