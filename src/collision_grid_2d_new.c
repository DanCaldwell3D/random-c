#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

typedef struct {
    int start;
    int count;
    int neighbours[8];
} GridCell2D;

typedef struct {
    GridCell2D* cells;
    int* flat_items;
    int grid_row_length;
    int num_cells;
    float cell_size;
    int max_items;
} Grid2D;

void populate_cell_neighbours(Grid2D* grid, int cell_index);
void reset_grid(Grid2D* grid);

int get_grid_size(float size_x, float size_y, float cell_size) {
    const int x_cells = (int)ceilf(size_x / cell_size);
    const int y_cells = (int)ceilf(size_y / cell_size);
    return x_cells * y_cells;
}

int get_grid_row_length(float size_x, float cell_size) {
    return (int)ceilf(size_x / cell_size);
}

void populate_cell_neighbours(Grid2D* grid, int cell_index) {
    /*
     * 0, 1, 2
     * 3, *, 4
     * 5, 6, 7
     */
    GridCell2D* cell = &grid->cells[cell_index];

    cell->neighbours[0] = cell_index - grid->grid_row_length - 1;
    cell->neighbours[1] = cell_index - grid->grid_row_length;
    cell->neighbours[2] = cell_index - grid->grid_row_length + 1;
    cell->neighbours[3] = cell_index - 1;
    cell->neighbours[4] = cell_index + 1;
    cell->neighbours[5] = cell_index + grid->grid_row_length - 1;
    cell->neighbours[6] = cell_index + grid->grid_row_length;
    cell->neighbours[7] = cell_index + grid->grid_row_length + 1;

    if (cell_index < grid->grid_row_length) {
        cell->neighbours[0] = -1;
        cell->neighbours[1] = -1;
        cell->neighbours[2] = -1;
    }

    if (cell_index >= (grid->num_cells - grid->grid_row_length)) {
        cell->neighbours[5] = -1;
        cell->neighbours[6] = -1;
        cell->neighbours[7] = -1;
    }

    if (cell_index % grid->grid_row_length == 0) {
        cell->neighbours[0] = -1;
        cell->neighbours[3] = -1;
        cell->neighbours[5] = -1;
    }

    if (cell_index % grid->grid_row_length == grid->grid_row_length - 1) {
        cell->neighbours[2] = -1;
        cell->neighbours[4] = -1;
        cell->neighbours[7] = -1;
    }
}

Grid2D* new_grid(float size_x, float size_y, float cell_size, int max_items) {
    Grid2D* grid = malloc(sizeof(Grid2D));
    if (!grid) return NULL;

    grid->cell_size = cell_size;
    grid->num_cells = get_grid_size(size_x, size_y, cell_size);
    grid->grid_row_length = get_grid_row_length(size_x, cell_size);
    grid->max_items = max_items;

    grid->cells = (GridCell2D*)calloc(grid->num_cells, sizeof(GridCell2D));
    if (!grid->cells) {
        free(grid);
        return NULL;
    }

    grid->flat_items = (int*)malloc(max_items * sizeof(int));
    if (!grid->flat_items) {
        free(grid->cells);
        free(grid);
        return NULL;
    }

    for (int i = 0; i < grid->num_cells; i++) {
        populate_cell_neighbours(grid, i);
    }

    reset_grid(grid);

    return grid;
}

void free_grid(Grid2D* grid) {
    if (!grid) return;
    free(grid->flat_items);
    free(grid->cells);
    free(grid);
}

void reset_grid(Grid2D* grid) {
    for (int i = 0; i < grid->num_cells; i++) {
        grid->cells[i].count = 0;
    }
}

void increment_cell_count(Grid2D* grid, int cell_index) {
    grid->cells[cell_index].count++;
}

void build_grid(Grid2D* grid) {
    int cursor = 0;
    for (int i = 0; i < grid->num_cells; i++) {
        int c = grid->cells[i].count;
        grid->cells[i].start = cursor;
        cursor += c;
        grid->cells[i].count = 0;
    }
}

void insert_item(Grid2D* grid, int item_index, int cell_index) {
    GridCell2D* cell = &grid->cells[cell_index];
    grid->flat_items[cell->start + cell->count] = item_index;
    cell->count++;
}

int xy_to_cell(Grid2D* grid, float x, float y) {
    int grid_x = (int)(x / grid->cell_size);
    int grid_y = (int)(y / grid->cell_size);

    const int max_x = grid->grid_row_length - 1;
    const int max_y = (grid->num_cells / grid->grid_row_length) - 1;

    if (grid_x < 0) grid_x = 0;
    else if (grid_x > max_x) grid_x = max_x;

    if (grid_y < 0) grid_y = 0;
    else if (grid_y > max_y) grid_y = max_y;

    return grid_y * grid->grid_row_length + grid_x;
}
