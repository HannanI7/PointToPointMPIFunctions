//K214934 Hannan Irfan 6E

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void my_MPI_Allgatherv(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                       void *recvbuf, int *recvcounts, int *displs, MPI_Datatype recvtype,
                       MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    MPI_Request *requests = (MPI_Request *)malloc(sizeof(MPI_Request) * size * 2);
    MPI_Status *statuses = (MPI_Status *)malloc(sizeof(MPI_Status) * size * 2);

    int send_type_size;
    MPI_Type_size(sendtype, &send_type_size);

    int recv_type_size;
    MPI_Type_size(recvtype, &recv_type_size);

    // Start non-blocking sends
    for (int i = 0; i < size; i++) {
        MPI_Isend((char *)sendbuf + displs[rank] * sendcount * send_type_size,
                  sendcount, sendtype, i, 0, comm, &requests[i]);
    }

    // Start non-blocking receives
    for (int i = 0; i < size; i++) {
        MPI_Irecv((char *)recvbuf + displs[i] * recvcounts[rank] * recv_type_size,
                  recvcounts[i], recvtype, i, 0, comm, &requests[size + i]);
    }

    // Wait for all sends and receives to complete
    MPI_Waitall(2 * size, requests, statuses);

    free(requests);
    free(statuses);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *sendbuf_arr = (int *)malloc(sizeof(int) * size * 3);
    for (int i = 0; i < size; i++) {
        sendbuf_arr[i * 3] = rank;
        sendbuf_arr[i * 3 + 1] = rank * 2;
        sendbuf_arr[i * 3 + 2] = rank * 3;
    }

    int *recvbuf_all = (int *)malloc(sizeof(int) * size * 3);
    int *recvcounts = (int *)malloc(sizeof(int) * size);
    int *displs = (int *)malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        recvcounts[i] = 3;
        displs[i] = i * 3;
    }

    my_MPI_Allgatherv(sendbuf_arr, 3, MPI_INT, recvbuf_all, recvcounts, displs, MPI_INT, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Received all data:\n");
        for (int i = 0; i < size * 3; i++) {
            printf("%d ", recvbuf_all[i]);
        }
        printf("\n");
    }

    free(sendbuf_arr);
    free(recvbuf_all);
    free(recvcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}

