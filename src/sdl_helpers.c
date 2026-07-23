#include <SDL3/SDL.h>

bool SDL_RenderGeoCircle(SDL_Renderer* renderer, float x, float y, float radius, int sides, SDL_FColor color) {
    SDL_Vertex* vertices = SDL_calloc(sides + 2, sizeof(SDL_Vertex));
    int* indices = SDL_calloc(sides * 3, sizeof(int));

    vertices[0].position.x = x;
    vertices[0].position.y = y;
    vertices[0].color = color;

    float angle_radians = (SDL_PI_F * 2) * (1.0f / sides);

    for (int i = 0; i <= sides; i++) {
        vertices[i + 1].position.x = x + (SDL_sinf(angle_radians * i) * radius);
        vertices[i + 1].position.y = y + (SDL_cosf(angle_radians * i) * radius);
        vertices[i + 1].color = color;
    }

    for (int i = 0; i < sides; i++) {
        int triangle = i * 3;

        indices[triangle] = 0;
        indices[triangle + 1] = i + 1;
        indices[triangle + 2] = i + 2;
    }

    SDL_RenderGeometry(renderer, NULL, vertices, sides + 2, indices, sides * 3);
    SDL_free(vertices);
    SDL_free(indices);

    return true;
}