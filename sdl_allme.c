#include <SDL3/SDL_video.h>
#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define NUM_BALLS 10
#define SIM_SUBSTEPS 20

#define MIN_BALL_RADIUS 5.0
#define MAX_BALL_RADIUS 10.0
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define GRID_CELL_SIZE 16

// Vec2 struct and functions

typedef struct {
    float x;
    float y;
} Vec2;

void vec2_add_inplace(Vec2* vec_a, const Vec2* vec_b) {
    vec_a->x = vec_a->x + vec_b->x;
    vec_a->y = vec_a->y + vec_b->y;
}

Vec2 vec2_add(const Vec2* vec_a, const Vec2* vec_b) {
    Vec2 result;
    result.x = vec_a->x + vec_b->x;
    result.y = vec_a->y + vec_b->y;

    return result;
}

void vec2_subtract_inplace(Vec2* vec_a, const Vec2* vec_b) {
    vec_a->x = vec_a->x - vec_b->x;
    vec_a->y = vec_a->y - vec_b->y;
}

Vec2 vec2_subtract(const Vec2* vec_a, const Vec2* vec_b) {
    Vec2 result;
    result.x = vec_a->x - vec_b->x;
    result.y = vec_a->y - vec_b->y;

    return result;
}

void vec2_scale_inplace(Vec2* vec, const float scalar) {
    vec->x = vec->x * scalar;
    vec->y = vec->y * scalar;
}

Vec2 vec2_scale(const Vec2* vec, const float scalar) {
    Vec2 result;
    result.x = vec->x * scalar;
    result.y = vec->y * scalar;

    return result;
}

float vec2_length(const Vec2* vec) {
    const float vec_length = SDL_sqrtf((vec->x * vec->x) + (vec->y * vec->y));

    return vec_length;
}

float vec2_dotproduct(const Vec2* vec_a, const Vec2* vec_b) {
    const float dot_product = (vec_a->x * vec_b->x) + (vec_a->y * vec_b->y);

    return dot_product;
}


typedef struct {
    Vec2 position;
    Vec2 velocity;
    float radius;
    float mass;
    SDL_FColor color;
} Ball;

typedef struct {
    int cell[GRID_CELL_SIZE]; // to be replaced with an expandable data format
    int neighbours[8];
} GridCell;

typedef struct IntListNode {
    int value;
    struct IntListNode* next;
} IntListNode;

typedef struct {
    int length;
    IntListNode* node;
} IntList;

IntListNode* create_list_node(const int value) {
    IntListNode* node = SDL_malloc(sizeof(IntListNode));

    node->value = value;
    node->next = NULL;

    return node;
}

IntList* new_list() {
    // holds linked list metadata and pointer to the first node
    IntList* list = SDL_malloc(sizeof(IntList));

    list->node = NULL;
    list->length = -1;

    return list;
}


void append_list(IntList* list, int value) {
    // handle empty list
    if (list->node == NULL) {
        list->node = create_list_node(value);
        list->length += 1;
        return;
    }

    // first node
    IntListNode* current_node = list->node;
    // following node (NULL if there's only one node)
    IntListNode* next_node = current_node->next;

    // traverse nodes until the end is reached
    while (next_node != NULL) {
        current_node = next_node;
        next_node = current_node->next;
    }
    
    current_node->next = create_list_node(value);
    list->length += 1;
}

void print_list(const IntList* list) {
    const IntListNode* current_node = list->node;
    
    for (int i = 0; i < list->length; i++) {
        printf("%i\n", current_node->value);
        current_node = current_node->next;
    }
}

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

    GridCell* collision_grid;
    int collision_grid_stride_x;
    int collision_grid_size;

    Ball ball_array[NUM_BALLS];

} AppState;

int ball_position_to_grid(const AppState* as, const int ball_index) {
    const Vec2 position = as->ball_array[ball_index].position;
    // convert to grid coordinates in x and y
    const int grid_x = (int)position.x / (MAX_BALL_RADIUS * 2);
    const int grid_y = (int)position.y / (MAX_BALL_RADIUS * 2);
    // convert x and y grid coords to flattened grid
    const int grid_position = (grid_y * as->collision_grid_stride_x) + grid_x;

    return grid_position;
}

int get_collision_grid_size() {
    const float circumference = MAX_BALL_RADIUS * 2;

    const int x_cells = (int)SDL_ceilf(WINDOW_WIDTH / circumference);
    const int y_cells = (int)SDL_ceilf(WINDOW_HEIGHT / circumference);

    const int num_cells = x_cells * y_cells;

    return num_cells;
}

