#include "main.h"

int processId, size, lamportTimer, pyrkonNumber = 0, pyrkonHost = -1, pyrkonTicketsNumber, workshopsNumber;
int* workshopsTickets;
int requestTS = INT_MAX;
int pyrkonTicketRequestTS = INT_MAX;
volatile bool programEnd = false;
volatile bool wantPyrkonTicket = true;

ticketHandler pyrkonTicket;

pthread_mutex_t konto_mut = PTHREAD_MUTEX_INITIALIZER, 
                timerMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t pyrkonStartSem, ticketsDetailsSem, pyrkonTicketSem, allLeavedPyrkon;
pthread_t communicationThread, ticketsThread;

extern void initializeHandlers();
extern void * comFunc(void *);

MPI_Datatype MPI_PACKET_T;


// pthread_t threadDelay;
// GQueue *delayStack;
// pthread_mutex_t packetMut = PTHREAD_MUTEX_INITIALIZER;

/* typedef struct {  // Nie ruszać, do użytku wewnętrznego przez wątek komunikacyjny 
    packet_t *newP;
    int type;
    int dst;
} stackEl_t; */

/* void *delayFunc(void *ptr) {   //Wątek wprowadzający sztuczne opóźnienia komunikacyjne
    while (!programEnd) {
	int percent = (rand()%2 + 1);
        struct timespec t = { 0, percent*5000 };
        struct timespec rem = { 1, 0 };
        if (!processId)
        nanosleep(&t,&rem);
	pthread_mutex_lock( &packetMut );
	stackEl_t *stackEl = g_queue_pop_tail( delayStack );
	pthread_mutex_unlock( &packetMut );
        if (!programEnd && stackEl) {
	//    println(" GOOD %d %p %d\n", end, stackEl, stackEl->type);
	    MPI_Send( stackEl->newP, 1, MPI_PACKET_T, stackEl->dst, stackEl->type, MPI_COMM_WORLD);
	    free(stackEl->newP);
	    free(stackEl);
        }
    }
    return 0;
} */

void check_thread_support(int provided) {
    printf("THREAD SUPPORT: %d\n", provided);
    switch (provided)
    {
    case MPI_THREAD_SINGLE:
        printf("Brak wsparcia dla wątków, kończę\n");
        fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
        MPI_Finalize();
        exit(-1);
        break;
    case MPI_THREAD_FUNNELED:
        printf("Tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
        break;
    case MPI_THREAD_SERIALIZED:
        /* Potrzebne zamki wokół wywołań biblioteki MPI*/
        printf("Tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
        break;
    case MPI_THREAD_MULTIPLE:
        printf("Pełne wsparcie dla wątków\n");
        break;
    default:
        printf("Nikt nic nie wie\n");
    }
}

void initializeMPI(int argc, char *argv[]) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    // check_thread_support(provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);  
    MPI_Comm_size(MPI_COMM_WORLD, &size);
}

void createMPIDataTypes() {
    //MPI_PACKET_T
    const int nitems = FIELDNO;                    
    int blocklengths[FIELDNO] = {1, 1, 1, 1, 1, 1, 1};          
    MPI_Aint offsets[FIELDNO];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, requestTS);
    offsets[2] = offsetof(packet_t, pyrkonNumber);
    offsets[3] = offsetof(packet_t, workshopNumber);
    offsets[4] = offsetof(packet_t, ticketsNumber);
    offsets[5] = offsetof(packet_t, dst);
    offsets[6] = offsetof(packet_t, src);                        
    MPI_Datatype typy[FIELDNO] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};     /* tu dodać typ nowego pola (np MPI_BYTE, MPI_INT) */
    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PACKET_T);
    MPI_Type_commit(&MPI_PACKET_T);
}

void initializeLamportTimer() {
    lamportTimer = 0;               //wszyscy startują z zerowym zegarem 
    // pthread_mutex_lock(&timerMutex);
    // lamportTimer = processId;
    // pthread_mutex_unlock(&timerMutex);
}

void runThreads() {
    pthread_create(&communicationThread, NULL, comFunc, 0);
    //delayStack = g_queue_new();
    //pthread_create( &threadDelay, NULL, delayFunc, 0);
}

void initializeSemaphores() {
    sem_init(&pyrkonStartSem, 0, 0);
    sem_init(&ticketsDetailsSem, 0, 0);
    sem_init(&pyrkonTicketSem, 0, 0);
    sem_init(&allLeavedPyrkon, 0, 0);
}

void initialize(int argc, char *argv[]) {
    println("Process initialized");
    initializeMPI(argc, argv);
    createMPIDataTypes();
    initializeLamportTimer();
    initializeHandlers();
    // srand(processId); //for every process set unique rand seed
    srand(time(NULL));
    initializeSemaphores();
    runThreads();
}

void finalize(void) {
    pthread_mutex_destroy(&timerMutex);
    pthread_join(communicationThread, NULL);
    pthread_join(ticketsThread, NULL);
    sem_destroy(&pyrkonStartSem);
    sem_destroy(&pyrkonTicketSem);
    //pthread_join(threadDelay,NULL);

    MPI_Type_free(&MPI_PACKET_T);
    MPI_Finalize();
    //g_queue_free(delayStack);
}
