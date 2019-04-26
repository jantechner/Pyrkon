#include "main.h"

int requestTimestamp;
extern functionPointer handlers[];

/* Wątek komunikacyjny - dla każdej otrzymanej wiadomości wywołuje jej handler */
void *comFunc(void *ptr) {
    // println("Wejście do wątku komunikacyjnego\n");
    MPI_Status status;
    packet_t pakiet;

    while (!end) {
        
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        if (status.MPI_TAG != WANT_START_PYRKON && status.MPI_TAG != WANT_START_PYRKON_ACK) {
            pthread_mutex_lock(&timerMutex);
            requestTimestamp = lamportTimer;
            lamportTimer = max(lamportTimer, pakiet.ts) + 1;
            pthread_mutex_unlock(&timerMutex);
        } else {
            pthread_mutex_lock(&timerMutex);
            requestTimestamp = lamportTimer;
            pthread_mutex_unlock(&timerMutex);
        }

        pakiet.src = status.MPI_SOURCE;
        handlers[(int)status.MPI_TAG](&pakiet);
    }
    return 0;
}