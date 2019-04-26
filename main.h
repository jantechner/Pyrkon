#ifndef MAINH
#define MAINH

#include <mpi.h>
#include <cstdlib>
#include <cstdio> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

#define ROOT 0

typedef struct {
    int ts; /* zegar lamporta */
    int kasa; 
    int dst; /* pole ustawiane w sendPacket */
    int src; /* pole ustawiane w wątku komunikacyjnym na rank nadawcy */
    /* przy dodaniu nowych pól zwiększy FIELDNO i zmodyfikuj plik init.c od linijki 76*/
} packet_t;
#define FIELDNO 4   //liczba pól w strukturze packet_t

typedef void (*functionPointer)(packet_t *); //typ wskaźnik na funkcję zwracającej void i z argumentem packet_t*

//Messages types 
#define FINISH 1
#define WANT_START_PYRKON 2
#define WANT_START_PYRKON_ACK 3
#define PYRKON_START 4
#define MAX_HANDLERS 5 //MAX_HANDLERS musi się równać wartości ostatniego typu pakietu + 1

//Messages structs
extern MPI_Datatype MPI_PAKIET_T;
extern int rank, size;
extern int lamportTimer;

extern volatile bool end;
extern int pyrkonHost;

extern pthread_t communicationThread/*, threadDelay*/;
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

#define println(FORMAT, ...) printf("%c[%d;%dm [%d][%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamportTimer, ##__VA_ARGS__, 27,0,37);

#ifdef DEBUG
#define debug(...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#endif
