#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdbool.h>
#include <stdio.h>

#include "src/vec2.h"
#include "src/collision_grid_2d.h"
#include "src/sdl_helpers.h"

#define NUM_BALLS 500
#define SIM_SUBSTEPS 24
#define RESTITUTION 0.99

#define MIN_BALL_RADIUS 5.0
#define MAX_BALL_RADIUS 30.0
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 1000

typedef struct {
    Vec2 position;
    Vec2 velocity;
    double radius;
    double mass;
    bool rest;
    SDL_FColor color;
} Ball;



typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;

    int window_width;
    int window_height;

    double mouse_x;
    double mouse_y;

    Uint64 last_time;

    double gravity;
    double scene_scale;

    Grid2D* collision_grid;

    Ball ball_array[NUM_BALLS];
    SDL_GeoCircles* circles;

} AppState;

float lerp(float start, float end, float position) {
    if (start > end) {
        return start - ((start - end) * position);
    } else if (start < end) {
        return start + ((end - start) * position);
    } else {
        return start;
    }
}

SDL_FColor fcolor_lerp(SDL_FColor start_color, SDL_FColor end_color, float position) {
    SDL_FColor lerp_colour;

    lerp_colour.r = lerp(start_color.r, end_color.r, position);
    lerp_colour.g = lerp(start_color.g, end_color.g, position);
    lerp_colour.b = lerp(start_color.b, end_color.b, position);
    lerp_colour.a = lerp(start_color.a, end_color.a, position);

    return lerp_colour;
}

SDL_FColor hsv_to_rgb(float hue, float saturation, float value) {
    float r;
    float g;
    float b;

    float hue_range = hue * 6.0f;

    switch((int)hue_range) {
        case 0:
            r = 1.0f;
            g = hue_range;
            b = 0.0f;
            break;
        case 1:
            r = 1.0f - (hue_range - 1.0f);
            g = 1.0f;
            b = 0.0f;
            break;
        case 2:
            r = 0.0f;
            g = 1.0f;
            b = hue_range - 2.0f;
            break;
        case 3:
            r = 0.0f;
            g = 1.0f - (hue_range - 3.0f);
            b = 1.0f;
            break;
        case 4:
            r = hue_range - 4.0f;
            g = 0.0f;
            b = 1.0f;
            break;
        case 5:
            r = 1.0f;
            g = 0.0f;
            b = 1.0f - (hue_range - 5.0f);
            break;
        default:
            r = 1.0f;
            g = 0.0f;
            b = 0.0f;
    }

    const SDL_FColor rgba = {r, g, b, 1.0f};

    return rgba;
}

// SDL_FColor circle_rainbow_color(AppState* as) {

//     as->colour_current_time = SDL_GetTicks();
//     SDL_FColor circle_color;

//     if (as->colour_current_time - as->colour_start_time < as->transition_ticks) {
//         float lerp_position = (float)(as->colour_current_time - as->colour_start_time) / (float)as->transition_ticks;
//         circle_color = fcolor_lerp(as->last_circle_color, as->next_circle_color, lerp_position);
//     } else {
//         as->colour_start_time = as->colour_current_time;
//         as->last_circle_color = as->next_circle_color;
//         as->next_circle_color = (SDL_FColor){SDL_randf(), SDL_randf(), SDL_randf(), SDL_ALPHA_OPAQUE_FLOAT};
//         circle_color = as->last_circle_color;
//     }

//     return circle_color;
// }

