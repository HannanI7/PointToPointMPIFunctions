//K214934 Hannan Irfan 6E

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void my_MPI_Alltoallv(void *sendbuf, int *sendcounts, int *sdispls, MPI_Datatype sendtype,
                      void *recvbuf, int *recvcounts, int *rdispls, MPI_Datatype recvtype,
                      MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    MPI_Request *requests_send = (MPI_Request *)malloc(sizeof(MPI_Request) * size);
    MPI_Request *requests_recv = (MPI_Request *)malloc(sizeof(MPI_Request) * size);
    MPI_Status *statuses_send = (MPI_Status *)malloc(sizeof(MPI_Status) * size);
    MPI_Status *statuses_recv = (MPI_Status *)malloc(sizeof(MPI_Status) * size);

    int send_type_size;
    MPI_Type_size(sendtype, &send_type_size);

    int recv_type_size;
    MPI_Type_size(recvtype, &recv_type_size);

    // Start non-blocking sends
    for (int i = 0; i < size; i++) {
        MPI_Isend((char *)sendbuf + sdispls[i] * send_type_size,
                  sendcounts[i], sendtype, i, 0, comm, &requests_send[i]);
    }

    // Start non-blocking receives
    for (int i = 0; i < size; i++) {
        MPI_Irecv((char *)recvbuf + rdispls[i] * recv_type_size,
                  recvcounts[i], recvtype, i, 0, comm, &requests_recv[i]);
    }

    // Wait for all sends and receives to complete
    MPI_Waitall(size, requests_send, statuses_send);
    MPI_Waitall(size, requests_recv, statuses_recv);

    free(requests_send);
    free(requests_recv);
    free(statuses_send);
    free(statuses_recv);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *sendbuf = (int *)malloc(sizeof(int) * size * 3);
    int *recvbuf = (int *)malloc(sizeof(int) * size * 3);

    int sendcounts[size];
    int sdispls[size];
    int recvcounts[size];
    int rdispls[size];

    for (int i = 0; i < size; i++) {
        sendcounts[i] = 3;
        sdispls[i] = i * 3;
        recvcounts[i] = 3;
        rdispls[i] = i * 3;
    }

    for (int i = 0; i < size; i++) {
        sendbuf[i * 3] = rank;
        sendbuf[i * 3 + 1] = rank * 2;
        sendbuf[i * 3 + 2] = rank * 3;
    }

    my_MPI_Alltoallv(sendbuf, sendcounts, sdispls, MPI_INT,
                     recvbuf, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD);

    printf("Rank %d received data: ", rank);
    for (int i = 0; i < size * 3; i++) {
        printf("%d ", recvbuf[i]);
    }
    printf("\n");

    free(sendbuf);
    free(recvbuf);

    MPI_Finalize();
    return 0;
}

