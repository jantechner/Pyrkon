#include "main.h"

extern int noHostsPermissions;
extern void notifyOthers(int, int, int);
int workshopsTicketsNumberReceived;
int processesThatGotTicketsInfo = 0;

functionPointer handlers[MAX_HANDLERS];

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
        println("               PYRKON TICKET PERMISSION od %d || Otrzymano już %d", pakiet->src, pyrkonTicket.permissions);
        if (pyrkonTicket.permissions == size - pyrkonTicket.amount) {
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

void wantWorkshopTicketHandler(packet_t *pakiet) {
    int senderId = pakiet->src;
    int n = pakiet->workshopNumber;
    println("       Timery: %d %d", workshopsTickets[n].requestTS, pakiet->requestTS);
    if (workshopsTickets[n].want) {
        if (!workshopsTickets[n].has && (workshopsTickets[n].requestTS > pakiet->requestTS || (workshopsTickets[n].requestTS == pakiet->requestTS && processId > senderId))) {
            sendPacket(pakiet, senderId, WANT_WORKSHOP_TICKET_ACK);
        } else {
            workshopsTickets[n].waiting.push_back(senderId);
        }
    } else {
        sendPacket(pakiet, senderId, WANT_WORKSHOP_TICKET_ACK);
    }
}
void wantWorkshopTicketAckHandler(packet_t *pakiet) {
    int n = pakiet->workshopNumber;
    if (!workshopsTickets[n].has && workshopsTickets[n].want) {
        workshopsTickets[n].permissions++;
        println("               WORKSHOP %d TICKET PERMISSION od %d || Otrzymano już %d", n, pakiet->src, workshopsTickets[n].permissions);
        if (workshopsTickets[n].permissions == size - workshopsTickets[n].amount || workshopsTickets[n].amount == size) {
            workshopsTickets[n].has = true;
            workshopsTickets[n].requestTS = INT_MAX;
            workshopsTickets[n].permissions = 0;
            sem_post(&workshopTicketSem);
        }
    }
}
void freeWorkshopTicket(int n) {
    workshopsTickets[n].want = false;
    workshopsTickets[n].has = false;
    while (!workshopsTickets[n].waiting.empty()) {
        packet_t pakiet;
        pakiet.workshopNumber = n;
        sendPacket(&pakiet, workshopsTickets[n].waiting.front(), WANT_WORKSHOP_TICKET_ACK);
        workshopsTickets[n].waiting.pop_front();
    }
}

void startPyrkonHandler(packet_t *pakiet) {
    pyrkonHost.requestTS = INT_MAX;
    pyrkonHost.permissions = 0;
    pyrkonNumber++;
    sem_post(&pyrkonStartSem);
}

void pyrkonTicketsHandler(packet_t *pakiet) {
    pyrkonTicket.amount = pakiet->ticketsNumber;
    println("           PYRKON TICKETS: %d", pyrkonTicket.amount);
}

void workshopsTicketsHandler(packet_t *pakiet) {
    int n = pakiet->workshopNumber;
    if (n == -1) {                      /* odbiera ilość warsztatów */
        workshopsNumber = pakiet->ticketsNumber;
        workshopsTickets.clear();
        workshopsTicketsNumberReceived = 0;
        println("           WORKSHOPS NUMBER: %d", workshopsNumber);
    } else {                            /* odbiera ilość biletów na poszczególne warsztaty */
        workshopsTickets.push_back(*new mutualExclusionStruct);
        workshopsTickets[n].amount = pakiet->ticketsNumber;
        workshopsTicketsNumberReceived++;
        println("           WORKSHOP %d TICKETS NUMBER: %d", n, workshopsTickets[n].amount);
        if (workshopsTicketsNumberReceived == workshopsNumber) {
            notifyOthers(GOT_TICKETS_INFO, 0, 0);
        }
    }
}

void gotTicketsInfoHandler(packet_t * pakiet) {
    processesThatGotTicketsInfo++;
    if (processesThatGotTicketsInfo == size - 1) {
        processesThatGotTicketsInfo = 0;
        sem_post(&everyoneGetsTicketsInfoSem);
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

void initializeHandlers() {
    handlers[WANT_TO_BE_HOST] = wantToBeHostHandler;
    handlers[WANT_TO_BE_HOST_ACK] = wantToBeHostAckHandler;
    handlers[HOST_CHOSEN] = hostChosenHandler;
    handlers[PYRKON_START] = startPyrkonHandler;
    handlers[PYRKON_NUMBER_INCREMENTED] = pyrkonNumberIncremented;
    handlers[PYRKON_TICKETS] = pyrkonTicketsHandler;
    handlers[WORKSHOPS_TICKETS] = workshopsTicketsHandler;
    handlers[GOT_TICKETS_INFO]  = gotTicketsInfoHandler;
    handlers[WANT_PYRKON_TICKET] = wantPyrkonTicketHandler;
    handlers[WANT_PYRKON_TICKET_ACK] = wantPyrkonTicketAckHandler;
    handlers[WANT_WORKSHOP_TICKET] = wantWorkshopTicketHandler;
    handlers[WANT_WORKSHOP_TICKET_ACK] = wantWorkshopTicketAckHandler;
}