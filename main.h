#ifndef MAINH
#define MAINH

#include <mpi.h>
#include <cstdlib>
#include <cstdio> 
#include <climits>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <ctime>
#include <algorithm>
#include <deque>
#include <string>

using namespace std;

typedef struct {
    int ts;                 /* zegar lamporta */
    int requestTS;          /* zegar lamporta w chwili wysyłania requestu - Ricart-Agrawala Algorithm */
    int pyrkonNumber;       /* numer aktualnego Pyrkonu */
    int workshopNumber;     /* opcjonalny numer warsztatów na które uczestnik chce zdobyć bilet */
    int ticketsNumber;      /* ilość miejsc na dane warsztaty */
    int dst;                /* pole ustawiane w sendPacket */
    int src;                /* pole ustawiane w wątku komunikacyjnym na processId nadawcy */
} packet_t;
#define FIELDNO 7   //liczba pól w strukturze packet_t

typedef struct mutualExclusionStruct {
    mutualExclusionStruct() : requestTS(INT_MAX), want(false), has(false), permissions(0) {}
    int amount;             /* ile jest zasobu danego typu */
    int requestTS;          /* znacznik czasowy wysłania żądania */
    volatile bool want;     /* czy proces chce otrzymać zasób */
    volatile bool has;      /* czy proces ma już zasób */
    int permissions;        /* ilość pozytywnych odpowiedzi na żądanie */
    deque<int> waiting;     /* kolejka procesów czekających na zasób */
} mutualExclusionStruct;

extern mutualExclusionStruct pyrkonHost, pyrkonTicket;
extern deque<mutualExclusionStruct> workshopsTickets;

//Messages structs
extern MPI_Datatype MPI_PACKET_T;

typedef void (*functionPointer)(packet_t *); //typ wskaźnik na funkcję zwracającej void i z argumentem packet_t*

//Messages types
#define WANT_TO_BE_HOST 1
#define WANT_TO_BE_HOST_ACK 2
#define HOST_CHOSEN 3
#define PYRKON_START 4
#define PYRKON_NUMBER_INCREMENTED 5
#define PYRKON_TICKETS 6
#define WORKSHOPS_TICKETS 7
#define GOT_TICKETS_INFO 8
#define WANT_PYRKON_TICKET 9
#define WANT_PYRKON_TICKET_ACK 10
#define WANT_WORKSHOP_TICKET 11
#define WANT_WORKSHOP_TICKET_ACK 12
#define MAX_HANDLERS 13

extern int processId, size, pyrkonNumber, workshopsNumber, lamportTimer;
#define MIN_WORKSHOPS 3
#define MAX_WORKSHOPS 8

extern pthread_mutex_t timerMutex;
extern sem_t pyrkonHostSem, pyrkonStartSem, everyoneGetsTicketsInfoSem, pyrkonTicketSem, pyrkonNumberIncrementedSem, workshopTicketSem;
extern pthread_t ticketsThread;

extern void * prepareAndSendTicketsDetails(void *);
extern void sendPacket(packet_t *, int, int);

#define println(FORMAT, ...) printf("%c[%d;%dm [%d][%d][%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(processId/7))%2, 31+(6+processId)%7, processId, lamportTimer, pyrkonNumber, ##__VA_ARGS__, 27,0,37);

#ifdef DEBUG
#define debug(...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(processId/7))%2, 31+(6+processId)%7, processId, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#endif
