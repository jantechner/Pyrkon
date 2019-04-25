#ifndef MAINH
#define MAINH

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define ROOT 0
#define STARTING_MONEY 1000

typedef struct {
    int ts; /* zegar lamporta */
    int kasa; 
    int dst; /* pole ustawiane w sendPacket */
    int src; /* pole ustawiane w wątku komunikacyjnym na rank nadawcy */
    /* przy dodaniu nowych pól zwiększy FIELDNO i zmodyfikuj plik init.c od linijki 76*/
} packet_t;
#define FIELDNO 4   //liczba pól w strukturze packet_t

typedef void (*f_w)(packet_t *); //typ wskaźnik na funkcję zwracającej void i z argumentem packet_t*

//Messages types 
#define FINISH 1
#define APP_MSG 2
#define GIVE_YOUR_STATE 3
#define MY_STATE_IS 4
#define WANT_START_PYRKON 5
#define MAX_HANDLERS 6 //MAX_HANDLERS musi się równać wartości ostatniego typu pakietu + 1

//Messages structs
MPI_Datatype MPI_PAKIET_T;

extern f_w handlers[MAX_HANDLERS];
extern int rank, size, sum;
extern int lamportTimer;

extern volatile bool end;
extern volatile bool pyrkonInProgress;

extern pthread_t communicationThread, monitorThread, threadDelay;
extern pthread_mutex_t konto_mut, timerMutex;
extern sem_t all_sem;
// extern GQueue *delayStack; //do użytku wewnętrznego (implementacja opóźnień komunikacyjnych)

extern void *monitorFunc();                     //wątek monitora, który po jakimś czasie ma wykryć stan
extern void *comFunc();                         //wątek komunikacyjny
extern void sendPacket(packet_t *, int, int);

#define max(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

#define PROB_OF_SENDING 35
#define PROB_OF_PASSIVE 5
#define PROB_OF_SENDING_DECREASE 3
#define PROB_SENDING_LOWER_LIMIT 1
#define PROB_OF_PASSIVE_INCREASE 1

/* makra do wypisywania na ekranie */
#define P_WHITE printf("%c[%d;%dm",27,1,37);
#define P_BLACK printf("%c[%d;%dm",27,1,30);
#define P_RED printf("%c[%d;%dm",27,1,31);
#define P_GREEN printf("%c[%d;%dm",27,1,33);
#define P_BLUE printf("%c[%d;%dm",27,1,34);
#define P_MAGENTA printf("%c[%d;%dm",27,1,35);
#define P_CYAN printf("%c[%d;%d;%dm",27,1,36);
#define P_SET(X) printf("%c[%d;%dm",27,1,31+(6+X)%7);
#define P_CLR printf("%c[%d;%dm",27,0,37);

/* Tutaj dodaj odwołanie do zegara lamporta */
#define println(FORMAT, ...) printf("%c[%d;%dm [%d][%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamportTimer, ##__VA_ARGS__, 27,0,37);

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.*/

#ifdef DEBUG
#define debug(...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);

#else
#define debug(...) ;
#endif
#endif
