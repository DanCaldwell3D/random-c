#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#define GRID_CELL_SIZE 16

typedef struct {
    int cell[GRID_CELL_SIZE]; // to be replaced with an expandable data format
    int neighbours[8]; // store neighbouring cells for easy lookup
} GridCell2D;

typedef struct {
    GridCell2D* grid_cells;
    int grid_stride;
    int num_cells;
    float cell_size;
} Grid2D;

// int ball_position_to_grid(const AppState* as, const int ball_index) {
//     const Vec2 position = as->ball_array[ball_index].position;
//     const int cell = (int)(MAX_BALL_RADIUS * 2);
//     int grid_x = (int)position.x / cell;
//     int grid_y = (int)position.y / cell;

//     const int max_x = as->collision_grid_stride_x - 1;
//     const int max_y = (as->collision_grid_size / as->collision_grid_stride_x) - 1;

//     if (grid_x < 0) grid_x = 0;
//     else if (grid_x > max_x) grid_x = max_x;

//     if (grid_y < 0) grid_y = 0;
//     else if (grid_y > max_y) grid_y = max_y;

//     return grid_y * as->collision_grid_stride_x + grid_x;
// }

int get_2d_collision_grid_size(const float size_x, const float size_y, const float cell_size) {
    
    const int x_cells = (int)ceilf(size_x / cell_size);
    const int y_cells = (int)ceilf(size_y / cell_size);
    
    const int num_cells = x_cells * y_cells;

    return num_cells;
}

int get_2d_collision_grid_stride_x(const float size_x, const float cell_size) {

    const int x_cells = (int)ceilf(size_x / cell_size);

    return x_cells;
}

Grid2D* new_2d_collision_grid(const float size_x, const float size_y, const float cell_size) {

    Grid2D* grid = malloc(sizeof(Grid2D));

    grid->cell_size = cell_size;
    grid->num_cells = get_2d_collision_grid_size(size_x, size_y, cell_size);
    grid->grid_stride = get_2d_collision_grid_stride_x(size_x, cell_size);

    grid->grid_cells = (GridCell2D*)calloc(grid->num_cells, sizeof(GridCell2D));

    return grid;
}

void clear_2d_grid_cell(const Grid2D* grid, const int cell_index) {

    for (int i = 0; i < GRID_CELL_SIZE; i++) {
        grid->grid_cells[cell_index].cell[i] = -1;
    }

}

void init_2d_collision_grid(const Grid2D* grid) {

    for (int i = 0; i < grid->num_cells; i++) {
        clear_2d_grid_cell(grid, i);

    }
}

void populate_2d_cell_neighbours(const Grid2D* grid, const int cell_index) {
    /* cell neighbour order
     * 0, 1, 2
     * 3, *, 4
     * 5, 6, 7
     */
    
    GridCell2D* cell = &grid->grid_cells[cell_index];
    
    // fill cells
    cell->neighbours[0] = cell_index - grid->grid_stride - 1;
    cell->neighbours[1] = cell_index - grid->grid_stride;
    cell->neighbours[2] = cell_index - grid->grid_stride + 1;
    cell->neighbours[3] = cell_index - 1;
    cell->neighbours[4] = cell_index + 1;
    cell->neighbours[5] = cell_index + grid->grid_stride - 1;
    cell->neighbours[6] = cell_index + grid->grid_stride;
    cell->neighbours[7] = cell_index + grid->grid_stride + 1;

    // handle top edge
    if (cell_index < grid->grid_stride) {
        cell->neighbours[0] = -1;
        cell->neighbours[1] = -1;
        cell->neighbours[2] = -1;
    }

    // handle bottom edge
    if (cell_index >= (grid->num_cells - grid->grid_stride)) {
        cell->neighbours[5] = -1;
        cell->neighbours[6] = -1;
        cell->neighbours[7] = -1;
    }

    // handle left edge
    if (cell_index % grid->grid_stride == 0) {
        cell->neighbours[0] = -1;
        cell->neighbours[3] = -1;
        cell->neighbours[5] = -1;
    }

    // handle right edge
    if (cell_index % grid->grid_stride == grid->grid_stride - 1) {
        cell->neighbours[2] = -1;
        cell->neighbours[4] = -1;
        cell->neighbours[7] = -1;
    }
}


void init_collision_grid(AppState* as) {
    as->collision_grid_size = get_collision_grid_size();
    as->collision_grid_stride_x = get_collision_grid_stride_x();
    as->collision_grid = (GridCell2D*)SDL_calloc(as->collision_grid_size, sizeof(GridCell2D));
    
    if (!as->collision_grid) {
        SDL_Log("collision grid allocation failed: %s", SDL_GetError());
    } else {
        for (int i = 0; i < as->collision_grid_size; i++) {
            clear_grid_cell(as, i);
        }
    }

    for (int i = 0; i < as->collision_grid_size; i++) {
        populate_cell_neighbours(as, i);
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