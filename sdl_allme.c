#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define NUM_BALLS 200

typedef struct {
    double x;
    double y;
} Vec2;

void vec2_add_inplace(Vec2* vec_a, Vec2* vec_b) {
    vec_a->x = vec_a->x + vec_b->x;
    vec_a->y = vec_a->y + vec_b->y;
}

Vec2 vec2_add(Vec2* vec_a, Vec2* vec_b) {
    Vec2 result;
    result.x = vec_a->x + vec_b->x;
    result.y = vec_a->y + vec_b->y;

    return result;
}

void vec2_subtract_inplace(Vec2* vec_a, Vec2* vec_b) {
    vec_a->x = vec_a->x - vec_b->x;
    vec_a->y = vec_a->y - vec_b->y;
}

Vec2 vec2_subtract(Vec2* vec_a, Vec2* vec_b) {
    Vec2 result;
    result.x = vec_a->x - vec_b->x;
    result.y = vec_a->y - vec_b->y;

    return result;
}

void vec2_scalar_multiply_inplace(Vec2* vec, double scalar) {
    vec->x = vec->x * scalar;
    vec->y = vec->y * scalar;
}

Vec2 vec2_scalar_multiply(Vec2* vec, double scalar) {
    Vec2 result;
    result.x = vec->x * scalar;
    result.y = vec->y * scalar;

    return result;
}

float vec2_length(Vec2* vec) {
    float vec_length;
    vec_length = SDL_sqrtf((vec->x * vec->x) + (vec->y * vec->y));

    return vec_length;
}

float vec2_dotproduct(Vec2* vec_a, Vec2* vec_b) {
    float dot_product;
    dot_product = (vec_a->x * vec_b->x) + (vec_a->y * vec_b->y);

    return dot_product;
}

typedef struct {
    Vec2 position;
    Vec2 velocity;
    float radius;
    SDL_FColor color;
} Ball;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    int window_width;
    int window_height;

    float mouse_x;
    float mouse_y;

    Uint64 last_time;

    SDL_FColor last_circle_color;
    SDL_FColor next_circle_color;
    int transition_ticks;
    Uint64 colour_start_time;
    Uint64 colour_current_time;

    float gravity;
    float scene_scale;  // pixels per metre

    Ball ball_array[NUM_BALLS];

} AppState;

bool SDL_RenderGeoCircle(SDL_Renderer* renderer, float x, float y, float radius, int sides, SDL_FColor color) {
    // draws triangles fanning out from the centre point to create a circle/n-sided polygon
    SDL_Vertex* vertices = SDL_calloc(sides + 2, sizeof(SDL_Vertex));
    int* indices = SDL_calloc(sides * 3, sizeof(int));

    // place vertices and set colour
    vertices[0].position.x = x;
    vertices[0].position.y = y;
    vertices[0].color = color;
    
    float angle_radians = (SDL_PI_F * 2) * (1.0 / sides);

    for (int i = 0; i <= sides; i++) {
        // refer to unit circle and sin/cosine for formula
        // sin 0 = 0, sin 90 = 1, cos 0 = 1, cos 90 = 0
        vertices[i + 1].position.x = x + (SDL_sinf(angle_radians * i) * radius);
        vertices[i + 1].position.y = y + (SDL_cosf(angle_radians * i) * radius);
        vertices[i + 1].color = color;
    }

    // create indices list for triangles
    for (int i = 0; i < sides; i++) {
        int triangle = i * 3;

        indices[triangle] = 0;
        indices[triangle + 1] = i + 1;
        indices[triangle + 2] = i + 2;
    }

    // render triangle list
    SDL_RenderGeometry(renderer, NULL, vertices, sides + 2, indices, sides * 3);
    SDL_free(vertices);
    SDL_free(indices);

    return true;
}

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

