#pragma once


#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>


#include "Shapes.h"


//					Fill Circle
SDL_Texture* fillCircle( SDL_Renderer* renderer, Circle& c, SDL_Color cl, SDL_Rect& rect)
{
	SDL_Surface* sfc = SDL_CreateRGBSurface(0, c.r * 2, c.r * 2, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_LockSurface(sfc);

	uint32_t size = sfc->h * sfc->pitch;
	uint8_t* pixel = new uint8_t[size];

	for (int row = 0; row < c.r * 2; row++)
	{
		for (int col = 0; col < c.r * 2; col++)
		{
			int index = col * sfc->pitch + row * 4;

			if ((row - c.r) * (row - c.r) + (col - c.r) * (col - c.r) <= c.r * c.r)
			{
				pixel[index + 0] = cl.r;
				pixel[index + 1] = cl.g;
				pixel[index + 2] = cl.b;
				pixel[index + 3] = cl.a;
			}
			else
			{
				pixel[index + 0] = 0;
				pixel[index + 1] = 0;
				pixel[index + 2] = 0;
				pixel[index + 3] = 0;
			}
		}
	}

	memcpy(sfc->pixels, pixel, size);
	//SDL_memset(sfc->pixels, SDL_MapRGBA(sfc->format, cl.r, cl.g, cl.b, cl.a), sfc->h * sfc->pitch);
	SDL_UnlockSurface(sfc);

	SDL_Texture* txr = SDL_CreateTextureFromSurface(renderer, sfc);
	SDL_FreeSurface(sfc);
	delete pixel;

	rect.x = c.pos.x - c.r;
	rect.y = c.pos.y - c.r;
	SDL_QueryTexture(txr, NULL, NULL, &rect.w, &rect.h);
	return txr;
}