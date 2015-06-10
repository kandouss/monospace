#include "SDL/SDL.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "msp_config.h"
#include "msp_buffer.h"
#include "msp_output_sdl.h"
 
static SDL_Surface *buffer_surface;

void msp_output_sdl_render(void)
{
	SDL_Surface * screen = SDL_GetVideoSurface();
	if(SDL_BlitSurface(buffer_surface, NULL, screen, NULL) == 0)
		SDL_UpdateRect(screen, 0, 0, 0, 0);
	SDL_Delay(1);
}

static int event_filter(const SDL_Event * event)
{ 
	return event->type == SDL_QUIT; 
}
int msp_output_sdl_show_yy(msp_buffer *b, const char *name)
{
	atexit(SDL_Quit);
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return -1;
	
	SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT
			, 24
			, SDL_SWSURFACE);

	SDL_WM_SetCaption(name, name);

	buffer_surface = SDL_CreateRGBSurfaceFrom( b->start, 
			SCREEN_WIDTH, SCREEN_HEIGHT, 16, SCREEN_WIDTH*2,
			0xF0F0, 0xF0F0, 0xF0F0, 0);

	SDL_SetEventFilter(event_filter);
	return 0;
}
int msp_output_sdl_show_gs8(msp_buffer *b, const char *name)
{
	atexit(SDL_Quit);
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return -1;
	
	SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT
			, 24
			, SDL_SWSURFACE);

	SDL_WM_SetCaption(name, name);

	buffer_surface = SDL_CreateRGBSurfaceFrom( b->start, 
			SCREEN_WIDTH, SCREEN_HEIGHT, 8, SCREEN_WIDTH,
			0xFF, 0xFF, 0xFF, 0);

	SDL_SetEventFilter(event_filter);
	
	return 0;
}


int msp_output_sdl_show_rgb(msp_buffer *b, const char *name)
{
	atexit(SDL_Quit);
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return -1;
	
	SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT
			, 24
			, SDL_SWSURFACE);

	SDL_WM_SetCaption(name, name);

	buffer_surface = SDL_CreateRGBSurfaceFrom( b->start, 
			SCREEN_WIDTH, SCREEN_HEIGHT, 24, SCREEN_WIDTH*3,
			0xFF0000, 0x00FF00, 0x0000FF, 0);

	SDL_SetEventFilter(event_filter);
	
	return 0;
}


