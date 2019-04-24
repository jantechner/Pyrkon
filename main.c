#include "main.h"

pthread_t communicationThread, monitorThread;

/* zamek do synchronizacji zmiennych współdzielonych */
pthread_mutex_t konto_mut = PTHREAD_MUTEX_INITIALIZER;
sem_t all_sem;

int konto = STARTING_MONEY;
int sum = 0;  //suma zbierana przez monitor
volatile char end = FALSE;

extern void inicjuj(int argc, char **argv);
void mainLoop(void);
extern void finalizuj(void);

int main(int argc, char **argv) {
    inicjuj(argc, argv);
    mainLoop();
    finalizuj();
    return 0;
}

/* Wątek główny - przesyłający innym pieniądze */
void mainLoop(void) {
    int prob_of_sending = PROB_OF_SENDING;
    int dst, percent;
    packet_t pakiet;

    while (!end) {
        percent = rand() % 100;
        if ((percent < prob_of_sending) && (konto > 0)) {
            do {
                dst = rand() % (size);
            } while (dst == rank);

            /* losuję, ile kasy komuś wysłać */
            percent = rand() % konto;
            pakiet.kasa = percent;

                pthread_mutex_lock(&konto_mut);
                konto -= percent;
                pthread_mutex_unlock(&konto_mut);

            sendPacket(&pakiet, dst, APP_MSG);
            /* z biegiem czasu coraz rzadziej wysyłamy (przyda się do wykrywania zakończenia) */
            if (prob_of_sending > PROB_SENDING_LOWER_LIMIT) {
                prob_of_sending -= PROB_OF_SENDING_DECREASE;
                printf("[%d] zmniejszono prawdopodobieństwo do %d", rank, prob_of_sending);
            }

            println("-> wysłałem %d do %d\n", pakiet.kasa, dst);

            //wait a while
            percent = rand() % 2 + 1;
            struct timespec t = {percent, 0};
            struct timespec rem = {1, 0};
            nanosleep(&t, &rem);
        }
    }
}