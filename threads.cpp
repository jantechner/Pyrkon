#include "main.h"

extern string getMessageCode(int);
extern functionPointer handlers[];

/* Wątek komunikacyjny - dla każdej otrzymanej wiadomości wywołuje jej handler */
void *comFunc(void *ptr) {
    // println("Wejście do wątku komunikacyjnego\n");
    MPI_Status status;
    packet_t pakiet;

    while (!programEnd) {
        MPI_Recv(&pakiet, 1, MPI_PACKET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&timerMutex);
        lamportTimer = max(lamportTimer, pakiet.ts) + 1;
        println("%d -> %s", status.MPI_SOURCE, getMessageCode(status.MPI_TAG).c_str());
        pthread_mutex_unlock(&timerMutex);

        pakiet.src = status.MPI_SOURCE;

        if(pakiet.pyrkonNumber == pyrkonNumber) handlers[(int)status.MPI_TAG](&pakiet);
    }
    return 0;
}