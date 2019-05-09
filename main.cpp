#include "main.h"

int currentWorkshop;

extern void initialize(int argc, char **argv);
extern void finalize();
extern void freePyrkonTicket();
extern void freeWorkshopTicket(int);
void mainLoop();
void choosePyrkonHost();
void safelyIncrementPyrkonNumber();
void getTicketsDetails();
void getPyrkonTicket();
void attendWorkshops();
void notifyAll(int, int, int);
void notifyOthers(int, int, int);
void setrequestTS(packet_t *, int, int);
void waitAWhile();
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
        safelyIncrementPyrkonNumber();
        getTicketsDetails();
        getPyrkonTicket();
        attendWorkshops();
        freePyrkonTicket();
    }
}

void choosePyrkonHost() {
    notifyOthers(WANT_TO_BE_HOST, 0, 0);
    waitFor(&pyrkonHostSem);
}

void safelyIncrementPyrkonNumber() {
    
    if (isHost) {                   // wyślij wszystkim, żeby sobie zmienili numer Pyrkonu, a następnie zmień sobie
        notifyOthers(PYRKON_START, 0, 0);
        pyrkonNumber++;
    } else waitFor(&pyrkonStartSem);  

    println("Pyrkon %d", pyrkonNumber);

    // tutaj poczekaj aż wszyscy sobie zmienią numer, żeby żaden proces nie poleciał do przodu (w szczególności host) i nie zaczął wysyłać wiadomości o biletach
    if(isHost) notifyAll(PYRKON_NUMBER_INCREMENTED, 0, 0);
    waitFor(&pyrkonNumberIncrementedSem);
}

void getTicketsDetails() {
    if (isHost) {
        isHost = false;
        pthread_create(&ticketsThread, NULL, prepareAndSendTicketsDetails, 0);  //uruchomienie wątku, który wylosuje i roześle wszystkim informacje o biletach
    }
    waitFor(&everyoneGetsTicketsInfoSem);   //czekanie, aż wszyscy otrzymają informację o biletach - inaczej mogą wystąpić Segmentation Fault ze względu na brak struktur tworzonych w momencie odebrania wiadomości
}

void getPyrkonTicket() {
    pyrkonTicket.want = true;
    notifyOthers(WANT_PYRKON_TICKET, 0, 0);
    waitFor(&pyrkonTicketSem);
    println("ENTER PYRKON");
}

void attendWorkshops() {
    deque<int> notAttendedWorkshops;
    for (int i = 0; i < workshopsNumber; i++) notAttendedWorkshops.push_back(i);

    int numberOfWorkshopsIWantAttend = rand() % (workshopsNumber - 1) + 1;  //od 1 do prawie wszystkich
    println("Idę na %d warsztatów", numberOfWorkshopsIWantAttend);

    while (numberOfWorkshopsIWantAttend > 0) {
        int index = rand() % notAttendedWorkshops.size();
        currentWorkshop = notAttendedWorkshops[index];

        workshopsTickets[currentWorkshop].want = true;
        notifyOthers(WANT_WORKSHOP_TICKET, currentWorkshop, 0);
        waitFor(&workshopTicketSem);
        println("JESTEM NA WARSZTACIE %d", currentWorkshop);
        waitAWhile();
        freeWorkshopTicket(currentWorkshop);
        
        notAttendedWorkshops.erase(notAttendedWorkshops.begin() + index);
        numberOfWorkshopsIWantAttend--;
    }

    println("EXIT PYRKON");
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
        if (dst != processId) {
            sendPacket(&pakiet, dst, message);
        }
    }
}

void sendPacket(packet_t *data, int dst, int type) {

    pthread_mutex_lock(&timerMutex);
        data->ts = ++lamportTimer;
        setrequestTS(data, lamportTimer, type);                     /* jeżeli konieczne, ustaw requestTimer na aktualny lamportTimer - Ricart Agrawala Aglorithm */
        // println("%s -> %d", getMessageCode(type).c_str(), dst);     /* printowanie w mutexie, żeby zapewnić idealną informację o punktach w czasie */
    pthread_mutex_unlock(&timerMutex);

    data->pyrkonNumber = pyrkonNumber;

    MPI_Send(data, 1, MPI_PACKET_T, dst, type, MPI_COMM_WORLD);
}

string getMessageCode(int n) {
    string code;
    switch (n) {
        case WANT_TO_BE_HOST: code = "WANT_TO_BE_HOST"; break;
        case PYRKON_START: code = "PYRKON_START"; break;
        case PYRKON_NUMBER_INCREMENTED: code = "PYRKON_NUMBER_INCREMENTED"; break;
        case PYRKON_TICKETS: code = "PYRKON_TICKETS"; break;
        case WORKSHOPS_TICKETS: code = "WORKSHOPS_TICKETS"; break;
        case GOT_TICKETS_INFO: code = "GOT_TICKETS_INFO"; break;
        case WANT_PYRKON_TICKET: code = "WANT_PYRKON_TICKET"; break;
        case WANT_PYRKON_TICKET_ACK: code = "WANT_PYRKON_TICKET_ACK"; break;
        case WANT_WORKSHOP_TICKET: code = "WANT_WORKSHOP_TICKET"; break;
        case WANT_WORKSHOP_TICKET_ACK: code = "WANT_WORKSHOP_TICKET_ACK"; break;
        default: code = "UNKNOWN";
    }
    return code;
}

void setrequestTS(packet_t *data, int timer, int type) {
    if (type == WANT_TO_BE_HOST) {
        if (myHostRequest.TS == INT_MAX) myHostRequest.TS = timer;
        data->requestTS = myHostRequest.TS;
    } else if (type == WANT_PYRKON_TICKET) {
        if (pyrkonTicket.requestTS == INT_MAX) pyrkonTicket.requestTS = timer;
        data->requestTS = pyrkonTicket.requestTS;
    } else if (type == WANT_WORKSHOP_TICKET) {
        if (workshopsTickets[currentWorkshop].requestTS == INT_MAX) workshopsTickets[currentWorkshop].requestTS = timer;
        data->requestTS = workshopsTickets[currentWorkshop].requestTS;
    }
}

void waitFor(sem_t *semaphore) {
    sem_wait(semaphore);
}

void waitAWhile() {
    unsigned int microseconds = rand() % 1500000 + 500000;
    usleep(microseconds);
}