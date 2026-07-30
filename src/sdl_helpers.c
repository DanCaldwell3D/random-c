#include <SDL3/SDL.h>
#include "sdl_helpers.h"


SDL_GeoCircles* SDL_CreateGeoCircles(int max_circles, int sides_per_circle) {

    SDL_GeoCircles* circles = SDL_calloc(1, sizeof(SDL_GeoCircles));

    circles->max_circles = max_circles;
    circles->sides_per_circle = sides_per_circle;
    circles->vertices = SDL_calloc(max_circles * (sides_per_circle + 2), sizeof(SDL_Vertex));
    circles->indices = SDL_calloc(max_circles * sides_per_circle * 3, sizeof(int));

    return circles;
}

void SDL_DestroyGeoCircles(SDL_GeoCircles* circles) {

    if (!circles) return;

    SDL_free(circles->vertices);
    SDL_free(circles->indices);
    SDL_free(circles);
}

void SDL_GeoCirclesUpdate(SDL_GeoCircles* circles, int circle_index, float x, float y, float radius, SDL_FColor color) {
    if (circle_index < 0 || circle_index >= circles->max_circles) return;
    
    int sides = circles->sides_per_circle;
    int vert_start = circle_index * (sides + 2);
    int indices_start = circle_index * sides * 3;

    circles->vertices[vert_start].position.x = x;
    circles->vertices[vert_start].position.y = y;
    circles->vertices[vert_start].color = color;

    float angle_radians = (SDL_PI_F * 2.0f) / (float)sides;

    for (int i = 0; i <= sides; i++) {
        circles->vertices[vert_start + i + 1].position.x = x + (SDL_sinf(angle_radians * i) * radius);
        circles->vertices[vert_start + i + 1].position.y = y + (SDL_cosf(angle_radians * i) * radius);
        circles->vertices[vert_start + i + 1].color = color;
    }

    for (int i = 0; i < sides; i++) {
        int triangle = indices_start + (i * 3);

        circles->indices[triangle] = vert_start;
        circles->indices[triangle + 1] = vert_start + i + 1;
        circles->indices[triangle + 2] = vert_start + i + 2;
    }
}

void SDL_RenderGeoCircles(SDL_GeoCircles* circles, SDL_Renderer* renderer, int count) {
    int sides = circles->sides_per_circle;

    SDL_RenderGeometry(
        renderer, 
        NULL, 
        circles->vertices, 
        count * (sides + 2), 
        circles->indices, 
        count * sides * 3
    );
}
