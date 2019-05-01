#include "main.h"

int processesThatLeftPyrkon = 0;

extern void initialize(int argc, char **argv);
extern void finalize();
extern void freePyrkonTicket();
void mainLoop();
void choosePyrkonHost();
void getPyrkonTicket();
void safelyIncrementPyrkonNumber();
void getTicketsDetails();
void leavePyrkonAndWaitForOthers();
void notifyAll(int, int, int);
void notifyOthers(int, int, int);
void setrequestTS(packet_t *, int, int);

void waitFor(sem_t *);
string getMessageCode(int);

int main(int argc, char *argv[]) {
    initialize(argc, argv);
    mainLoop();
    finalize();
    return 0;
}

void mainLoop(void) {

    for (int i = 0; i < 3; i++) {
        
        choosePyrkonHost();
        safelyIncrementPyrkonNumber();  //wbrew pozorom nie jest to takie trywialne, bo wiadomości o innym numerze Pyrkonu są odrzucane !!!
        getTicketsDetails();
        getPyrkonTicket();
            println("ENTER PYRKON");

        // int time = rand() % 4 + 2;
        println("ENTER PYRKON for %d seconds", 1);
        // sleep(1);
        println("EXIT PYRKON");

            println("EXIT PYRKON");
        freePyrkonTicket();

        // leavePyrkonAndWaitForOthers();       //nieużywane 
    }
    if (processId == 0) notifyAll(FINISH,0,0);
}

void choosePyrkonHost() {
    pyrkonHost.want = true;
    notifyOthers(WANT_TO_BE_HOST, 0, 0);
    waitFor(&pyrkonHostSem);
}

void safelyIncrementPyrkonNumber() {
    // wyślij wszystkim, żeby sobie zmienili numer Pyrkonu, a następnie zmień sobie
    if (pyrkonHost.has) {
        notifyOthers(PYRKON_START, 0, 0);
        pyrkonNumber++;
    } else waitFor(&pyrkonStartSem);

    pyrkonHost.requestTS = INT_MAX;     //dopiero tutaj można zmienić tą wartość, bo inaczej host nie otrzyma potwierdzeń
    pyrkonHost.want = false;            //podobnie z tym         

    println("Pyrkon %d", pyrkonNumber);

    // tutaj poczekaj aż wszyscy sobie zmienią numer, żeby żaden proces nie poleciał do przodu (w szczególności host) i nie zaczął wysyłać wiadomości o biletach
    if(pyrkonHost.has) notifyAll(PYRKON_NUMBER_INCREMENTED, 0, 0);
    waitFor(&pyrkonNumberIncrementedSem);
}

void getTicketsDetails() {
    if (pyrkonHost.has) {
        pyrkonHost.has = false;
        pthread_create(&ticketsThread, NULL, prepareAndSendTicketsDetails, 0);  //uruchomienie wątku, który wylosuje i roześle wszystkim informacje o biletach
    }
    waitFor(&ticketsDetailsSem);    //wszyscy się tutaj zatrzymują i czekają na info o biletach
}

void getPyrkonTicket() {
    pyrkonTicket.want = true;
    notifyOthers(WANT_PYRKON_TICKET, 0, 0);
    waitFor(&pyrkonTicketSem);
}

/* void leavePyrkonAndWaitForOthers() {
    notifyAll(LEAVING_PYRKON, 0, 0);
    waitFor(&allLeftPyrkon);
} */

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
        if (dst != processId) {
            sendPacket(&pakiet, dst, message);
        }
    }
}

void sendPacket(packet_t *data, int dst, int type) {
    pthread_mutex_lock(&timerMutex);
    data->ts = ++lamportTimer;
    setrequestTS(data, lamportTimer, type);                     /* jeżeli konieczne, ustaw requestTimer na aktualny lamportTimer - Ricart Agrawala Aglorithm */
    println("%s -> %d", getMessageCode(type).c_str(), dst);     /* printowanie w mutexie, żeby zapewnić idealną informację o punktach w czasie */
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
    switch (n) {
        case PYRKON_START: code = "PYRKON_START"; break;
        case FINISH: code = "FINISH"; break;
        case WANT_TO_BE_HOST: code = "WANT_TO_BE_HOST"; break;
        case WANT_TO_BE_HOST_ACK: code = "WANT_TO_BE_HOST_ACK"; break;
        case HOST_CHOSEN: code = "HOST_CHOSEN"; break;
        case PYRKON_TICKETS: code = "PYRKON_TICKETS"; break;
        case WORKSHOPS_TICKETS: code = "WORKSHOPS_TICKETS"; break;
        case WANT_PYRKON_TICKET: code = "WANT_PYRKON_TICKET"; break;
        case WANT_PYRKON_TICKET_ACK: code = "WANT_PYRKON_TICKET_ACK"; break;
        case LEAVING_PYRKON: code = "LEAVING_PYRKON"; break;
        case PYRKON_NUMBER_INCREMENTED: code = "PYRKON_NUMBER_INCREMENTED"; break;
        default: code = "UNKNOWN";
    }
    return code;
}

void setrequestTS(packet_t *data, int timer, int type) {
    if (type == WANT_TO_BE_HOST) {
        if (pyrkonHost.requestTS == INT_MAX) pyrkonHost.requestTS = timer;
        data->requestTS = pyrkonHost.requestTS;
    } else if (type == WANT_PYRKON_TICKET) {
        if (pyrkonTicket.requestTS == INT_MAX) pyrkonTicket.requestTS = timer;
        data->requestTS = pyrkonTicket.requestTS;
    }
}

void waitFor(sem_t *semaphore) {
    sem_wait(semaphore);
}