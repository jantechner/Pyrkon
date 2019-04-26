#include "main.h"

// Przypisanie typu wiadomości do handlera na końcu pliku!!!

extern int permissionsReceived;
extern int timerBeforeReceiving;

// void finishHandler(packet_t *);
// void startPyrkonRequestHandler(packet_t *);
// void startPyrkonPermissionHandler(packet_t *);
// void startPyrkonHandler(packet_t *);

f_w handlers[MAX_HANDLERS];


void finishHandler(packet_t *pakiet) {
    println("Otrzymałem FINISH");
    end = true;
}

void startPyrkonRequestHandler(packet_t *pakiet) {
    int requestFrom = pakiet->src;
    println("       Request od procesu %d || Timery: %d %d", requestFrom, timerBeforeReceiving, pakiet->ts);
    if (pyrkonHost != rank) {
        if (timerBeforeReceiving > pakiet->ts || (timerBeforeReceiving == pakiet->ts && rank < requestFrom)) {
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
        sem_post(&all_sem);
    }
}

void startPyrkonHandler(packet_t *pakiet) {
    // println("START from %d\n", pakiet->src);
    // pyrkonInProgress = true;
    pyrkonHost = pakiet->src;
    sem_post(&all_sem);
    println("Pyrkon host: %d", pyrkonHost);
}

void initializeHandlers() {
    handlers[FINISH] = finishHandler;
    handlers[WANT_START_PYRKON] = startPyrkonRequestHandler;
    handlers[WANT_START_PYRKON_ACK] = startPyrkonPermissionHandler;
    handlers[PYRKON_START] = startPyrkonHandler;
}


/* void myStateHandler(packet_t *pakiet) {
    static int statePacketsCnt = 0;  //zmienna statyczna, istnieje przez cały czas działania programu

    statePacketsCnt++;
    sum += pakiet->kasa;
    println("Suma otrzymana: %d, total: %d\n", pakiet->kasa, sum);
    //println( "%d statePackets from %d\n", statePacketsCnt, pakiet->src);
    if (statePacketsCnt == size)
    {
        sem_post(&all_sem);
    }
} */
