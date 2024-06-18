#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void quickSort(int *arr, int left, int right) {
    int i = left, j = right;
    int tmp;
    int pivot = arr[(left + right) / 2];

    /* partition */
    while (i <= j) {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        if (i <= j) {
            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
        }
    };

    /* recursion */
    if (left < j)
        quickSort(arr, left, j);
    if (i < right)
        quickSort(arr, i, right);
}

void parallelQuickSort(int *global_arr, int num_elements, int rank, int size) {
    int *sub_arr = (int *)malloc(num_elements / size * sizeof(int));
    MPI_Scatter(global_arr, num_elements / size, MPI_INT, sub_arr, num_elements / size, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process sorts its partition
    quickSort(sub_arr, 0, num_elements / size - 1);

    // Gather the sorted sub-arrays into the global array
    MPI_Gather(sub_arr, num_elements / size, MPI_INT, global_arr, num_elements / size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Now, the master process needs to merge the sorted sub-arrays
        // For simplicity, this example will just sort the partially sorted global array again
        quickSort(global_arr, 0, num_elements - 1);
    }

    free(sub_arr);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Start timing after initializing MPI and getting rank and size
    double start_time = MPI_Wtime();

    int num_elements = 100000; // Size of the array to sort
    int *global_arr = NULL;

    if (rank == 0) {
        global_arr = (int *)malloc(num_elements * sizeof(int));
        // Initialize array with random numbers
        srand(time(NULL));
        for (int i = 0; i < num_elements; i++) {
            global_arr[i] = rand() % 1000000;
        }
        printf("Number of processes: %d\n", size);
    }

    parallelQuickSort(global_arr, num_elements, rank, size);

    if (rank == 0) {
        printf("Sorted array:\n");
        for (int i = 0; i < num_elements; i++) {
            printf("%d ", global_arr[i]);
        }
        printf("\n");
        free(global_arr);

        // Stop timing and print the execution time
        double end_time = MPI_Wtime();
        printf("Execution time: %f seconds\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}