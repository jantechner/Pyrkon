#include "main.h"

extern int permissionsReceived, pyrkonTicketPermissionsReceived, leavedPyrkon;
extern bool hasPyrkonTicket;
int workshopsTicketsNumberReceived;
deque<int> waitingForPyrkonTicket;


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
    println("       Timery: %d %d ProcessID: %d  Host: %d", requestTimestamp, pakiet->requestTimestamp, processID, pyrkonHost);
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
        requestTimestamp = INT_MAX;
        sem_post(&pyrkonStartSem);
    }
}

void wantPyrkonTicketHandler(packet_t *pakiet) {
    int requestFrom = pakiet->src;
    println("       Timery: %d %d", pyrkonTicketRequestTS, pakiet->requestTimestamp);

    if (!pyrkonVisited) {
        if (!hasPyrkonTicket && (pyrkonTicketRequestTS > pakiet->requestTimestamp || (pyrkonTicketRequestTS == pakiet->requestTimestamp && processID > requestFrom))) {
            pakiet->src = processID;
            sendPacket(pakiet, requestFrom, WANT_PYRKON_TICKET_ACK);
        } else {
            waitingForPyrkonTicket.push_back(requestFrom);
        }
    } else {
        packet_t pakiet2;
        sendPacket(&pakiet2, requestFrom, WANT_PYRKON_TICKET_ACK);
    }
}

void freePyrkonTicket() {
    pyrkonVisited = true;
    while (!waitingForPyrkonTicket.empty()) {
        packet_t pakiet;
        sendPacket(&pakiet, waitingForPyrkonTicket.front(), WANT_PYRKON_TICKET_ACK);
        waitingForPyrkonTicket.pop_front();
    }
}

void wantPyrkonTicketAckHandler(packet_t *pakiet) {
    pyrkonTicketPermissionsReceived++;
    println("               TICKET PERMISSION od %d || Otrzymano już %d", pakiet->src, pyrkonTicketPermissionsReceived);
    if (pyrkonTicketPermissionsReceived == size - pyrkonTicketsNumber) {
        println("I HAVE PYRKON TICKET\n");
        hasPyrkonTicket = true;
        pyrkonTicketRequestTS = INT_MAX;
        sem_post(&pyrkonTicketSem);
    }
}

void startPyrkonHandler(packet_t *pakiet) {
    pyrkonNumber++;
    pyrkonHost = pakiet->src;  //teorytycznie niepotrzebne, bo tylko host musi wiedzieć, że jest hostem 
    requestTimestamp = INT_MAX;
    sem_post(&pyrkonStartSem);
}

void pyrkonTicketsHandler(packet_t *pakiet) {
    pyrkonTicketsNumber = pakiet->ticketsNumber;
    println("Pyrkon tickets: %d", pyrkonTicketsNumber);
    println("TICKETS DETAILS RECEIVED");
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