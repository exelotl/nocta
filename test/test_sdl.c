#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "SDL.h"
#include "nocta.h"

uint8_t* wav_data;
uint32_t wav_len, wav_pos;

nocta_engine* engine;
nocta_unit* filter;
nocta_unit* gainer;
	
void mix(void* userdata, uint8_t* stream, int len) {
	memset(stream, 0, len);
	
	int remaining = wav_len - wav_pos;
	if (remaining == 0) return;
	if (remaining < len) len = remaining;
	
	SDL_MixAudio(stream, &(wav_data[wav_pos]), len, SDL_MIX_MAXVOLUME/2);
	nocta_unit_process(gainer, (int16_t*) stream, len/2);
	wav_pos += len;
}

// GUI parameters:

typedef struct {
	char* name;
	int (*get)();
	void (*set)(int val);
	int min, max;
} param;

param gui_params[5];
SDL_Window* window;
SDL_Renderer* renderer;

int get_gainer_vol() { return nocta_gainer_vol(gainer); }
void set_gainer_vol(int val) { nocta_gainer_set_vol(gainer, val); }

int get_gainer_pan() { return nocta_gainer_pan(gainer); }
void set_gainer_pan(int val) { nocta_gainer_set_pan(gainer, val); }

int get_filter_mode() { return nocta_filter_mode(filter); }
void set_filter_mode(int val) { nocta_filter_set_mode(filter, val); }

int get_filter_freq() { return nocta_filter_freq(filter); }
void set_filter_freq(int val) { nocta_filter_set_freq(filter, val); }

int get_filter_res() { return nocta_filter_res(filter); }
void set_filter_res(int val) { nocta_filter_set_res(filter, val); }

void init_gui();
void close_gui();
bool update_gui();


int main() {
	
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
	
	engine = nocta_engine_new(spec.freq);
	gainer = nocta_unit_new(engine);
	nocta_gainer_init(gainer); 
	filter = nocta_unit_new(engine);
	nocta_filter_init(filter);
	nocta_unit_add(gainer, filter);
	nocta_filter_set_freq(filter, 1000);
	nocta_filter_set_res(filter, 100);
	
	init_gui();
	
	bool running = true;
	
	while (wav_pos < wav_len && running) {
		SDL_Delay(50);
		running = update_gui();
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
		300, 400, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

	gui_params[0] = (param) { "vol", get_gainer_vol, set_gainer_vol, 0, 127 };
	gui_params[1] = (param) { "pan", get_gainer_pan, set_gainer_pan, -128, 127 };
	gui_params[2] = (param) { "mode", get_filter_mode, set_filter_mode, 0, NOCTA_FILTER_NUM_MODES };
	gui_params[3] = (param) { "freq", get_filter_freq, set_filter_freq, 100, 22050 };
	gui_params[4] = (param) { "res", get_filter_res, set_filter_res, 0, 255 };	
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
	
	for (int i=0; i<5; i++) {
		if (mouse_down && mouse_y > i*44 && mouse_y < i*44+44) {
			double val = mouse_x / 300.0;
			val *= gui_params[i].max - gui_params[i].min;
			val += gui_params[i].min;
			gui_params[i].set(val);
		}
		
		double width = gui_params[i].get();
		width -= gui_params[i].min;
		width /= gui_params[i].max - gui_params[i].min;
		
		SDL_SetRenderDrawColor(renderer, 30,30,30,255);
		SDL_RenderFillRect(renderer, &(SDL_Rect){2, i*44+2, 296, 40});
		SDL_SetRenderDrawColor(renderer, 80,80,80,255);
		SDL_RenderFillRect(renderer, &(SDL_Rect){4, i*44+4, width*292.0, 36});
	}
	SDL_RenderPresent(renderer);
	
	return true;
}