void handle_screen_edge_collision(AppState* as, int ball_index) {
    if (as->ball_array[ball_index].position.x < as->ball_array[ball_index].radius) {
        as->ball_array[ball_index].position.x = as->ball_array[ball_index].radius;
        as->ball_array[ball_index].velocity.x = -as->ball_array[ball_index].velocity.x;
    } else if (as->ball_array[ball_index].position.x > as->window_width - as->ball_array[ball_index].radius) {
        as->ball_array[ball_index].position.x = as->window_width - as->ball_array[ball_index].radius;
        as->ball_array[ball_index].velocity.x = -as->ball_array[ball_index].velocity.x;
    }

    if (as->ball_array[ball_index].position.y < as->ball_array[ball_index].radius) {
        as->ball_array[ball_index].position.y = as->ball_array[ball_index].radius;
        as->ball_array[ball_index].velocity.y = -as->ball_array[ball_index].velocity.y;
    } else if (as->ball_array[ball_index].position.y > as->window_height - as->ball_array[ball_index].radius) {
        as->ball_array[ball_index].position.y = as->window_height - as->ball_array[ball_index].radius;
        as->ball_array[ball_index].velocity.y = -as->ball_array[ball_index].velocity.y;
    }
}

void handle_ball_collision(AppState* as, int ball_index_a, int ball_index_b) {

    Vec2 distance_vec = vec2_subtract(as->ball_array[ball_index_b].position, as->ball_array[ball_index_a].position);
    double ball_distance = vec2_length(distance_vec);
    double min_distance = as->ball_array[ball_index_a].radius + as->ball_array[ball_index_b].radius;

    if (ball_distance < 1e-6) {
        return;
    }

    if (ball_distance < min_distance) {
        double normalise_factor = 1.0 / ball_distance;
        Vec2 collision_normal = vec2_scale(distance_vec, normalise_factor);

        double move_distance = min_distance - ball_distance;

        Vec2 move_vec_a = vec2_scale(collision_normal, move_distance / 2.0 * -1.0);
        Vec2 new_position_a = vec2_add(as->ball_array[ball_index_a].position, move_vec_a);
        as->ball_array[ball_index_a].position = new_position_a;

        Vec2 move_vec_b = vec2_scale(collision_normal, move_distance / 2.0);
        Vec2 new_position_b = vec2_add(as->ball_array[ball_index_b].position, move_vec_b);
        as->ball_array[ball_index_b].position = new_position_b;

        double mass_a = as->ball_array[ball_index_a].mass;
        double mass_b = as->ball_array[ball_index_b].mass;
        double total_mass = mass_a + mass_b;

        double projected_velocity_a = vec2_dot(as->ball_array[ball_index_a].velocity, collision_normal);
        double projected_velocity_b = vec2_dot(as->ball_array[ball_index_b].velocity, collision_normal);

        double momentum_a = mass_a * projected_velocity_a;
        double momentum_b = mass_b * projected_velocity_b;
        double restitution = RESTITUTION;

        double mass_projected_velocity_a =
            (momentum_a + momentum_b -
            mass_b * (projected_velocity_a - projected_velocity_b) * restitution) /
            total_mass;

        double mass_projected_velocity_b =
            (momentum_a + momentum_b -
            mass_a * (projected_velocity_b - projected_velocity_a) * restitution) /
            total_mass;

        Vec2 collision_velocity_a = vec2_scale(collision_normal, (mass_projected_velocity_a - projected_velocity_a));
        Vec2 collision_velocity_b = vec2_scale(collision_normal, (mass_projected_velocity_b - projected_velocity_b));

        vec2_add_inplace(&as->ball_array[ball_index_a].velocity, collision_velocity_a);
        vec2_add_inplace(&as->ball_array[ball_index_b].velocity, collision_velocity_b);
    }
}

