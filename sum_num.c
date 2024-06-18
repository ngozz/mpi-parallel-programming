#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int rank, size, i;
    long long int N; // Total number of elements
    int elements_per_proc;
    int *data = NULL, *sub_data;
    long long int local_sum = 0, total_sum = 0; // Use long long int for sums
    double startTime, endTime;

    MPI_Init(&argc, &argv);               // Initialize MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get current process rank
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get total number of processes

    // Print the number of processes (only by the master process)
    if (rank == 0) {
        printf("Number of processes: %d\n", size);
    }

    // Start timing
    startTime = MPI_Wtime();

    // Accept N from command line argument if provided, else use a default value
    if (argc > 1)
    {
        N = atoll(argv[1]); // Use atoll for converting string to long long int
    }
    else
    {
        if (rank == 0)
        { // Only the master process should handle the default or prompt for input
            printf("No input provided for N. Using default value 1000.\n");
        }
        N = 1000; // Default value
    }

    elements_per_proc = N / size; // Calculate number of elements per process

    // Master process prepares data
    if (rank == 0)
    {
        data = (int *)malloc(N * sizeof(int));
        for (i = 0; i < N; i++)
        {
            data[i] = i + 1; // Fill the array with numbers 1 to N
        }
    }

    // Allocate memory for sub-data in all processes
    sub_data = (int *)malloc(elements_per_proc * sizeof(int));

    // Distribute parts of the array to all processes
    MPI_Scatter(data, elements_per_proc, MPI_INT, sub_data, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process calculates its partial sum
    for (i = 0; i < elements_per_proc; i++)
    {
        local_sum += sub_data[i];
    }

    // Gather all partial sums to the master process and calculate the total sum
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // End timing
    endTime = MPI_Wtime();

    // Master process prints the result
    if (rank == 0)
    {
        printf("Total sum = %lld\n", total_sum);
        printf("Execution time: %f seconds\n", endTime - startTime);
    }

    // Clean up
    if (rank == 0)
    {
        free(data);
    }
    free(sub_data);

    MPI_Finalize(); // Finalize MPI
    return 0;
}

// a program that calculates the sum of an array of numbers in parallel. The idea is to divide the array into equal parts, distribute these parts among multiple processes, each process calculates the sum of its part, and finally, the partial sums are combined to get the total sum.

// Pseudocode
// Initialize MPI: Start MPI and get the total number of processes and the current process's rank.
// Prepare Data: The master process (rank 0) prepares an array of numbers.
// Distribute Data: Divide the array into equal parts and distribute each part to different processes.
// Local Sum Calculation: Each process calculates the sum of its received part.
// Gather Partial Sums and Calculate Total Sum: Collect the partial sums from all processes to the master process and calculate the total sum.
// Finalize MPI: Close the MPI environment.