SDL_FColor circle_rainbow_color(AppState* as) {
    
    as->colour_current_time = SDL_GetTicks();
    SDL_FColor circle_color;
    
    if (as->colour_current_time - as->colour_start_time < as->transition_ticks) {
        float lerp_position = ((float)as->colour_current_time - (float)as->colour_start_time) / (float)as->transition_ticks;
        circle_color = fcolor_lerp(as->last_circle_color, as->next_circle_color, lerp_position);
    } else {
        as->colour_start_time = as->colour_current_time;
        as->last_circle_color = as->next_circle_color;
        as->next_circle_color = (SDL_FColor){SDL_randf(), SDL_randf(), SDL_randf(), SDL_ALPHA_OPAQUE_FLOAT};
        circle_color = as->last_circle_color;
    }

    return circle_color;
}

void handle_screen_edge_collision(AppState* as, int ball_index) {
    // x bounds
    if (as->ball_array[ball_index].position.x < as->ball_array[ball_index].radius) {
        as->ball_array[ball_index].position.x = as->ball_array[ball_index].radius;
        as->ball_array[ball_index].velocity.x = -as->ball_array[ball_index].velocity.x;
    } else if (as->ball_array[ball_index].position.x > as->window_width - as->ball_array[ball_index].radius) {
        as->ball_array[ball_index].position.x = as->window_width - as->ball_array[ball_index].radius;
        as->ball_array[ball_index].velocity.x = -as->ball_array[ball_index].velocity.x;
    }
    // y bounds
    if (as->ball_array[ball_index].position.y < as->ball_array[ball_index].radius) {
        as->ball_array[ball_index].position.y = as->ball_array[ball_index].radius;
        as->ball_array[ball_index].velocity.y = -as->ball_array[ball_index].velocity.y;
    } else if (as->ball_array[ball_index].position.y > as->window_height - as->ball_array[ball_index].radius) {
        as->ball_array[ball_index].position.y = as->window_height - as->ball_array[ball_index].radius;
        as->ball_array[ball_index].velocity.y = -as->ball_array[ball_index].velocity.y;
    }
}

void handle_ball_collision_no_mass(AppState* as, int ball_index_a) {
    int array_len = sizeof(as->ball_array) / sizeof(Ball);
    for (int ball_index_b = 0; ball_index_b < array_len; ball_index_b++) {
        if (ball_index_b == ball_index_a) {
            continue;
        } else {

            Vec2 distance_vec = vec2_subtract(&as->ball_array[ball_index_b].position, &as->ball_array[ball_index_a].position);
            float ball_distance = vec2_length(&distance_vec);
            float min_distance = as->ball_array[ball_index_a].radius + as->ball_array[ball_index_b].radius;

            if (ball_distance < min_distance) {
                // scale distance vector to become unit collision normal
                float normalise_factor = 1.0 / ball_distance;
                Vec2 collision_normal = vec2_scalar_multiply(&distance_vec, normalise_factor);

                // move balls apart
                float move_distance = min_distance - ball_distance;
            
                Vec2 move_vec_a = vec2_scalar_multiply(&collision_normal, move_distance / 2.0 * -1.0);
                Vec2 new_position_a = vec2_add(&as->ball_array[ball_index_a].position, &move_vec_a);
                as->ball_array[ball_index_a].position = new_position_a;
                
                Vec2 move_vec_b = vec2_scalar_multiply(&collision_normal, move_distance / 2.0);
                Vec2 new_position_b = vec2_add(&as->ball_array[ball_index_b].position, &move_vec_b);
                as->ball_array[ball_index_b].position = new_position_b;
                
                // calculate new velocities
                float dot_product_a = vec2_dotproduct(&as->ball_array[ball_index_a].velocity, &collision_normal);
                Vec2 transform_a = vec2_scalar_multiply(&collision_normal, -2.0 * dot_product_a);
                vec2_add_inplace(&as->ball_array[ball_index_a].velocity, &transform_a);

                float dot_product_b = vec2_dotproduct(&as->ball_array[ball_index_b].velocity, &collision_normal);
                Vec2 transform_b = vec2_scalar_multiply(&collision_normal, -2.0 * dot_product_b);
                vec2_add_inplace(&as->ball_array[ball_index_b].velocity, &transform_b);
            }
        }
    }
}

