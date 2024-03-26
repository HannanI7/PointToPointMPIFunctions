//K214934 Hannan Irfan 6E

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void my_MPI_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                      void *recvbuf, int recvcount, MPI_Datatype recvtype,
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
        MPI_Isend(sendbuf, sendcount, sendtype, i, 0, comm, &requests[i]);
    }

    // Start non-blocking receives
    for (int i = 0; i < size; i++) {
        MPI_Irecv((char *)recvbuf + i * recvcount * recv_type_size,
                  recvcount, recvtype, i, 0, comm, &requests[size + i]);
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

    int sendbuf = rank;
    int recvbuf[size];

    my_MPI_Allgather(&sendbuf, 1, MPI_INT, recvbuf, 1, MPI_INT, MPI_COMM_WORLD);

    printf("Rank %d received data: ", rank);
    for (int i = 0; i < size; i++) {
        printf("%d ", recvbuf[i]);
    }
    printf("\n");

    MPI_Finalize();
    return 0;
}