int get_collision_grid_stride_x() {
    const float circumference = MAX_BALL_RADIUS * 2;
    const int x_cells = (int)SDL_ceilf(WINDOW_WIDTH / circumference);

    return x_cells;
}

void populate_cell_neighbours(const AppState* as, const int cell_index) {
    /* cell neighbour order
     * 0, 1, 2
     * 3, *, 4
     * 5, 6, 7
     */
    
    GridCell* cell = &as->collision_grid[cell_index];
    
    // fill cells
    cell->neighbours[0] = cell_index - as->collision_grid_stride_x - 1;
    cell->neighbours[1] = cell_index - as->collision_grid_stride_x;
    cell->neighbours[2] = cell_index - as->collision_grid_stride_x + 1;
    cell->neighbours[3] = cell_index - 1;
    cell->neighbours[4] = cell_index + 1;
    cell->neighbours[5] = cell_index + as->collision_grid_stride_x - 1;
    cell->neighbours[6] = cell_index + as->collision_grid_stride_x;
    cell->neighbours[7] = cell_index + as->collision_grid_stride_x + 1;

    // handle top edge
    if (cell_index < as->collision_grid_stride_x) {
        cell->neighbours[0] = -1;
        cell->neighbours[1] = -1;
        cell->neighbours[2] = -1;
    }

    // handle bottom edge
    if (cell_index >= (as->collision_grid_size - as->collision_grid_stride_x)) {
        cell->neighbours[5] = -1;
        cell->neighbours[6] = -1;
        cell->neighbours[7] = -1;
    }

    // handle left edge
    if (cell_index % as->collision_grid_stride_x == 0) {
        cell->neighbours[0] = -1;
        cell->neighbours[3] = -1;
        cell->neighbours[5] = -1;
    }

    // handle right edge
    if (cell_index % as->collision_grid_stride_x == as->collision_grid_stride_x - 1) {
        cell->neighbours[2] = -1;
        cell->neighbours[4] = -1;
        cell->neighbours[7] = -1;
    }
}

void init_collision_grid(AppState* as) {
    as->collision_grid_size = get_collision_grid_size();
    as->collision_grid_stride_x = get_collision_grid_stride_x();
    as->collision_grid = (GridCell*)SDL_calloc(as->collision_grid_size, sizeof(GridCell));

    for (int i = 0; i < as->collision_grid_size; i++) {
        populate_cell_neighbours(as, i);
    }
}

void clear_grid_cell(const AppState* as, const int cell_index) {
    for (int i = 0; i < GRID_CELL_SIZE; i++) {
        as->collision_grid[cell_index].cell[i] = -1;
    }
}

void add_ball_to_grid(const AppState* as, const int ball_index, const int cell_index) {
    for (int i = 0; i < GRID_CELL_SIZE; i++) {
        if (as->collision_grid[cell_index].cell[i] == -1) {
            as->collision_grid[cell_index].cell[i] = ball_index;
            return;
        }
    }
}

bool is_cell_occupied(const AppState* as, const int cell_index) {
    if (as->collision_grid[cell_index].cell[0] != -1) {
        return true;
    } else {
        return false;
    }
}

void reset_collision_grid(const AppState* as) {
    // reset grid cells to -1 (empty cell)
    for (int i = 0; i < as->collision_grid_size; i++) {
        clear_grid_cell(as, i);
    }
    // populate grid with ball indices
    for (int i = 0; i < NUM_BALLS; i++) {
        const int grid_cell = ball_position_to_grid(as, i);
        add_ball_to_grid(as, i, grid_cell);
    }
}

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

