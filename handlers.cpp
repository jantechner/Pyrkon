#include "main.h"

extern int permissionsReceived;
int workshopsTicketsNumberReceived;

void startPyrkonRequestHandler(packet_t *);
void startPyrkonPermissionHandler(packet_t *);
void startPyrkonHandler(packet_t *);
void pyrkonTicketsHandler(packet_t *);
void workshopsTicketsHandler(packet_t *);
void finishHandler(packet_t *);

functionPointer handlers[MAX_HANDLERS];

void initializeHandlers() {
    handlers[WANT_START_PYRKON] = startPyrkonRequestHandler;
    handlers[WANT_START_PYRKON_ACK] = startPyrkonPermissionHandler;
    handlers[PYRKON_START] = startPyrkonHandler;
    handlers[PYRKON_TICKETS] = pyrkonTicketsHandler;
    handlers[WORKSHOPS_TICKETS] = workshopsTicketsHandler;
    handlers[FINISH] = finishHandler;
}

void startPyrkonRequestHandler(packet_t *pakiet) {
    int requestFrom = pakiet->src;
    println("       Timery: %d %d", requestTimestamp, pakiet->requestTimestamp);
    if (processID != pyrkonHost) {
        if (requestTimestamp > pakiet->requestTimestamp || (requestTimestamp == pakiet->requestTimestamp && processID > requestFrom)) {
            pakiet->src = processID;
            sendPacket(pakiet, requestFrom, WANT_START_PYRKON_ACK);
        }
    }
}

void startPyrkonPermissionHandler(packet_t *pakiet) {
    permissionsReceived++;
    println("               PERMISSION od %d || Otrzymano już %d", pakiet->src, permissionsReceived);
    if (permissionsReceived == size - 1) {
        println("I AM PYRKON HOST\n");
        pyrkonHost = processID;
        sem_post(&pyrkonStartSem);
    }
}

void startPyrkonHandler(packet_t *pakiet) {
    pyrkonNumber++;
    pyrkonHost = pakiet->src;
    sem_post(&pyrkonStartSem);
}

void pyrkonTicketsHandler(packet_t *pakiet) {
    pyrkonTicketsNumber = pakiet->ticketsNumber;
    println("Pyrkon tickets: %d", pyrkonTicketsNumber);
}

void workshopsTicketsHandler(packet_t *pakiet) {
    if (pakiet->workshopNumber == -1) {
        workshopsNumber = pakiet->ticketsNumber;
        delete [] workshopsTickets;                         /* delete previous tickets */
        workshopsTickets = new int[workshopsNumber];    /* allocate memory for new tickets */
        workshopsTicketsNumberReceived = 0;
        println("Workshops number : %d", pakiet->ticketsNumber);
    } else {
        workshopsTickets[pakiet->workshopNumber] = pakiet->ticketsNumber;
        workshopsTicketsNumberReceived++;
        println("Workshop %d tickets number: %d", pakiet->workshopNumber, pakiet->ticketsNumber);
        if (workshopsTicketsNumberReceived == workshopsNumber) {
            sem_post(&ticketsDetailsSem);
        }
    }
}

void finishHandler(packet_t *pakiet) {
    programEnd = true;
}