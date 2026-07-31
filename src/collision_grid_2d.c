#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

/** A single cell in the spatial grid. Items are stored in a flat array,
 *  with each cell tracking its contiguous range via start + count. */
typedef struct {
    int start;
    int count;
    int neighbours[8];
} GridCell2D;

/** A 2D spatial hash grid that partitions space into uniform cells.
 *  Items are placed into flat_items via a two-pass (count + insert) approach
 *  that avoids per-frame heap allocations. */
typedef struct {
    GridCell2D* cells;
    int* flat_items;
    int grid_row_length;
    int num_cells;
    float cell_size;
    int max_items;
} Grid2D;

void populate_cell_neighbours(Grid2D* grid, int cell_index);
void reset_cell_counts(Grid2D* grid);

/** Return the total number of grid cells needed to cover a size_x by size_y area */
int get_grid_size(float size_x, float size_y, float cell_size) {
    const int x_cells = (int)ceilf(size_x / cell_size);
    const int y_cells = (int)ceilf(size_y / cell_size);
    return x_cells * y_cells;
}

/** Return the number of grid cells along the x axis */
int get_grid_row_length(float size_x, float cell_size) {
    return (int)ceilf(size_x / cell_size);
}

/** Precompute the 8 neighbour indices for a given cell. Out-of-bounds
 *  neighbours are set to -1. Layout:
 *    0 1 2
 *    3 * 4
 *    5 6 7   */
void populate_cell_neighbours(Grid2D* grid, int cell_index) {
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

/** Create a new grid covering the given dimensions with uniform cell_size.
 *  flat_items is sized to hold up to max_items total across all cells.
 *  Returns NULL on allocation failure. */
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

    reset_cell_counts(grid);

    return grid;
}

/** Free all memory associated with grid */
void free_grid(Grid2D* grid) {
    if (!grid) return;
    free(grid->flat_items);
    free(grid->cells);
    free(grid);
}

/** Zero out all cell counts in preparation for the count pass */
void reset_cell_counts(Grid2D* grid) {
    for (int i = 0; i < grid->num_cells; i++) {
        grid->cells[i].count = 0;
    }
}

/** Count pass: increment the item count for the given cell */
void increment_cell_count(Grid2D* grid, int cell_index) {
    grid->cells[cell_index].count++;
}

/** Build pass: compute prefix sums (start offsets) from the per-cell counts
 *  accumulated during the count pass. Resets each count to 0 in preparation
 *  for the insert pass. */
void compute_cell_offsets(Grid2D* grid) {
    int cursor = 0;
    for (int i = 0; i < grid->num_cells; i++) {
        int c = grid->cells[i].count;
        grid->cells[i].start = cursor;
        cursor += c;
        grid->cells[i].count = 0;
    }
}

/** Insert pass: place item_index into the flat array at the next available
 *  position within the cell's reserved range. Requires build_grid() to have
 *  been called first. */
void insert_item(Grid2D* grid, int item_index, int cell_index) {
    GridCell2D* cell = &grid->cells[cell_index];
    grid->flat_items[cell->start + cell->count] = item_index;
    cell->count++;
}

/** Convert an (x, y) position to the corresponding grid cell index,
 *  clamping out-of-bounds coordinates to the nearest edge cell */
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
