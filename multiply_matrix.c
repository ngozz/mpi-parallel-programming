#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Function to multiply matrices, modified to take N as a parameter
void multiplyMatrices(int *A, int *B, int *C, int rows, int N)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < N; j++)
        {
            C[i * N + j] = 0;
            for (int k = 0; k < N; k++)
            {
                C[i * N + j] += A[i * N + k] * B[k * N + j];
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int rank, size;
    double startTime, endTime;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Print the number of processes
    if (rank == 0)
    {
        printf("Number of processes: %d\n", size);
    }

    // Start timing
    startTime = MPI_Wtime();

    // Check for correct number of arguments and parse N
    if (argc != 2)
    {
        if (rank == 0)
        {
            fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int N = atoi(argv[1]);
    if (N % size != 0)
    {
        if (rank == 0)
        {
            fprintf(stderr, "N must be divisible by the number of processes.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int *A = NULL, *B = NULL, *C = NULL, *subA = NULL, *subC = NULL;
    int rows = N / size; // Rows of A to send to each process

    // Master process initializes data and distributes it
    if (rank == 0)
    {
        A = (int *)malloc(N * N * sizeof(int));
        B = (int *)malloc(N * N * sizeof(int));
        C = (int *)malloc(N * N * sizeof(int));
        // Initialize matrices A and B with some values
        for (int i = 0; i < N * N; i++)
        {
            A[i] = i + 1;
            B[i] = (i + 1) * 2;
        }
    }

    // Allocate memory for sub-matrices and matrix B in all processes
    subA = (int *)malloc(rows * N * sizeof(int));
    subC = (int *)malloc(rows * N * sizeof(int));
    if (rank != 0)
    {
        B = (int *)malloc(N * N * sizeof(int));
    }

    // Distribute parts of matrix A and the whole of matrix B to all processes
    MPI_Scatter(A, rows * N, MPI_INT, subA, rows * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform local multiplication
    multiplyMatrices(subA, B, subC, rows, N);

    // Gather the computed parts of matrix C from all processes
    MPI_Gather(subC, rows * N, MPI_INT, C, rows * N, MPI_INT, 0, MPI_COMM_WORLD);

    // End timing
    endTime = MPI_Wtime();

    // Master process prints the result
    if (rank == 0)
    {
        printf("Result matrix C:\n");
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                printf("%d ", C[i * N + j]);
            }
            printf("\n");
        }
        printf("Execution time: %f seconds\n", endTime - startTime);
        free(A);
        free(B);
        free(C);
    }
    else
    {
        free(B);
    }
    free(subA);
    free(subC);

    MPI_Finalize();
    return 0;
}

// a parallel matrix multiplication program using MPI. This example involves more complex data distribution and gathering. The program multiplies two matrices in parallel. Each process computes a part of the result matrix.

// Pseudocode
// Initialize MPI: Start MPI and get the total number of processes and the current process's rank.
// Prepare Data:
// The master process (rank 0) generates two matrices, A and B.
// Distribute Data:
// Divide matrix A into equal parts by rows and send each part to different processes.
// Send the whole of matrix B to all processes.
// Local Multiplication:
// Each process multiplies its part of matrix A with matrix B to compute a part of the result matrix C.
// Gather Results:
// Gather the computed parts of matrix C from all processes to the master process.
// Finalize MPI: Close the MPI environment.