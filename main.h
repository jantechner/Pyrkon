#ifndef MAINH
#define MAINH

#include <mpi.h>
#include <cstdlib>
#include <cstdio> 
#include <climits>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <algorithm>
#include <string>

using namespace std;

#define ROOT 0

typedef struct {
    int ts;                 /* zegar lamporta */
    int requestTimestamp;   /* zegar lamporta w chwili wysyłania requestu - Ricart-Agrawala Algorithm */
    int pyrkonNumber;       /* numer aktualnego Pyrkonu */
    int workshopNumber;     /* opcjonalny numer warsztatów na które uczestnik chce zdobyć bilet */
    int ticketsNumber;      /* ilość miejsc na dane warsztaty */
    int dst;                /* pole ustawiane w sendPacket */
    int src;                /* pole ustawiane w wątku komunikacyjnym na processID nadawcy */
} packet_t;
#define FIELDNO 7   //liczba pól w strukturze packet_t

//Messages structs
extern MPI_Datatype MPI_PACKET_T;

typedef void (*functionPointer)(packet_t *); //typ wskaźnik na funkcję zwracającej void i z argumentem packet_t*

//Messages types
#define PYRKON_START 1
#define FINISH 2
#define WANT_START_PYRKON 3
#define WANT_START_PYRKON_ACK 4
#define WANT_PYRKON_TICKET 5
#define WANT_PYRKON_TICKET_ACK 6
#define WANT_WORKSHOP_TICKET 7
#define WANT_WORKSHOP_TICKET_ACK 8

#define MAX_HANDLERS 9

extern int processID, size, pyrkonNumber, pyrkonHost;
extern int requestTimestamp;
extern int lamportTimer;

extern volatile bool programEnd;

extern pthread_mutex_t timerMutex;
extern sem_t pyrkonStartSem;
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

#define println(FORMAT, ...) printf("%c[%d;%dm [%d][%d][%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(processID/7))%2, 31+(6+processID)%7, processID, lamportTimer, requestTimestamp == INT_MAX ? -1 : requestTimestamp, ##__VA_ARGS__, 27,0,37);

#ifdef DEBUG
#define debug(...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(processID/7))%2, 31+(6+processID)%7, processID, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#endif
