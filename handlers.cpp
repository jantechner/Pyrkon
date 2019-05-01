#include "main.h"

extern int permissionsReceived, leavedPyrkon;
int workshopsTicketsNumberReceived;


void startPyrkonRequestHandler(packet_t *);
void startPyrkonPermissionHandler(packet_t *);
void wantPyrkonTicketHandler(packet_t *);
void wantPyrkonTicketAckHandler(packet_t *);
void startPyrkonHandler(packet_t *);
void pyrkonTicketsHandler(packet_t *);
void workshopsTicketsHandler(packet_t *);
void leavePyrkonHandler(packet_t *);
void finishHandler(packet_t *);

functionPointer handlers[MAX_HANDLERS];

void initializeHandlers() {
    handlers[WANT_START_PYRKON] = startPyrkonRequestHandler;
    handlers[WANT_START_PYRKON_ACK] = startPyrkonPermissionHandler;
    handlers[WANT_PYRKON_TICKET] = wantPyrkonTicketHandler;
    handlers[WANT_PYRKON_TICKET_ACK] = wantPyrkonTicketAckHandler;
    handlers[PYRKON_START] = startPyrkonHandler;
    handlers[PYRKON_TICKETS] = pyrkonTicketsHandler;
    handlers[WORKSHOPS_TICKETS] = workshopsTicketsHandler;
    handlers[LEAVE_PYRKON] = leavePyrkonHandler;
    handlers[FINISH] = finishHandler;
}

void startPyrkonRequestHandler(packet_t *pakiet) {
    int requestFrom = pakiet->src;
    println("       Timery: %d %d processId: %d  Host: %d", requestTS, pakiet->requestTS, processId, pyrkonHost);
    if (processId != pyrkonHost) {
        if (requestTS > pakiet->requestTS || (requestTS == pakiet->requestTS && processId > requestFrom)) {
            pakiet->src = processId;
            sendPacket(pakiet, requestFrom, WANT_START_PYRKON_ACK);
        }
    }
}

void startPyrkonPermissionHandler(packet_t *pakiet) {
    permissionsReceived++;
    println("               PERMISSION od %d || Otrzymano już %d", pakiet->src, permissionsReceived);
    if (permissionsReceived == size - 1) {
        println("I AM PYRKON HOST\n");
        pyrkonHost = processId;
        requestTS = INT_MAX;
        sem_post(&pyrkonStartSem);
    }
}

void wantPyrkonTicketHandler(packet_t *pakiet) {
    int senderId = pakiet->src;
    println("       Timery: %d %d", pyrkonTicket.requestTS, pakiet->requestTS);
    if (pyrkonTicket.want) {
        if (!pyrkonTicket.has && (pyrkonTicket.requestTS > pakiet->requestTS || (pyrkonTicket.requestTS == pakiet->requestTS && processId > senderId))) {
            sendPacket(pakiet, senderId, WANT_PYRKON_TICKET_ACK);
        } else {
            pyrkonTicket.waiting.push_back(senderId);
        }
    } else {
        sendPacket(pakiet, senderId, WANT_PYRKON_TICKET_ACK);
    }
}

void wantPyrkonTicketAckHandler(packet_t *pakiet) {
    pyrkonTicket.permissions++;
    println("               TICKET PERMISSION od %d || Otrzymano już %d", pakiet->src, pyrkonTicket.permissions);
    if (pyrkonTicket.permissions == size - pyrkonTicket.number) {
        pyrkonTicket.has = true;
        pyrkonTicket.requestTS = INT_MAX;
        pyrkonTicket.permissions = 0;
        println("I HAVE PYRKON TICKET\n");
        sem_post(&pyrkonTicketSem);
    }
}

void freePyrkonTicket() {
    pyrkonTicket.want = false;
    pyrkonTicket.has = false;
    while (!pyrkonTicket.waiting.empty()) {
        packet_t pakiet;
        sendPacket(&pakiet, pyrkonTicket.waiting.front(), WANT_PYRKON_TICKET_ACK);
        pyrkonTicket.waiting.pop_front();
    }
}

void startPyrkonHandler(packet_t *pakiet) {
    pyrkonNumber++;
    pyrkonHost = pakiet->src;  //teorytycznie niepotrzebne, bo tylko host musi wiedzieć, że jest hostem 
    requestTS = INT_MAX;
    sem_post(&pyrkonStartSem);
}

void pyrkonTicketsHandler(packet_t *pakiet) {
    // pyrkonTicketsNumber = pakiet->ticketsNumber;
    pyrkonTicket.number = pakiet->ticketsNumber;
    sem_post(&ticketsDetailsSem);
}

void workshopsTicketsHandler(packet_t *pakiet) {
    if (pakiet->workshopNumber == -1) {
        workshopsNumber = pakiet->ticketsNumber;
        delete [] workshopsTickets;                         /* delete previous tickets */
        workshopsTickets = NULL;
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

void leavePyrkonHandler(packet_t * pakiet) {
    leavedPyrkon++;
    println("               WYSZŁO JUŻ %d", leavedPyrkon);
    if (leavedPyrkon == size) {
        sem_post(&allLeavedPyrkon);
    }
}

void finishHandler(packet_t *pakiet) {
    programEnd = true;
}