void update_ball_position(AppState* as, int substeps) {
    // how much time has passed?
    // double delta_t = (float)(SDL_GetTicksNS() - as->last_time) / 1000000000.0;
    double frame_time = 1.0 / 60.0;
    double delta_t = frame_time / substeps;
    // SDL_Log("delta t: %f", delta_t);
    for (int i = 0; i < substeps; i++) {
        for (int j = 0; j < sizeof(as->ball_array) / sizeof(Ball); j++) {
            // apply gravity
            as->ball_array[j].velocity.y += (as->gravity * as->scene_scale * delta_t);

            // update position
            Vec2 timestep_velocity = (Vec2){as->ball_array[j].velocity.x * delta_t * as->scene_scale,
                as->ball_array[j].velocity.y * as-> scene_scale * delta_t};
            vec2_add_inplace(&as->ball_array[j].position, &timestep_velocity);
            
            // handle collision with other balls
            handle_ball_collision_no_mass(as, j);

            // handle collision with screen bounds
            handle_screen_edge_collision(as, j);
            
        }
    }
    as->last_time = SDL_GetTicksNS();
}

// SDL callbacks
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    // create appstate, use calloc to zero out values
    AppState* as = (AppState*)SDL_calloc(1, sizeof(AppState));
    *appstate = as;
    SDL_Log("ball array length: %lu", sizeof(as->ball_array) / sizeof(Ball));
    as->window_height = 1000;
    as->window_width = 1000;
    
    as->last_time = SDL_GetTicksNS();

    // as->last_circle_color = (SDL_FColor){SDL_randf(), SDL_randf(), SDL_randf(), SDL_ALPHA_OPAQUE_FLOAT};
    // as->next_circle_color = (SDL_FColor){SDL_randf(), SDL_randf(), SDL_randf(), SDL_ALPHA_OPAQUE_FLOAT};
    // as->transition_ticks = 500;
    // as->colour_start_time = SDL_GetTicks();
    // as->colour_current_time = SDL_GetTicks();

    // initialise ball physics
    as->gravity = 9.8;
    as->scene_scale = 20.0;
    
    for (int i = 0; i < sizeof(as->ball_array) / sizeof(Ball); i++) {
        as->ball_array[i].position = (Vec2){SDL_randf() * as->window_width, SDL_randf() * as->window_height};
        as->ball_array[i].velocity = (Vec2){(SDL_randf() - 0.5) * 2.0 * as->scene_scale, (SDL_randf() - 0.5) * as->scene_scale};
        // as->ball_array[i].radius = (SDL_randf() * 15.0) + 5.0;
        as->ball_array[i].radius = 10.0;
        as->ball_array[i].color = (SDL_FColor){SDL_randf(), SDL_randf(), SDL_randf(), SDL_ALPHA_OPAQUE_FLOAT};

        // move ball within window bounds if necessary
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
        SDL_Log("ball %i pos: %f, %f", i, as->ball_array[i].position.x, as->ball_array[i].position.y);
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
    
    SDL_SetRenderDrawColorFloat(as->renderer, 0.0, 0.0, 0.0, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(as->renderer);
    
    SDL_SetRenderDrawColorFloat(as->renderer, 1.0, 1.0, 0.0, SDL_ALPHA_OPAQUE_FLOAT);
    
    // SDL_FColor circle_color = circle_rainbow_color(as);
    update_ball_position(as, 50);

    for (int i = 0; i < sizeof(as->ball_array) / sizeof(Ball); i++) {
        // SDL_Log("rendering ball %i", i);
        // SDL_Log("pos x: %f y: %f", as->ball_array[i].position.x, as->ball_array[i].position.y);
        SDL_RenderGeoCircle(as->renderer, as->ball_array[i].position.x, as->ball_array[i].position.y, as->ball_array[i].radius, 48, as->ball_array[i].color);
    }
    SDL_RenderPresent(as->renderer);
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (appstate != NULL) {
        AppState* as = appstate;
        SDL_DestroyRenderer(as->renderer);
        SDL_DestroyWindow(as->window);
        SDL_free(as);
    }
}