#include "main.h"

pthread_t communicationThread, monitorThread;

/* zamek do synchronizacji zmiennych współdzielonych */
pthread_mutex_t konto_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timerMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t all_sem;

int lamportTimer = 0;
int konto = STARTING_MONEY;
int sum = 0;  //suma zbierana przez monitor
volatile bool end = false;
volatile bool pyrkonInProgress = false;

extern void inicjuj(int argc, char **argv);
void mainLoop(void);
extern void finalizuj(void);
int chooseDestination(void);

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
            dst = chooseDestination();
            
            percent = rand() % konto;
            pakiet.kasa = percent;

            pthread_mutex_lock(&konto_mut);
            konto -= percent;
            pthread_mutex_unlock(&konto_mut);

            sendPacket(&pakiet, dst, APP_MSG);
            /* z biegiem czasu coraz rzadziej wysyłamy (przyda się do wykrywania zakończenia) */
            if (prob_of_sending > PROB_SENDING_LOWER_LIMIT) {
                prob_of_sending -= PROB_OF_SENDING_DECREASE;
                // printf("[%d] zmniejszono prawdopodobieństwo do %d", rank, prob_of_sending);
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

int chooseDestination() {
    int dst;
    do { dst = rand() % (size); } while (dst == rank);
    return dst;
}

void sendPacket(packet_t *data, int dst, int type)
{

    pthread_mutex_lock(&timerMutex);
        data->ts = ++lamportTimer;
    pthread_mutex_unlock(&timerMutex);

    MPI_Send(data, 1, MPI_PAKIET_T, dst, type, MPI_COMM_WORLD);
    /*
    packet_t *newP = (packet_t *)malloc(sizeof(packet_t));
    stackEl_t *stackEl = (stackEl_t *)malloc(sizeof(stackEl_t));
    memcpy(newP,data, sizeof(packet_t));
    stackEl->dst = dst;
    stackEl->type = type;
    stackEl->newP = newP;
    pthread_mutex_lock( &packetMut );
    g_queue_push_head( delayStack, stackEl );
    pthread_mutex_unlock( &packetMut );
    */
}