#include "SDL.h"

int SDL_main(int argc, char **argv) {
	SDL_Window *window;
	SDL_Surface *surface;
	SDL_Renderer *renderer;
	SDL_Event event;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		return 1;
	}

	window = SDL_CreateWindow("Android Test", 0, 0, 800, 600, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 255, 0));
	SDL_UpdateWindowSurface(window);

	while (1) {
		SDL_WaitEvent(&event);

		if (event.type == SDL_QUIT) {
			break;
		}
	}

	SDL_Quit();
	return 0;
}
