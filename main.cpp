#include "main.h"

int permissionsReceived, pyrkonTicketPermissionsReceived, leavedPyrkon = 0;
bool hasPyrkonTicket;

extern void initialize(int argc, char **argv);
extern void finalize();
extern void freePyrkonTicket();
void mainLoop();
void choosePyrkonHost();
void getPyrkonTicket();
void notifyAll(int, int, int);
void notifyOthers(int, int, int);
void setrequestTS(packet_t*, int, int);
void waitFor(sem_t *);
string getMessageCode(int);

int main(int argc, char *argv[]) {
    initialize(argc, argv);
    mainLoop();
    finalize();
    return 0;
}

void mainLoop(void) {

    // for(int i=0; i<3; i++) {
        // sleep(3);   //te 3 sekundy chronią przed wyścigiem jednego procesu
        println("                                                       NEW PYRKON");
        leavedPyrkon = 0;
        
        choosePyrkonHost();      
        if (processId == pyrkonHost) {    //ostatni, który wysyła informację, że wyszedł mógłby być nowym hostem 
            notifyOthers(PYRKON_START, 0, 0);
            pyrkonNumber++;
            pthread_create(&ticketsThread, NULL, prepareAndSendTicketsDetails, 0);
        }
        waitFor(&ticketsDetailsSem);
        
        getPyrkonTicket();
            println("ENTER PYRKON");

            // wait a while
            int time = rand() % 4 + 2;
            sleep(time);    

            println("EXIT PYRKON");
        freePyrkonTicket();

        pyrkonHost = -1;
        notifyAll(LEAVE_PYRKON, 0, 0);

        waitFor(&allLeavedPyrkon);
       
    // }

    // if (processId == 0) notifyAll(FINISH,0,0);
}

void choosePyrkonHost() {
    permissionsReceived = 0;
    

    notifyOthers(WANT_START_PYRKON, 0, 0);
    waitFor(&pyrkonStartSem);
}

void getPyrkonTicket() {
    println("                                            WANT TICKET");
    // pyrkonTicketPermissionsReceived = 0;
    // hasPyrkonTicket = false;
    // wantPyrkonTicket = true;

    pyrkonTicket.want = true;

    notifyOthers(WANT_PYRKON_TICKET, 0, 0);
    waitFor(&pyrkonTicketSem);
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
        if (dst != processId ) {
            sendPacket(&pakiet, dst, message);
        }
    }
}

void sendPacket(packet_t *data, int dst, int type) {

    pthread_mutex_lock(&timerMutex);
    data->ts = ++lamportTimer;
    setrequestTS(data, lamportTimer, type);                  /* jeżeli konieczne, ustaw requestTimer na aktualny lamportTimer - Ricart Agrawala Aglorithm */
    println("%s -> %d", getMessageCode(type).c_str(), dst);         /* printowanie w mutexie, żeby zapewnić idealną informację o punktach w czasie */
    pthread_mutex_unlock(&timerMutex);

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
        case WANT_PYRKON_TICKET: code = "WANT_PYRKON_TICKET"; break;
        case WANT_PYRKON_TICKET_ACK: code = "WANT_PYRKON_TICKET_ACK"; break;
        case LEAVE_PYRKON: code = "LEAVE_PYRKON"; break;
        default: code = "UNKNOWN";
    }
    return code;
}

void setrequestTS(packet_t * data, int timer, int type) {

    if (type == WANT_START_PYRKON) {
        if (requestTS == INT_MAX) {
            requestTS = timer;
        }
        data->requestTS = requestTS;
    } else if (type == WANT_PYRKON_TICKET) {
        if (pyrkonTicket.requestTS == INT_MAX) pyrkonTicket.requestTS = timer;
        data->requestTS = pyrkonTicket.requestTS;
    } else {
        data->requestTS = 0;
    }
}

void waitFor(sem_t *semaphore) {
    sem_wait(semaphore);
}