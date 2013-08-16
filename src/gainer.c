#include "common.h"

typedef struct {
	uint8_t vol;
	int8_t pan;
} gainer_data;

static void gainer_process(nocta_unit* self, int16_t* buffer, size_t length);

void nocta_gainer_init(nocta_unit* self) {
	self->name = "gainer";
	self->process = gainer_process;
	
	gainer_data* data = malloc(sizeof(gainer_data));
	*data = (gainer_data) {
		.vol = 128,
		.pan = 0
	};
	self->data = data;
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
		*sample = ((int)(*sample) * amount_l) >> 7;
		sample++;
		*sample = ((int)(*sample) * amount_r) >> 7;
		sample++;
	}
}


// getters and setters

uint8_t nocta_gainer_vol(nocta_unit* self) {
	gainer_data* data = self->data;
	return data->vol;
}
void nocta_gainer_set_vol(nocta_unit* self, uint8_t vol) {
	gainer_data* data = self->data;
	data->vol = vol;
}

int8_t nocta_gainer_pan(nocta_unit* self) {
	gainer_data* data = self->data;
	return data->pan;
}
void nocta_gainer_set_pan(nocta_unit* self, int8_t pan) {
	gainer_data* data = self->data;
	data->pan = pan;
}
