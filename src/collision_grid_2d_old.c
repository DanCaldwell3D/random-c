#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#define GRID_CELL_SIZE 16

typedef struct {
    int cell[GRID_CELL_SIZE]; // to be replaced with an expandable data format
    int neighbours[8]; // store neighbouring cells for easy lookup
} GridCell2D;

typedef struct {
    GridCell2D* grid_cells;
    int grid_row_length;
    int num_cells;
    float cell_size;
} Grid2D;

// declarations

int get_2d_collision_grid_size(const float size_x, const float size_y, const float cell_size);
int get_2d_collision_grid_row_length(const float size_x, const float cell_size);
Grid2D* new_2d_collision_grid(const float size_x, const float size_y, const float cell_size);
void clear_2d_grid_cell(Grid2D* grid, const int cell_index);
void populate_2d_cell_neighbours(Grid2D* grid, const int cell_index);
void init_2d_collision_grid(Grid2D* grid);
void reset_2d_collision_grid(Grid2D* grid);
int add_item_index_to_grid(Grid2D* grid, const int item_index, const int cell_index);
bool is_cell_occupied(Grid2D* grid, const int cell_index);
int xy_location_to_grid_cell(Grid2D* grid, const float x, const float y);
void free_2d_collision_grid(Grid2D* grid);


int xy_location_to_grid_cell(Grid2D* grid, const float x, const float y) {
    // returns the grid cell that corresponds to the given x and y locations

    int grid_x = (int)x / grid->cell_size;
    int grid_y = (int)y / grid->cell_size;

    // bounds checking and enforcement
    const int max_x = grid->grid_row_length - 1;
    const int max_y = (grid->num_cells / grid->grid_row_length) - 1;

    if (grid_x < 0) grid_x = 0;
    else if (grid_x > max_x) grid_x = max_x;

    if (grid_y < 0) grid_y = 0;
    else if (grid_y > max_y) grid_y = max_y;

    return grid_y * grid->grid_row_length + grid_x;
}

int get_2d_collision_grid_size(const float size_x, const float size_y, const float cell_size) {
    // returns the total number of cells within the given area

    const int x_cells = (int)ceilf(size_x / cell_size);
    const int y_cells = (int)ceilf(size_y / cell_size);
    
    const int num_cells = x_cells * y_cells;

    return num_cells;
}

int get_2d_collision_grid_row_length(const float size_x, const float cell_size) {
    // returns the number of cells across the x axis

    const int x_cells = (int)ceilf(size_x / cell_size);

    return x_cells;
}

Grid2D* new_2d_collision_grid(const float size_x, const float size_y, const float cell_size) {
    // creates a new grid at the given cell size that covers the given dimensions
    // calculates and stores cell size, grid stride along x axis, and total number
    // of cells for easy traversal

    Grid2D* grid = malloc(sizeof(Grid2D));

    grid->cell_size = cell_size;
    grid->num_cells = get_2d_collision_grid_size(size_x, size_y, cell_size);
    grid->grid_row_length = get_2d_collision_grid_row_length(size_x, cell_size);

    grid->grid_cells = (GridCell2D*)calloc(grid->num_cells, sizeof(GridCell2D));

    if (!grid->grid_cells) {
        free_2d_collision_grid(grid);
        printf("Grid allocation failed");
        return NULL;
    }

    init_2d_collision_grid(grid);

    return grid;
}

void free_2d_collision_grid(Grid2D* grid) {
    free(grid->grid_cells);
    free(grid);
}

void clear_2d_grid_cell(Grid2D* grid, const int cell_index) {
    // sets all sub-cells to -1 in the given grid cell
    for (int i = 0; i < GRID_CELL_SIZE; i++) {
        grid->grid_cells[cell_index].cell[i] = -1;
    }
}

void populate_2d_cell_neighbours(Grid2D* grid, const int cell_index) {
    /* cell neighbour order
     * 0, 1, 2
     * 3, *, 4
     * 5, 6, 7
     */
    
    GridCell2D* cell = &grid->grid_cells[cell_index];
    
    // fill cells
    cell->neighbours[0] = cell_index - grid->grid_row_length - 1;
    cell->neighbours[1] = cell_index - grid->grid_row_length;
    cell->neighbours[2] = cell_index - grid->grid_row_length + 1;
    cell->neighbours[3] = cell_index - 1;
    cell->neighbours[4] = cell_index + 1;
    cell->neighbours[5] = cell_index + grid->grid_row_length - 1;
    cell->neighbours[6] = cell_index + grid->grid_row_length;
    cell->neighbours[7] = cell_index + grid->grid_row_length + 1;

    // handle top edge
    if (cell_index < grid->grid_row_length) {
        cell->neighbours[0] = -1;
        cell->neighbours[1] = -1;
        cell->neighbours[2] = -1;
    }

    // handle bottom edge
    if (cell_index >= (grid->num_cells - grid->grid_row_length)) {
        cell->neighbours[5] = -1;
        cell->neighbours[6] = -1;
        cell->neighbours[7] = -1;
    }

    // handle left edge
    if (cell_index % grid->grid_row_length == 0) {
        cell->neighbours[0] = -1;
        cell->neighbours[3] = -1;
        cell->neighbours[5] = -1;
    }

    // handle right edge
    if (cell_index % grid->grid_row_length == grid->grid_row_length - 1) {
        cell->neighbours[2] = -1;
        cell->neighbours[4] = -1;
        cell->neighbours[7] = -1;
    }
}

void init_2d_collision_grid(Grid2D* grid) {
    // set all grid cells to -1 (empty) and initialise neighbour grid lookups
    for (int i = 0; i < grid->num_cells; i++) {
        clear_2d_grid_cell(grid, i);
        populate_2d_cell_neighbours(grid, i);
    }
}
void reset_2d_collision_grid(Grid2D* grid) {
    // set all grid cells to -1 (empty)
    for (int i = 0; i < grid->num_cells; i++) {
        clear_2d_grid_cell(grid, i);
    }
}

int add_item_index_to_grid(Grid2D* grid, const int item_index, const int cell_index) {
    // adds an item index to the given grid cell. returns 0 if successful,
    // 1 if item or cell index are out of range, or 2 if grid cell is full
    if (item_index < 0 | cell_index < 0 | cell_index > grid->num_cells)
    {
        return 1;
    }

    for (int i = 0; i < GRID_CELL_SIZE; i++) {

        if (grid->grid_cells[cell_index].cell[i] == -1) {
            grid->grid_cells[cell_index].cell[i] = item_index;
            return 0;
        }
    }
    return 2;
}

bool is_cell_occupied(Grid2D* grid, const int cell_index) {
    // returns true if the first sub-cell is occupied

    if (grid->grid_cells[cell_index].cell[0] != -1) {
        return true;
    } else {
        return false;
    }
}

// void reset_collision_grid(const Grid2D* grid) {
//     // reset grid cells to -1 (empty cell)
//     for (int i = 0; i < grid->num_cells; i++) {
//         clear_2d_grid_cell(grid, i);
//     }
//     // populate grid with ball indices
//     for (int i = 0; i < NUM_BALLS; i++) {
//         const int grid_cell = ball_position_to_grid(as, i);
//         add_ball_to_grid(as, i, grid_cell);
//     }
// }