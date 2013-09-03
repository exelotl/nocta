#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "SDL.h"
#include "nocta.h"

uint8_t* wav_data;
uint32_t wav_len, wav_pos;

nocta_unit* filter;
nocta_unit* gainer;
nocta_unit* delay;

int notes[] = {
	100, 101, 102, 103, 104, 105, 106, 107
};
	
void mix(void* userdata, uint8_t* stream, int len) {
	memset(stream, 0, len);
	
	int remaining = wav_len - wav_pos;
	if (remaining == 0) wav_pos = 0;
	if (remaining < len) len = remaining;
	
	SDL_MixAudio(stream, &(wav_data[wav_pos]), len, SDL_MIX_MAXVOLUME/2);
	nocta_process(filter, (int16_t*) stream, len/2);
	nocta_process(delay, (int16_t*) stream, len/2);
	nocta_process(gainer, (int16_t*) stream, len/2);
	wav_pos += len;
}

// GUI parameters:

typedef struct {
	char* name;
	nocta_unit* unit;
	int param_id;
} gui_param;

gui_param gui_params[8];
SDL_Window* window;
SDL_Renderer* renderer;

void init_gui();
void close_gui();
bool update_gui();

int main(int argc, char* argv[]) {
	
	SDL_Init(SDL_INIT_EVERYTHING);
		
	SDL_AudioSpec spec = {
		.freq = 44100,
		.format = AUDIO_S16,
		.channels = 2,
		.samples = 512,
		.callback = mix
	};
	
	SDL_OpenAudio(&spec, NULL);
	
	SDL_AudioSpec wav;
	SDL_LoadWAV("paper_isaac_sacrificial_1337.wav", &wav, &wav_data, &wav_len);
	SDL_PauseAudio(false);
	
	nocta_context context = {
		.sample_rate = spec.freq
	};
	
	gainer = nocta_gainer(&context);
	filter = nocta_svfilter(&context);
	delay = nocta_delay(&context);
	nocta_set(filter, NOCTA_FILTER_FREQ, 1000);
	nocta_set(filter, NOCTA_FILTER_RES, 100);
	
	
	init_gui();
	
	while (update_gui()) {
		SDL_Delay(20);
	}
	
	close_gui();
	SDL_CloseAudio();
	SDL_Quit();
}

void init_gui() {
	window = SDL_CreateWindow(
		"libnocta demo",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		300, 600, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

	gui_params[0] = (gui_param) { "volume",         gainer, NOCTA_GAINER_VOL };
	gui_params[1] = (gui_param) { "panning",        gainer, NOCTA_GAINER_PAN };
	gui_params[2] = (gui_param) { "filter_mode",    filter, NOCTA_FILTER_MODE };
	gui_params[3] = (gui_param) { "filter_freq",    filter, NOCTA_FILTER_FREQ };
	gui_params[4] = (gui_param) { "filter_res",     filter, NOCTA_FILTER_RES };
	gui_params[5] = (gui_param) { "delay_wet",      delay,  NOCTA_DELAY_WET };
	gui_params[6] = (gui_param) { "delay_feedback", delay,  NOCTA_DELAY_FEEDBACK };
	gui_params[7] = (gui_param) { "delay_time",     delay,  NOCTA_DELAY_TIME };
}

void close_gui() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

bool update_gui() {
	SDL_Event e;
	static bool mouse_down;
	static int mouse_x, mouse_y;
	
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
			case SDL_MOUSEMOTION:
				mouse_x = e.motion.x;
				mouse_y = e.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouse_down = true;
				break;
			case SDL_MOUSEBUTTONUP:
				mouse_down = false;
				break;
			case SDL_QUIT:
				return false;
		}
	}
	
	SDL_SetRenderDrawColor(renderer, 0,0,0,255);
	SDL_RenderClear(renderer);
	
	for (int i=0; i<8; i++) {
		int param_id = gui_params[i].param_id;
		nocta_unit* unit = gui_params[i].unit;
		nocta_param* param = nocta_get_param(unit, param_id);
		
		if (mouse_down && mouse_y > i*44 && mouse_y < i*44+44) {
			double val = mouse_x / 300.0;
			val *= param->max - param->min;
			val += param->min;
			nocta_set(unit, param_id, val);
		}
		
		double width = nocta_get(unit, param_id);
		width -= param->min;
		width /= param->max - param->min;
		
		SDL_SetRenderDrawColor(renderer, 30,30,30,255);
		SDL_RenderFillRect(renderer, &(SDL_Rect){2, i*44+2, 296, 40});
		SDL_SetRenderDrawColor(renderer, 80,80,80,255);
		SDL_RenderFillRect(renderer, &(SDL_Rect){4, i*44+4, width*292.0, 36});
	}
	SDL_RenderPresent(renderer);
	
	return true;
}