#include "main.h"

pthread_t communicationThread, monitorThread;

/* mutexes synchronizing shared variables */
pthread_mutex_t konto_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timerMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t all_sem;

int pyrkonHost;
int lamportTimer;
int permissionsReceived = 0;
volatile bool end = false;
volatile bool pyrkonInProgress = false;

extern void inicjuj(int argc, char **argv);
void mainLoop(void);
extern void finalizuj(void);
void notifyAll(int);
void notifyOthers(int);
void choosePyrkonHost();
// void determineWorkshopsDetails(void);

int main(int argc, char **argv) {
    printf("%d\n", argc);
    inicjuj(argc, argv);
    mainLoop();
    finalizuj();
    return 0;
}

void mainLoop(void) {

    choosePyrkonHost();

    if (rank == pyrkonHost) notifyAll(PYRKON_START);

    // if (rank == pyrkonHost) determineWorkshopsDetails();

    // wait a while
    int percent = rand() % 2 + 1;
    struct timespec t = {percent, 0};
    struct timespec rem = {1, 0};
    nanosleep(&t, &rem);

    if (rank == 0) notifyAll(FINISH);
}

void choosePyrkonHost() {
    permissionsReceived = 0;
    pyrkonHost = -1;

    notifyOthers(WANT_START_PYRKON);

    
    sem_init(&all_sem, 0, 0);
    printf("TEST\n");
    sem_wait(&all_sem);
    printf("TEST2");

    // while(!pyrkonInProgress) {  //TODO może lepsze zastosowanie semafora -> pasywne czekanie 
    //     if (permissionsReceived == size - 1) {
    //         pyrkonHost = rank;
    //         break;
    //     }
    // }
}

void notifyAll(int message) {
    packet_t pakiet;
    for (int dst = 0; dst < size; dst++)
        sendPacket(&pakiet, dst, message);
}

void notifyOthers(int message) {
    packet_t pakiet;
    for (int dst = 0; dst < size; dst++) {
        if (dst != rank ) {
            sendPacket(&pakiet, dst, message);
            println("Start pyrkon %d -> %d", rank, dst);
        }
    }
}

void sendPacket(packet_t *data, int dst, int type) {
    if (/* type != WANT_START_PYRKON_ACK && */ type != WANT_START_PYRKON) {
        pthread_mutex_lock(&timerMutex);
        lamportTimer++;
        pthread_mutex_unlock(&timerMutex);
    }
    
    pthread_mutex_lock(&timerMutex);
    data->ts = lamportTimer;
    pthread_mutex_unlock(&timerMutex);

    MPI_Send(data, 1, MPI_PAKIET_T, dst, type, MPI_COMM_WORLD);
    
  /*   packet_t *newP = (packet_t *)malloc(sizeof(packet_t));
    stackEl_t *stackEl = (stackEl_t *)malloc(sizeof(stackEl_t));
    memcpy(newP,data, sizeof(packet_t));
    stackEl->dst = dst;
    stackEl->type = type;
    stackEl->newP = newP;
    pthread_mutex_lock( &packetMut );
    g_queue_push_head( delayStack, stackEl );
    pthread_mutex_unlock( &packetMut ); */
    
}