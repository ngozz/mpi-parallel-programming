#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 32
#define HEIGHT 32
#define ITERATIONS 100000 // Fixed number of iterations

void initializeGrid(int *grid, int width, int height);
void printGrid(int *grid, int width, int height);
void updateGrid(int *grid, int *newGrid, int width, int height, int rank, int size);

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int sliceHeight = HEIGHT / size;
    int *grid;
    int *slice = (int *)malloc(WIDTH * sliceHeight * sizeof(int));
    int *newSlice = (int *)malloc(WIDTH * sliceHeight * sizeof(int));

    if (rank == 0) {
        grid = (int *)malloc(WIDTH * HEIGHT * sizeof(int));
        initializeGrid(grid, WIDTH, HEIGHT);
    }

    MPI_Scatter(grid, WIDTH * sliceHeight, MPI_INT, slice, WIDTH * sliceHeight, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize before starting the timer
    double startTime = MPI_Wtime();

    for (int iter = 0; iter < ITERATIONS; iter++) {
        updateGrid(slice, newSlice, WIDTH, sliceHeight, rank, size);

        int *temp = slice;
        slice = newSlice;
        newSlice = temp;
    }

    MPI_Gather(slice, WIDTH * sliceHeight, MPI_INT, grid, WIDTH * sliceHeight, MPI_INT, 0, MPI_COMM_WORLD);

    double endTime = MPI_Wtime();

    if (rank == 0) {
        printGrid(grid, WIDTH, HEIGHT); // Print final grid
        printf("Number of processes: %d\n", size);
        printf("Execution time: %f seconds\n", endTime - startTime);
        free(grid);
    }

    free(slice);
    free(newSlice);

    MPI_Finalize();
    return 0;
}

void initializeGrid(int *grid, int width, int height) {
    srand(time(NULL));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            grid[i * width + j] = rand() % 2;
        }
    }
}

void printGrid(int *grid, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%c ", grid[i * width + j] ? 'X' : '.');
        }
        printf("\n");
    }
}

void updateGrid(int *grid, int *newGrid, int width, int height, int rank, int size) {
    int above = rank - 1;
    int below = rank + 1;
    MPI_Status status;

    // Temporary arrays to store received rows from neighbors
    int *topRow = (int *)malloc(width * sizeof(int));
    int *bottomRow = (int *)malloc(width * sizeof(int));

    // Send and receive the top row
    if (rank > 0) {
        MPI_Sendrecv(grid, width, MPI_INT, above, 0, topRow, width, MPI_INT, above, 0, MPI_COMM_WORLD, &status);
    }

    // Send and receive the bottom row
    if (rank < size - 1) {
        MPI_Sendrecv(&grid[width * (height - 1)], width, MPI_INT, below, 0, bottomRow, width, MPI_INT, below, 0, MPI_COMM_WORLD, &status);
    }

    // Compute new states for each cell
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int liveNeighbors = 0;

            // Determine the start and end indices for neighbors
            int rowStart = i > 0 ? i - 1 : (rank == 0 ? -1 : height - 1);
            int rowEnd = i < height - 1 ? i + 1 : (rank == size - 1 ? -1 : 0);
            int colStart = j > 0 ? j - 1 : -1;
            int colEnd = j < width - 1 ? j + 1 : -1;

            // Count live neighbors
            for (int x = rowStart; x <= rowEnd; x++) {
                for (int y = colStart; y <= colEnd; y++) {
                    if (x == i && y == j) continue; // Skip the cell itself
                    if (x == -1 && topRow[y] == 1) liveNeighbors++; // Top neighbor
                    else if (x == height && bottomRow[y] == 1) liveNeighbors++; // Bottom neighbor
                    else if (x >= 0 && x < height && y >= 0 && grid[x * width + y] == 1) liveNeighbors++;
                }
            }

            // Apply rules of the Game of Life
            if (grid[i * width + j] == 1 && (liveNeighbors < 2 || liveNeighbors > 3)) newGrid[i * width + j] = 0; // Die
            else if (grid[i * width + j] == 0 && liveNeighbors == 3) newGrid[i * width + j] = 1; // Live
            else newGrid[i * width + j] = grid[i * width + j]; // Stay the same
        }
    }

    free(topRow);
    free(bottomRow);
}