void update_balls(AppState* as, int substeps) {

    double frame_time = 1.0 / 60.0;
    double delta_t = frame_time / substeps;

    Grid2D* grid = as->collision_grid;
    GridCell2D* grid_cells = grid->cells;

    for (int i = 0; i < substeps; i++) {

        reset_cell_counts(grid);

        for (int ball_index = 0; ball_index < NUM_BALLS; ball_index++) {
            Ball* current_ball = &as->ball_array[ball_index];
            
            // apply gravity
            current_ball->velocity.y += (as->gravity * as->scene_scale * delta_t);
            
            // get velocity divided by timestep
            Vec2 timestep_velocity = (Vec2){
                current_ball->velocity.x * as->scene_scale * delta_t,
                current_ball->velocity.y * as->scene_scale * delta_t
            };
            
            // update ball position based on velocity
            vec2_add_inplace(&current_ball->position, timestep_velocity);
            
            // collision grid
            int cell_index = xy_to_cell(
                grid,
                current_ball->position.x,
                current_ball->position.y
            );
            increment_cell_count(grid, cell_index);
        }

        // add balls to grid
        compute_cell_offsets(grid);
        
        for (int ball_index = 0; ball_index < NUM_BALLS; ball_index++) {
            Ball* current_ball = &as->ball_array[ball_index];

            int cell_index = xy_to_cell(
                grid,
                current_ball->position.x,
                current_ball->position.y
            );
            
            insert_item(
                grid, 
                ball_index, 
                cell_index
            );
        }

        // resolve collision constraints
        for (int cell_index = 0; cell_index < grid->num_cells; cell_index++) {
            // skip empty cells
            if (grid_cells[cell_index].count == 0) {
                continue;
            }
            // handle cells with one item (look up neighbours only)
            if (grid_cells[cell_index].count == 1) {
                int ball_index_a = grid->flat_items[grid_cells[cell_index].start];
                int* neighbours = grid_cells[cell_index].neighbours;
                
                for (int i = 0; i < 8; i++) {
                    // skip neighbours off grid
                    if (neighbours[i] == -1) {
                        continue;
                    }
                    // skip empty neighbours
                    if (grid_cells[neighbours[i]].count == 0) {
                        continue;
                    }
                    // handle collision constraint with occupied neighbour
                    GridCell2D* neighbour_cell = &grid_cells[neighbours[i]];
                    for (int j = 0; j < neighbour_cell->count; j++) {
                        int ball_index_b = grid->flat_items[neighbour_cell->start + j];
                        if (ball_index_a < ball_index_b) {
                            handle_ball_collision(as, ball_index_a, ball_index_b);
                        }
                    }
                }
            }
            // handle cells with more than one item
            if (grid_cells[cell_index].count > 1) {
                int* neighbours = grid_cells[cell_index].neighbours;

                for (int i = 0; i < grid_cells[cell_index].count; i++) {
                    int ball_index_a = grid->flat_items[grid_cells[cell_index].start + i];
                    for (int j = 0; j < grid_cells[cell_index].count; j++) {
                        if (j <= i) {continue;}
                        int ball_index_b = grid->flat_items[grid_cells[cell_index].start +j];
                        if (ball_index_a < ball_index_b) {
                            handle_ball_collision(as, ball_index_a, ball_index_b);
                        }
                    }

                    for (int j = 0; j < 8; j++) {
                        // skip neighbours off grid
                        if (neighbours[j] == -1) {
                            continue;
                        }
                        // skip empty neighbours
                        if (grid_cells[neighbours[j]].count == 0) {
                            continue;
                        }
                        // handle collision constraint with occupied neighbour
                        GridCell2D* neighbour_cell = &grid_cells[neighbours[j]];
                        for (int j = 0; j < neighbour_cell->count; j++) {
                            int ball_index_b = grid->flat_items[neighbour_cell->start + j];
                            if (ball_index_a < ball_index_b) {
                                handle_ball_collision(as, ball_index_a, ball_index_b);
                            }
                        }
                    }
                }

            }

        }

        for (int ball_index = 0; ball_index < NUM_BALLS; ball_index++) {
            handle_screen_edge_collision(as, ball_index);
        }
    }
}

