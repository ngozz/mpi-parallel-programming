#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 32  // Width of the grid
#define HEIGHT 32 // Height of the grid
// #define ITERATIONS 10 // This line is commented out or removed

void initializeGrid(int *grid, int width, int height);
void printGrid(int *grid, int width, int height);
void updateGrid(int *grid, int *newGrid, int width, int height);

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2)
    {
        if (rank == 0)
        {
            printf("Usage: %s <iterations>\n", argv[0]);
            printf("Note: Use 0 for iterations to run indefinitely.\n");
        }
        MPI_Finalize();
        return 1;
    }

    int iterations = atoi(argv[1]); // Convert the argument to an integer

    int sliceHeight = HEIGHT / size; // Height of each slice
    int *grid;
    int *slice = (int *)malloc(WIDTH * sliceHeight * sizeof(int));
    int *newSlice = (int *)malloc(WIDTH * sliceHeight * sizeof(int));

    // Master process initializes the grid and distributes it
    if (rank == 0)
    {
        grid = (int *)malloc(WIDTH * HEIGHT * sizeof(int));
        initializeGrid(grid, WIDTH, HEIGHT);
    }
    MPI_Scatter(grid, WIDTH * sliceHeight, MPI_INT, slice, WIDTH * sliceHeight, MPI_INT, 0, MPI_COMM_WORLD);

    double startTime = MPI_Wtime(); // Start timing

    // Inside the main function, modify the simulation loop

    // Simulation loop
    int iter = 0;
    while (iterations == 0 || iter < iterations)
    {
        // Update slice
        updateGrid(slice, newSlice, WIDTH, sliceHeight);

        // Swap pointers for next iteration
        int *temp = slice;
        slice = newSlice;
        newSlice = temp;

        // Gather the slices back to the master process for printing
        MPI_Gather(slice, WIDTH * sliceHeight, MPI_INT, grid, WIDTH * sliceHeight, MPI_INT, 0, MPI_COMM_WORLD);

        // Master process prints the grid
        if (rank == 0)
        {
            system("cls"); // Clear the console for Windows, use "clear" for Linux/Mac
            printf("Iteration: %d\n", iter + 1);
            printGrid(grid, WIDTH, HEIGHT);
            Sleep(1); // Pause for a second (Windows), use sleep(1) for Linux/Mac
        }

        // Optionally, redistribute the grid back to all processes if necessary
        // MPI_Scatter(grid, WIDTH * sliceHeight, MPI_INT, slice, WIDTH * sliceHeight, MPI_INT, 0, MPI_COMM_WORLD);

        if (iterations != 0)
        {
            iter++;
        }
    }

    double endTime = MPI_Wtime(); // End timing

    // Gather the slices back to the master process
    MPI_Gather(slice, WIDTH * sliceHeight, MPI_INT, grid, WIDTH * sliceHeight, MPI_INT, 0, MPI_COMM_WORLD);

    // Master process prints the final grid and timing information
    if (rank == 0)
    {
        printf("Final grid:\n");
        printGrid(grid, WIDTH, HEIGHT);
        printf("Number of processes: %d\n", size);
        printf("Time taken: %f seconds\n", endTime - startTime);
        free(grid);
    }

    free(slice);
    free(newSlice);
    MPI_Finalize();
    return 0;
}

void initializeGrid(int *grid, int width, int height)
{
    srand(time(NULL)); // Seed the random number generator
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            grid[i * width + j] = rand() % 2; // Randomly initialize cells as alive (1) or dead (0)
        }
    }
}

void printGrid(int *grid, int width, int height)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            printf("%c ", grid[i * width + j] ? 'X' : '.'); // Print 'X' for alive cells, '.' for dead
        }
        printf("\n");
    }
}

void updateGrid(int *grid, int *newGrid, int width, int height)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int aliveNeighbors = 0;
            // Check all eight neighbors
            for (int y = -1; y <= 1; y++)
            {
                for (int x = -1; x <= 1; x++)
                {
                    if (x == 0 && y == 0)
                        continue;                       // Skip the cell itself
                    int ni = (i + y + height) % height; // Wrap around edges
                    int nj = (j + x + width) % width;
                    aliveNeighbors += grid[ni * width + nj];
                }
            }
            // Apply the Game of Life rules
            if (grid[i * width + j] == 1 && (aliveNeighbors < 2 || aliveNeighbors > 3))
            {
                newGrid[i * width + j] = 0; // Cell dies
            }
            else if (grid[i * width + j] == 0 && aliveNeighbors == 3)
            {
                newGrid[i * width + j] = 1; // Cell becomes alive
            }
            else
            {
                newGrid[i * width + j] = grid[i * width + j]; // No change
            }
        }
    }
}

// a parallel implementation of the Conway's Game of Life using MPI. This example demonstrates the use of parallel computing to simulate a cellular automaton on a distributed system. The Game of Life is a zero-player game, meaning its evolution is determined by its initial state, requiring no further input. It consists of a grid of cells that can live, die, or multiply based on a set of rules.

// Pseudocode
// Initialize MPI: Start MPI, get the total number of processes, and determine the rank of the current process.
// Distribute the Grid:
// The master process (rank 0) initializes the grid with an initial state.
// The grid is divided into equal horizontal slices, and each slice is sent to a different process.
// Simulation Loop:
// Each process computes the next state of its slice based on the current state and the state of its neighbors' edge rows.
// Processes exchange edge rows with their neighbors to maintain the correct state of the grid.
// Gather the Grid:
// After a predetermined number of iterations, all slices are gathered back to the master process.
// Finalize MPI: Close the MPI environment.