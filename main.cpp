#include "main.h"

int permissionsReceived;

extern void initialize(int argc, char **argv);
extern void finalize();
void mainLoop();
void notifyAll(int);
void notifyOthers(int);
void choosePyrkonHost();
// void determineWorkshopsDetails();

int main(int argc, char *argv[]) {
    initialize(argc, argv);
    mainLoop();
    finalize();
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

    sem_init(&pyrkonStartSem, 0, 0);
    sem_wait(&pyrkonStartSem);
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