// SDL callbacks
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    AppState* as = (AppState*)SDL_calloc(1, sizeof(AppState));
    *appstate = as;

    as->window_height = WINDOW_HEIGHT;
    as->window_width = WINDOW_WIDTH;

    as->last_time = SDL_GetTicksNS();

    as->gravity = 9.8;
    as->scene_scale = 20.0;

    as->circles = SDL_CreateGeoCircles(
        NUM_BALLS, 
        48
    );
    
    as->collision_grid = new_grid(
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        MAX_BALL_RADIUS * 2, 
        NUM_BALLS
    );

    for (int i = 0; i < NUM_BALLS; i++) {
        as->ball_array[i].position = (Vec2){(double)SDL_randf() * as->window_width, (double)SDL_randf() * as->window_height};
        as->ball_array[i].velocity = (Vec2){(SDL_randf() - 0.5f) * 2.0 * as->scene_scale, (SDL_randf() - 0.5f) * as->scene_scale};
        as->ball_array[i].radius = ((double)SDL_randf() * (MAX_BALL_RADIUS - MIN_BALL_RADIUS)) + MIN_BALL_RADIUS;
        as->ball_array[i].mass = (SDL_PI_D * as->ball_array[i].radius * as->ball_array[i].radius) / as->scene_scale;
        as->ball_array[i].color = hsv_to_rgb(SDL_randf(), 1.0f, 1.0f);
        as->ball_array[i].rest = false;

        // move balls inside window bounds if necessary
        if (as->ball_array[i].position.x < as->ball_array[i].radius) {
            as->ball_array[i].position.x = as->ball_array[i].radius;
        } else if (as->ball_array[i].position.x > as->window_width - as->ball_array[i].radius) {
            as->ball_array[i].position.x = as->window_width - as->ball_array[i].radius;
        }
        if (as->ball_array[i].position.y < as->ball_array[i].radius) {
            as->ball_array[i].position.y = as->ball_array[i].radius;
        } else if (as->ball_array[i].position.y > as->window_height - as->ball_array[i].radius) {
            as->ball_array[i].position.y = as->window_height - as->ball_array[i].radius;
        }

        int cell_index = xy_to_cell(
            as->collision_grid, 
            as->ball_array[i].position.x, 
            as->ball_array[i].position.y
        );

        increment_cell_count(as->collision_grid, cell_index);
        
        SDL_GeoCirclesUpdate(
            as->circles, 
            i, 
            as->ball_array[i].position.x, 
            as->ball_array[i].position.y, 
            as->ball_array[i].radius, 
            as->ball_array[i].color
        );
    }

    compute_cell_offsets(as->collision_grid);

    for (int i = 0; i < NUM_BALLS; i++) {
        int cell_index = xy_to_cell(
            as->collision_grid, 
            as->ball_array[i].position.x, 
            as->ball_array[i].position.y
        );

        insert_item(as->collision_grid, i, cell_index);
    }



    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Test Window",
        as->window_width,
        as->window_height,
        SDL_WINDOW_RESIZABLE,
        &as->window,
        &as->renderer)) {
            return SDL_APP_FAILURE;
        }

    SDL_SetRenderVSync(as->renderer, 1);

    SDL_SetRenderLogicalPresentation(as->renderer,
        as->window_width,
        as->window_height,
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

        return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    AppState* as = appstate;

    switch (event->type) {
        case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
        case SDL_EVENT_MOUSE_MOTION:
        as->mouse_x = event->motion.x;
        as->mouse_y = event->motion.y;
        return SDL_APP_CONTINUE;
        default:
        return SDL_APP_CONTINUE;
    }
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState* as = appstate;

    SDL_SetRenderDrawColorFloat(as->renderer, 0.0f, 0.0f, 0.0f, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(as->renderer);

    SDL_SetRenderDrawColorFloat(as->renderer, 1.0f, 1.0f, 0.0f, SDL_ALPHA_OPAQUE_FLOAT);

    update_balls(as, SIM_SUBSTEPS);

    for (int i = 0; i < NUM_BALLS; i++) {
        SDL_GeoCirclesUpdate(
            as->circles, 
            i,
            as->ball_array[i].position.x,
            as->ball_array[i].position.y,
            as->ball_array[i].radius,
            as->ball_array[i].color
        );
    }
    
    SDL_RenderGeoCircles(
        as->circles,
        as->renderer, 
        NUM_BALLS
    );
    
    SDL_RenderPresent(as->renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (appstate != NULL) {
        AppState* as = appstate;
        SDL_DestroyRenderer(as->renderer);
        SDL_DestroyWindow(as->window);
        free_grid(as->collision_grid);
        SDL_DestroyGeoCircles(as->circles);
        SDL_free(as);
    }
}
