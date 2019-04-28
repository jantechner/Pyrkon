#include "main.h"

int permissionsReceived;

extern void initialize(int argc, char **argv);
extern void finalize();
void mainLoop();
void choosePyrkonHost();
void notifyAll(int, int, int);
void notifyOthers(int, int, int);
bool requestTimestampIsNeeded(int);
void waitFor(sem_t *);
string getMessageCode(int);

int main(int argc, char *argv[]) {
    initialize(argc, argv);
    mainLoop();
    finalize();
    return 0;
}

void mainLoop(void) {
    choosePyrkonHost();
    if (processID == pyrkonHost) {
        notifyOthers(PYRKON_START, 0, 0);
        pyrkonNumber++;
        pthread_create(&ticketsThread, NULL, prepareAndSendTicketsDetails, 0);
    }
    waitFor(&ticketsDetailsSem);
    println("TICKETS DETAILS RECEIVED");

    // wait a while
    // int percent = rand() % 2 + 1;
    // struct timespec t = {percent, 0};
    // struct timespec rem = {1, 0};
    // nanosleep(&t, &rem);

    if (processID == 0) notifyAll(FINISH,0,0);
}

void choosePyrkonHost() {
    permissionsReceived = 0;
    pyrkonHost = -1;

    notifyOthers(WANT_START_PYRKON, 0, 0);
    waitFor(&pyrkonStartSem);

    requestTimestamp = INT_MAX;
}

void notifyAll(int message, int workshopNumber, int ticketsNumber) {
    packet_t pakiet;
    pakiet.workshopNumber = workshopNumber;
    pakiet.ticketsNumber = ticketsNumber;
    for (int dst = 0; dst < size; dst++)
        sendPacket(&pakiet, dst, message);
}

void notifyOthers(int message, int workshopNumber, int ticketsNumber) {
    packet_t pakiet;
    pakiet.workshopNumber = workshopNumber;
    pakiet.ticketsNumber = ticketsNumber;
    for (int dst = 0; dst < size; dst++) {
        if (dst != processID ) {
            sendPacket(&pakiet, dst, message);
        }
    }
}

void sendPacket(packet_t *data, int dst, int type) {

    pthread_mutex_lock(&timerMutex);
    data->ts = ++lamportTimer;
    if (requestTimestampIsNeeded(type)) requestTimestamp = lamportTimer;
    println("%s -> %d", getMessageCode(type).c_str(), dst);         /* printowanie w mutexie, żeby zapewnić idealną informację o punktach w czasie */
    pthread_mutex_unlock(&timerMutex);

    data->requestTimestamp = requestTimestamp;
    data->pyrkonNumber = pyrkonNumber;

    MPI_Send(data, 1, MPI_PACKET_T, dst, type, MPI_COMM_WORLD);
    
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

string getMessageCode(int n) {
    string code;
    switch(n) {
        case PYRKON_START: code = "PYRKON_START"; break;
        case FINISH: code = "FINISH"; break;
        case WANT_START_PYRKON: code = "WANT_START_PYRKON"; break;
        case WANT_START_PYRKON_ACK: code = "WANT_START_PYRKON_ACK"; break;
        case PYRKON_TICKETS: code = "PYRKON_TICKETS"; break;
        case WORKSHOPS_TICKETS: code = "WORKSHOPS_TICKETS"; break;
        default: code = "UNKNOWN";
    }
    return code;
}

bool requestTimestampIsNeeded(int type) {
    return requestTimestamp == INT_MAX && (int)getMessageCode(type).find("WANT") != -1 && (int)getMessageCode(type).find("ACK") == -1 ? true : false;
}

void waitFor(sem_t *semaphore) {
    sem_init(semaphore, 0, 0);
    sem_wait(semaphore);
}