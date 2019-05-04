#include "main.h"

extern int processesThatLeftPyrkon, noHostsPermissions;
int workshopsTicketsNumberReceived;

void wantToBeHostHandler(packet_t *);
void wantToBeHostAckHandler(packet_t *);
void wantPyrkonTicketHandler(packet_t *);
void wantPyrkonTicketAckHandler(packet_t *);
void startPyrkonHandler(packet_t *);
void pyrkonTicketsHandler(packet_t *);
void workshopsTicketsHandler(packet_t *);
void leavingPyrkonHandler(packet_t *);
void hostChosenHandler(packet_t *);
void pyrkonNumberIncremented(packet_t *);
void finishHandler(packet_t *);

functionPointer handlers[MAX_HANDLERS];

void initializeHandlers() {
    handlers[WANT_TO_BE_HOST] = wantToBeHostHandler;
    handlers[WANT_TO_BE_HOST_ACK] = wantToBeHostAckHandler;
    handlers[WANT_PYRKON_TICKET] = wantPyrkonTicketHandler;
    handlers[WANT_PYRKON_TICKET_ACK] = wantPyrkonTicketAckHandler;
    handlers[PYRKON_START] = startPyrkonHandler;
    handlers[PYRKON_TICKETS] = pyrkonTicketsHandler;
    handlers[WORKSHOPS_TICKETS] = workshopsTicketsHandler;
    handlers[LEAVING_PYRKON] = leavingPyrkonHandler;
    handlers[FINISH] = finishHandler;
    handlers[HOST_CHOSEN] = hostChosenHandler;
    handlers[PYRKON_NUMBER_INCREMENTED] = pyrkonNumberIncremented;
}


void wantToBeHostHandler(packet_t *pakiet) {
    int senderId = pakiet->src;
    // println("       Timery: %d %d, ID: %d %d", pyrkonHost.requestTS, pakiet->requestTS, processId, senderId);
    if (pyrkonHost.want) {
        if (!pyrkonHost.has && (pyrkonHost.requestTS > pakiet->requestTS || (pyrkonHost.requestTS == pakiet->requestTS && processId > senderId))) {
            pakiet->requestTS = pyrkonHost.requestTS;
            sendPacket(pakiet, senderId, WANT_TO_BE_HOST_ACK);
        } else {
            pyrkonHost.waiting.push_back(senderId);
        }
    } else {
        pakiet->requestTS = pyrkonHost.requestTS;
        sendPacket(pakiet, senderId, WANT_TO_BE_HOST_ACK);
    }
}

void wantToBeHostAckHandler(packet_t *pakiet) {
    if (!pyrkonHost.has && pyrkonHost.want) {
        int senderId = pakiet->src;

        if ((pyrkonHost.requestTS > pakiet->requestTS || (pyrkonHost.requestTS == pakiet->requestTS && processId > senderId))) {
            noHostsPermissions++;
        }
        println("Mój stamp %d, przysłany stamp %d, permissions %d", pyrkonHost.requestTS, pakiet->requestTS, noHostsPermissions);

        pyrkonHost.permissions++;
        println("               TICKET PERMISSION stamp %d || Otrzymano już %d", pakiet->requestTS, pyrkonHost.permissions);
        
        if (pyrkonHost.permissions == size - 1) {
            pyrkonHost.has = true;
            if (noHostsPermissions != size - 1) pyrkonHost.has = false;
            println("PRZECHODZĘ");

            sem_post(&pyrkonHostSem);
            
            pyrkonHost.want = false;
            while (!pyrkonHost.waiting.empty()) {
                pakiet->requestTS = pyrkonHost.requestTS;
                sendPacket(pakiet, pyrkonHost.waiting.front(), WANT_TO_BE_HOST_ACK);
                pyrkonHost.waiting.pop_front();
            }
            noHostsPermissions = 0;
            pyrkonHost.permissions = 0;
        }
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
    if (!pyrkonTicket.has && pyrkonTicket.want) {
        pyrkonTicket.permissions++;
        println("               TICKET PERMISSION od %d || Otrzymano już %d", pakiet->src, pyrkonTicket.permissions);
        if (pyrkonTicket.permissions == size - pyrkonTicket.number) {
            pyrkonTicket.has = true;
            pyrkonTicket.requestTS = INT_MAX;
            pyrkonTicket.permissions = 0;
            sem_post(&pyrkonTicketSem);
        }
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
    pyrkonHost.requestTS = INT_MAX;
    pyrkonHost.permissions = 0;
    pyrkonNumber++;
    sem_post(&pyrkonStartSem);
}

void pyrkonTicketsHandler(packet_t *pakiet) {
    println("RECEIVED TICKETS");
    pyrkonTicket.number = pakiet->ticketsNumber;
    // sem_post(&ticketsDetailsSem);
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

void leavingPyrkonHandler(packet_t * pakiet) {
    processesThatLeftPyrkon++;
    println("               WYSZŁO JUŻ %d", processesThatLeftPyrkon);
    if (processesThatLeftPyrkon == size) {
        processesThatLeftPyrkon = 0;
        sem_post(&allLeftPyrkon);
    }
}

void hostChosenHandler(packet_t *pakiet) {
    pyrkonHost.requestTS = INT_MAX;
    pyrkonHost.permissions = 0;
    sem_post(&pyrkonHostSem);
}

void pyrkonNumberIncremented(packet_t * pakiet) {
    sem_post(&pyrkonNumberIncrementedSem);
}

void finishHandler(packet_t *pakiet) {
    programEnd = true;
}