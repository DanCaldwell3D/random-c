#ifndef SDL_HELPERS_H
#define SDL_HELPERS_H

#include <SDL3/SDL.h>

typedef struct {
    SDL_Vertex* vertices;
    int* indices;
    int max_circles;
    int sides_per_circle;
} SDL_GeoCircles;

SDL_GeoCircles* SDL_CreateGeoCircles(int max_circles, int sides_per_circle);
void SDL_DestroyGeoCircles(SDL_GeoCircles* circles);
void SDL_GeoCirclesUpdate(SDL_GeoCircles* circles, int circle_index, float x, float y, float radius, SDL_FColor color);
void SDL_RenderGeoCircles(SDL_GeoCircles* circles, SDL_Renderer* renderer, int count);

#endif
