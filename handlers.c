#include "main.h"

// Przypisanie typu wiadomości do handlera na końcu pliku!!!

extern int konto;

void myStateHandler(packet_t *pakiet) {
    static int statePacketsCnt = 0;  //zmienna statyczna, istnieje przez cały czas działania programu

    statePacketsCnt++;
    sum += pakiet->kasa;
    println("Suma otrzymana: %d, total: %d\n", pakiet->kasa, sum);
    //println( "%d statePackets from %d\n", statePacketsCnt, pakiet->src);
    if (statePacketsCnt == size)
    {
        sem_post(&all_sem);
    }
}

void finishHandler(packet_t *pakiet) {
    /* właściwie nie wykorzystywane */
    println("Otrzymałem FINISH");
    end = TRUE;
}

void giveHandler(packet_t *pakiet) {
    /* monitor prosi, by mu podać stan kasy */
    /* tutaj odpowiadamy monitorowi, ile mamy kasy. Pamiętać o muteksach! */
    println("dostałem GIVE STATE\n");

    packet_t tmp;
    tmp.kasa = konto;
    sendPacket(&tmp, ROOT, MY_STATE_IS);
}

void appMsgHandler(packet_t *pakiet) {
    /* ktoś przysłał mi przelew */
    println("\tdostałem %d od %d\n", pakiet->kasa, pakiet->src);
    pthread_mutex_lock(&konto_mut);
    konto += pakiet->kasa;
    println("Stan obecny: %d\n", konto);
    pthread_mutex_unlock(&konto_mut);
}

f_w handlers[MAX_HANDLERS] = {[GIVE_YOUR_STATE] = giveHandler,
                              [FINISH] = finishHandler,
                              [MY_STATE_IS] = myStateHandler,
                              [APP_MSG] = appMsgHandler};