#include <SDL3/SDL.h>

typedef struct {
    float x;
    float y;
    float radius;
    int num_sides;
    SDL_FColor color;
    SDL_Vertex* vertices;
    int* indices;
} SDL_GeoCircle;

// single circle

SDL_GeoCircle SDL_CreateGeoCircle(float x, float y, float radius, int num_sides, SDL_FColor color) {

    SDL_GeoCircle circle;

    circle.x = x;
    circle.y = y;
    circle.radius = radius;
    circle.num_sides = num_sides;
    circle.color = color;

    circle.vertices = SDL_calloc(num_sides + 2, sizeof(SDL_Vertex));
    circle.indices = SDL_calloc(num_sides * 3, sizeof(int));

    // centre vert
    circle.vertices[0].position.x = x;
    circle.vertices[0].position.y = y;
    circle.vertices[0].color = color;

    // calculate the angle of the triangle wedge in radians
    float angle_radians = (SDL_PI_F * 2) * (1.0f / num_sides);

    // place the outer verts
    for (int i = 0; i <= num_sides; i++) {
        circle.vertices[i + 1].position.x = x + (SDL_sinf(angle_radians * i) * radius);
        circle.vertices[i + 1].position.y = y + (SDL_cosf(angle_radians * i) * radius);
        circle.vertices[i + 1].color = color;
    }

    // create triangle indices
    for (int i = 0; i < num_sides; i++) {
        int triangle = i * 3;

        circle.indices[triangle] = 0;
        circle.indices[triangle + 1] = i + 1;
        circle.indices[triangle + 2] = i + 2;
    }

    return circle;
}

void SDL_RenderGeoCircle(SDL_Renderer* renderer, SDL_GeoCircle* circle) {

    SDL_RenderGeometry(
        renderer, 
        NULL, 
        circle->vertices, 
        circle->num_sides + 2, 
        circle->indices, 
        circle->num_sides * 3
    );

}

void SDL_DestroyGeoCircle(SDL_GeoCircle* circle) {
    SDL_free(circle->vertices);
    SDL_free(circle->indices);
}