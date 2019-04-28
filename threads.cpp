#include "main.h"

extern string getMessageCode(int);
extern functionPointer handlers[];
extern void notifyAll(int, int, int);

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

void * prepareAndSendTicketsDetails(void *ptr) {

    int tickets = rand() % (size - 1) + 1;      //od 1 do size-1 biletów
    println("                   Ilość biletów: %d", tickets);
    notifyAll(PYRKON_TICKETS, 0, tickets);

    int workshops = rand() % (MAX_WORKSHOPS - MIN_WORKSHOPS + 1) + MIN_WORKSHOPS;   //od MIN do MAX warsztatów
    println("                   Ilość warsztatów: %d", workshops);
    notifyAll(WORKSHOPS_TICKETS, -1, workshops);

    for (int i = 0; i<workshops; i++) {
        int wTickets = rand() % size + 1;        // od 1 do wszystkich uczestników
        println("                   Ilość miejsc na %d warsztacie: %d", i, wTickets);
        notifyAll(WORKSHOPS_TICKETS, i, wTickets);
    }

    println("TICKETS DETAILS SENT");
    return 0;
}