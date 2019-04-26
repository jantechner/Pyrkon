#include "main.h"

// Przypisanie typu wiadomości do handlera na końcu pliku!!!

extern int permissionsReceived;
extern int requestTimestamp;

void finishHandler(packet_t *);
void startPyrkonRequestHandler(packet_t *);
void startPyrkonPermissionHandler(packet_t *);
void startPyrkonHandler(packet_t *);

functionPointer handlers[MAX_HANDLERS];

void initializeHandlers() {
    handlers[FINISH] = finishHandler;
    handlers[WANT_START_PYRKON] = startPyrkonRequestHandler;
    handlers[WANT_START_PYRKON_ACK] = startPyrkonPermissionHandler;
    handlers[PYRKON_START] = startPyrkonHandler;
}


void finishHandler(packet_t *pakiet) {
    println("Otrzymałem FINISH");
    end = true;
}

void startPyrkonRequestHandler(packet_t *pakiet) {
    int requestFrom = pakiet->src;
    println("       Request od procesu %d || Timery: %d %d", requestFrom, requestTimestamp, pakiet->ts);
    if (pyrkonHost != rank) {
        if (requestTimestamp > pakiet->ts || (requestTimestamp == pakiet->ts && rank < requestFrom)) {
            pakiet->src = rank;
            sendPacket(pakiet, requestFrom, WANT_START_PYRKON_ACK);
            println("Permission %d -> %d", rank, requestFrom);
        }
    }
}

void startPyrkonPermissionHandler(packet_t *pakiet) {
    permissionsReceived++;
    println("               PERMISSION od %d || Otrzymano już %d", pakiet->src, permissionsReceived);
    if (permissionsReceived == size - 1) {
        pyrkonHost = rank;
        sem_post(&pyrkonStartSem);
    }
}

void startPyrkonHandler(packet_t *pakiet) {
    // println("START from %d\n", pakiet->src);
    pyrkonHost = pakiet->src;
    sem_post(&pyrkonStartSem);
    println("Pyrkon host: %d", pyrkonHost);
}