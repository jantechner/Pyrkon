#include "main.h"

extern void notifyOthers(int, int, int);
int processesThatGotTicketsInfo = 0;
deque<hostRequest> hostRequests;

functionPointer handlers[MAX_HANDLERS];

void decideIfIAmHost() {
    bool canIBeHost = true;
    for (int i = 0; i < (int) hostRequests.size(); i++) {
        if (hostRequests[i].TS < myHostRequest.TS || (hostRequests[i].TS == myHostRequest.TS && hostRequests[i].processId < processId)) {
            canIBeHost = false;
            break;
        }
    }
    if (canIBeHost == true) isHost = true;
}

void wantToBeHostHandler(packet_t *pakiet) {
    hostRequest request = {pakiet->src, pakiet->requestTS};
    hostRequests.push_back(request);

    if ((int) hostRequests.size() == size - 1) {
        decideIfIAmHost();
        if (isHost) println("               I AM THE HOST");
        hostRequests.clear();
        myHostRequest.TS = INT_MAX;
        sem_post(&pyrkonHostSem);
    }
}

void wantPyrkonTicketHandler(packet_t *pakiet) {
    int senderId = pakiet->src;
    // println("       Timery: %d %d", pyrkonTicket.requestTS, pakiet->requestTS);
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
    // println("       Timery: %d %d", workshopsTickets[n].requestTS, pakiet->requestTS);
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
        println("           WORKSHOPS NUMBER: %d", workshopsNumber);
    } else {                            /* odbiera ilość biletów na poszczególne warsztaty */
        workshopsTickets.push_back(*new mutualExclusionStruct);
        workshopsTickets[n].amount = pakiet->ticketsNumber;
        println("           WORKSHOP %d TICKETS NUMBER: %d", n, workshopsTickets[n].amount);
        if ((int)workshopsTickets.size() == workshopsNumber) {
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

void pyrkonNumberIncremented(packet_t * pakiet) {
    sem_post(&pyrkonNumberIncrementedSem);
}

void initializeHandlers() {
    handlers[WANT_TO_BE_HOST] = wantToBeHostHandler;
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