SDL_FColor hsv_to_rgb(float hue, float saturation, float value) {
    // hue range 0-1
    // saturation range 0-1
    // value range 0-1

    float r;
    float g;
    float b;

    float hue_range = hue * 6.0;
    // SDL_Log("hue range: %i", (int)hue_range);
    switch((int)hue_range) {
        case 0:  // red - yellow
            r = 1.0;
            g = hue_range;
            b = 0.0;
            break;
        case 1:  // yellow - green
            r = 1.0 - (hue_range - 1.0);
            g = 1.0;
            b = 0.0;
            break;
        case 2:  // green - cyan
            r = 0.0;
            g = 1.0;
            b = hue_range - 2.0;
            break;
        case 3:  // cyan - blue
            r = 0.0;
            g = 1.0 - (hue_range - 3.0);
            b = 1.0;
            break;
        case 4:  // blue - magenta
            r = hue_range - 4.0;
            g = 0.0;
            b = 1.0;
            break;
        case 5:  // magenta - red
            r = 1.0;
            g = 0.0;
            b = 1.0 - (hue_range - 5.0);
            break;
        default:
            r = 1.0;
            g = 0.0;
            b = 0.0;
    }

    const SDL_FColor rgba = {r, g, b, 1.0};

    return rgba;
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

// ball physics
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

// void handle_ball_collision_no_mass(AppState* as, int ball_index_a) {
//     int array_len = sizeof(as->ball_array) / sizeof(Ball);
//     for (int ball_index_b = 0; ball_index_b < array_len; ball_index_b++) {
//         if (ball_index_b == ball_index_a) {
//             continue;
//         } else {

//             Vec2 distance_vec = vec2_subtract(&as->ball_array[ball_index_b].position, &as->ball_array[ball_index_a].position);
//             float ball_distance = vec2_length(&distance_vec);
//             float min_distance = as->ball_array[ball_index_a].radius + as->ball_array[ball_index_b].radius;

//             if (ball_distance < min_distance) {
//                 // scale distance vector to become unit collision normal
//                 float normalise_factor = 1.0 / ball_distance;
//                 Vec2 collision_normal = vec2_scale(&distance_vec, normalise_factor);

//                 // move balls apart
//                 float move_distance = min_distance - ball_distance;
            
//                 Vec2 move_vec_a = vec2_scale(&collision_normal, move_distance / 2.0 * -1.0);
//                 Vec2 new_position_a = vec2_add(&as->ball_array[ball_index_a].position, &move_vec_a);
//                 as->ball_array[ball_index_a].position = new_position_a;
                
//                 Vec2 move_vec_b = vec2_scale(&collision_normal, move_distance / 2.0);
//                 Vec2 new_position_b = vec2_add(&as->ball_array[ball_index_b].position, &move_vec_b);
//                 as->ball_array[ball_index_b].position = new_position_b;
                
//                 // calculate new velocities
//                 float dot_product_a = vec2_dotproduct(&as->ball_array[ball_index_a].velocity, &collision_normal);
//                 Vec2 transform_a = vec2_scale(&collision_normal, -2.0 * dot_product_a);
//                 vec2_add_inplace(&as->ball_array[ball_index_a].velocity, &transform_a);

//                 float dot_product_b = vec2_dotproduct(&as->ball_array[ball_index_b].velocity, &collision_normal);
//                 Vec2 transform_b = vec2_scale(&collision_normal, -2.0 * dot_product_b);
//                 vec2_add_inplace(&as->ball_array[ball_index_b].velocity, &transform_b);
//             }
//         }
//     }
// }

void handle_ball_collision_no_mass(AppState* as, int ball_index_a, int ball_index_b) {

    Vec2 distance_vec = vec2_subtract(&as->ball_array[ball_index_b].position, &as->ball_array[ball_index_a].position);
    float ball_distance = vec2_length(&distance_vec);
    float min_distance = as->ball_array[ball_index_a].radius + as->ball_array[ball_index_b].radius;

    if (ball_distance < min_distance) {
        // scale distance vector to become unit collision normal
        float normalise_factor = 1.0 / ball_distance;
        Vec2 collision_normal = vec2_scale(&distance_vec, normalise_factor);

        // move balls apart
        float move_distance = min_distance - ball_distance;
    
        Vec2 move_vec_a = vec2_scale(&collision_normal, move_distance / 2.0 * -1.0);
        Vec2 new_position_a = vec2_add(&as->ball_array[ball_index_a].position, &move_vec_a);
        as->ball_array[ball_index_a].position = new_position_a;
        
        Vec2 move_vec_b = vec2_scale(&collision_normal, move_distance / 2.0);
        Vec2 new_position_b = vec2_add(&as->ball_array[ball_index_b].position, &move_vec_b);
        as->ball_array[ball_index_b].position = new_position_b;
        
        // calculate new velocities
        float dot_product_a = vec2_dotproduct(&as->ball_array[ball_index_a].velocity, &collision_normal);
        Vec2 transform_a = vec2_scale(&collision_normal, -2.0 * dot_product_a);
        vec2_add_inplace(&as->ball_array[ball_index_a].velocity, &transform_a);

        float dot_product_b = vec2_dotproduct(&as->ball_array[ball_index_b].velocity, &collision_normal);
        Vec2 transform_b = vec2_scale(&collision_normal, -2.0 * dot_product_b);
        vec2_add_inplace(&as->ball_array[ball_index_b].velocity, &transform_b);
    }
}

void update_balls(AppState* as, int substeps) {

    double frame_time = 1.0 / 60.0;
    double delta_t = frame_time / substeps;

    for (int i = 0; i < substeps; i++) {
        for (int ball_index = 0; ball_index < NUM_BALLS; ball_index++) {
            // apply gravity
            as->ball_array[ball_index].velocity.y += (as->gravity * as->scene_scale * delta_t);

            // update position
            Vec2 timestep_velocity = (Vec2){as->ball_array[ball_index].velocity.x * delta_t * as->scene_scale,
                as->ball_array[ball_index].velocity.y * as-> scene_scale * delta_t};
            vec2_add_inplace(&as->ball_array[ball_index].position, &timestep_velocity);
            
            // handle collision with other balls via grid neighbour lookup
            int grid_cell = ball_position_to_grid(as, ball_index);
            
            // look for other balls in same cell
            for (int j = 0; j < GRID_CELL_SIZE; j++) {
                int grid_subcell = as->collision_grid[grid_cell].cell[j];
                if (grid_subcell == -1) {
                    break;
                } else if (grid_subcell == ball_index) {
                    continue;
                } else {
                    handle_ball_collision_no_mass(as, ball_index, grid_subcell);
                }
            }

            // look for populated neighbours
            for (int neighbour = 0; neighbour < 8; neighbour++) {
                int neighbour_cell = as->collision_grid[grid_cell].neighbours[neighbour]; 
                if (neighbour_cell != -1) {
                    // check if the neighbouring cell contains balls
                    for (int j = 0; j < GRID_CELL_SIZE; j++) {
                        int grid_subcell = as->collision_grid[neighbour_cell].cell[j];
                        if (grid_subcell != -1) {
                            handle_ball_collision_no_mass(as, ball_index, grid_subcell);
                        }
                    }
                }
            }

            // handle collision with screen bounds
            handle_screen_edge_collision(as, ball_index);
            
        }
    }
    as->last_time = SDL_GetTicksNS();
}

/* Nearest neighbour lookup for collisions 

- create 2d grid for ball indices
grid size should be based on the maximum ball size and minimum potential for collision (radius * 2?)
should grid be structured as a flattened array with strides or an array of arrays? try both
create helper functions
    to convert grid x, y coordinates to correct grid array index
    to get neighbouring grid coordinate indices, accounting for edges and corners

- put ball indices into the grid based on the .position
need to convert between coordinate systems, 

- iterate through the grid squares.
if the square contains an index:
    check if it contains more than one index or check if the surrounding squares contain any indices. if so:
        perform collision checks
    note - be careful of grid edges/corners, as checks can go out of bounds (literally)
    if there are two or more indices in total iany of them are populated perform the collision check

*/

// SDL callbacks
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    // create appstate, use calloc to zero out values
    AppState* as = (AppState*)SDL_calloc(1, sizeof(AppState));
    *appstate = as;
    // SDL_Log("ball array length: %lu", sizeof(as->ball_array) / sizeof(Ball));
    as->window_height = WINDOW_HEIGHT;
    as->window_width = WINDOW_WIDTH;
    
    as->last_time = SDL_GetTicksNS();

    // initialise ball physics
    as->gravity = 9.8;
    as->scene_scale = 20.0;
    
    // set up collision grid
    init_collision_grid(as);

    for (int i = 0; i < NUM_BALLS; i++) {
        // initialise balls
        as->ball_array[i].position = (Vec2){SDL_randf() * as->window_width, SDL_randf() * as->window_height};
        as->ball_array[i].velocity = (Vec2){(SDL_randf() - 0.5) * 2.0 * as->scene_scale, (SDL_randf() - 0.5) * as->scene_scale};
        // as->ball_array[i].radius = (SDL_randf() * 15.0) + 5.0;
        as->ball_array[i].radius = MAX_BALL_RADIUS;
        as->ball_array[i].mass = SDL_PI_F * as->ball_array[i].radius * as->ball_array[i].radius; // use ball area
        as->ball_array[i].color = hsv_to_rgb(SDL_randf(), 1.0, 1.0); 
        // as->ball_array[i].color = (SDL_FColor){SDL_randf(), SDL_randf(), SDL_randf(), SDL_ALPHA_OPAQUE_FLOAT};

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

        // put ball in grid
        int cell_index = ball_position_to_grid(as, i);
        add_ball_to_grid(as, i, cell_index);
        // SDL_Log("ball %i pos: %f, %f", i, as->ball_array[i].position.x, as->ball_array[i].position.y);
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
    update_balls(as, SIM_SUBSTEPS);

    reset_collision_grid(as);

    for (int i = 0; i < NUM_BALLS; i++) {
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