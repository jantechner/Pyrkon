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

#define ROOT 0

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

typedef struct ticketHandler {
    ticketHandler() : requestTS(INT_MAX), want(false), has(false), permissions(0) {}
    int number;             /* ile jest biletów danego typu */
    int requestTS;          /* znacznik czasowy wysłania żądania o bilet */
    volatile bool want;              /* czy proces chce bilet */
    volatile bool has;               /* czy proces ma już bilet */
    int permissions;        /* ilość pozytywnych odpowiedzi */
    deque<int> waiting;     /* kolejka procesów czekających na bilet */
} ticketHandler;

extern ticketHandler pyrkonTicket;


//Messages structs
extern MPI_Datatype MPI_PACKET_T;

typedef void (*functionPointer)(packet_t *); //typ wskaźnik na funkcję zwracającej void i z argumentem packet_t*

//Messages types
#define PYRKON_START 1
#define PYRKON_TICKETS 2
#define WORKSHOPS_TICKETS 3
#define LEAVE_PYRKON 4
#define FINISH 5
#define WANT_START_PYRKON 6
#define WANT_START_PYRKON_ACK 7
#define WANT_PYRKON_TICKET 8
#define WANT_PYRKON_TICKET_ACK 9
#define WANT_WORKSHOP_TICKET 10
#define WANT_WORKSHOP_TICKET_ACK 11
#define MAX_HANDLERS 12

extern int processId, size, pyrkonNumber, pyrkonHost;
extern int pyrkonTicketsNumber, workshopsNumber;
#define MIN_WORKSHOPS 3
#define MAX_WORKSHOPS 8
extern int* workshopsTickets;
extern int requestTS, pyrkonTicketRequestTS;
extern int lamportTimer;

extern volatile bool programEnd;
extern volatile bool wantPyrkonTicket;

extern pthread_mutex_t timerMutex;
extern sem_t pyrkonStartSem, ticketsDetailsSem, pyrkonTicketSem, allLeavedPyrkon;
extern pthread_t ticketsThread;
extern void * prepareAndSendTicketsDetails(void *);
// extern GQueue *delayStack; //do użytku wewnętrznego (implementacja opóźnień komunikacyjnych)

extern void sendPacket(packet_t *, int, int);

#define P_WHITE printf("%c[%d;%dm",27,1,37);
#define P_BLACK printf("%c[%d;%dm",27,1,30);
#define P_RED printf("%c[%d;%dm",27,1,31);
#define P_GREEN printf("%c[%d;%dm",27,1,33);
#define P_BLUE printf("%c[%d;%dm",27,1,34);
#define P_MAGENTA printf("%c[%d;%dm",27,1,35);
#define P_CYAN printf("%c[%d;%d;%dm",27,1,36);
#define P_SET(X) printf("%c[%d;%dm",27,1,31+(6+X)%7);
#define P_CLR printf("%c[%d;%dm",27,0,37);

#define println(FORMAT, ...) printf("%c[%d;%dm [%d][%d][%d][%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(processId/7))%2, 31+(6+processId)%7, processId, lamportTimer, requestTS == INT_MAX ? 0 : requestTS, pyrkonTicketRequestTS == INT_MAX ? 0 : pyrkonTicketRequestTS, ##__VA_ARGS__, 27,0,37);

#ifdef DEBUG
#define debug(...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(processId/7))%2, 31+(6+processId)%7, processId, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#endif
