#ifndef COLLISION_GRID_2D_H
#define COLLISION_GRID_2D_H

/*
 *  collision_grid_2d — spatial hash grid with two-pass insertion
 *
 *  Usage (four steps per frame):
 *
 *    1. Zero counts:     reset_cell_counts(grid);
 *
 *    2. Count pass:      for each item i:
 *                            cell = xy_to_cell(grid, i.x, i.y);
 *                            increment_cell_count(grid, cell);
 *
 *    3. Build pass:      compute_cell_offsets(grid);
 *
 *    4. Insert pass:     for each item i:
 *                            cell = xy_to_cell(grid, i.x, i.y);
 *                            insert_item(grid, i, cell);
 *
 *    After step 4, iterate neighbours to find collision pairs:
 *        for each cell c (with count > 0):
 *            for j = 0 .. c->count:
 *                item_a = grid->flat_items[c->start + j]
 *                for each neighbour n of c:
 *                    // pair (item_a, item_b) for collision check
 */

/** A cell in the spatial grid. */
typedef struct {
    int start;          // start location in flat lookup array
    int count;          // number of items in cell
    int neighbours[8];  // neighbouring cell indices lookup
} GridCell2D;

/** 2D spatial hash grid. Items are stored in flat_items via a two-pass
 *  (count + insert) approach to avoid per-frame heap allocations. */
typedef struct {
    GridCell2D* cells;      // array of cells
    int* flat_items;        // flat item lookup array
    int grid_row_length;    // number of cells in a grid row
    int num_cells;          // total number of cells in the grid
    float cell_size;        // physical size of cell
    int max_items;          // max items to store
} Grid2D;

/** Create a grid covering size_x by size_y with uniform cell_size.
 *  flat_items sized to hold up to max_items total.
 *  Resets grid on creation (step 1)
 *  Returns NULL on allocation failure. */
Grid2D* new_grid(float size_x, float size_y, float cell_size, int max_items);

/** Free all memory associated with grid */
void free_grid(Grid2D* grid);

/** Zero all cell counts (step 1) */
void reset_cell_counts(Grid2D* grid);

/** Increment count for cell_index, call
*   xy_to_cell to get index (step 2) */
void increment_cell_count(Grid2D* grid, int cell_index);

/** Compute prefix-sum start offsets and reset counts (step 3) */
void compute_cell_offsets(Grid2D* grid);

/** Insert item_index at next slot in cell's range (step 4) */
void insert_item(Grid2D* grid, int item_index, int cell_index);

/** Convert (x, y) to cell index, clamping to edges */
int xy_to_cell(Grid2D* grid, float x, float y);

#endif // COLLISION_GRID_2D_H
