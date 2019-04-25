#include "main.h"

void *monitorFunc(void *ptr) {
    packet_t data;

    for (int i = 5; i > 0; i--) {
        println("Start monitora za %d s\n", i);
        sleep(1);
    }

    // // TUTAJ WYKRYWANIE STANu
    // sem_init(&all_sem, 0, 0);
    // println("MONITOR START \n");
    // for (int i = 0; i < size; i++) {
    //     sendPacket(&data, i, GIVE_YOUR_STATE);
    // }
    // sem_wait(&all_sem);

    for (int i = 1; i < size; i++) {
        sendPacket(&data, i, FINISH);
    }
    sendPacket(&data, 0, FINISH);

    // P_RED;
    // printf("\n\tW systemie jest: [%d]\n\n", sum);
    // P_CLR
    return 0;
}

/* Wątek komunikacyjny - dla każdej otrzymanej wiadomości wywołuje jej handler */
void *comFunc(void *ptr) {
    println("Wejście do wątku komunikacyjnego\n");
    MPI_Status status;
    packet_t pakiet;

    while (!end) {
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&timerMutex);
        lamportTimer = max(lamportTimer, pakiet.ts) + 1;
        pthread_mutex_unlock(&timerMutex);

        pakiet.src = status.MPI_SOURCE;
        handlers[(int)status.MPI_TAG](&pakiet);
    }
    println("Koniec!");
    return 0;
}