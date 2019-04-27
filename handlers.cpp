#include "main.h"

extern int permissionsReceived;

void startPyrkonRequestHandler(packet_t *);
void startPyrkonPermissionHandler(packet_t *);
void startPyrkonHandler(packet_t *);
void finishHandler(packet_t *);

functionPointer handlers[MAX_HANDLERS];

void initializeHandlers() {
    handlers[WANT_START_PYRKON] = startPyrkonRequestHandler;
    handlers[WANT_START_PYRKON_ACK] = startPyrkonPermissionHandler;
    handlers[PYRKON_START] = startPyrkonHandler;
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

void finishHandler(packet_t *pakiet) {
    programEnd = true;
}