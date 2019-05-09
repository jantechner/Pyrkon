#include "main.h"

int processId, size, lamportTimer, pyrkonNumber = 0, workshopsNumber;
mutualExclusionStruct pyrkonTicket;
deque<mutualExclusionStruct> workshopsTickets;

hostRequest myHostRequest = {processId, INT_MAX};
bool isHost = false;

pthread_mutex_t konto_mut = PTHREAD_MUTEX_INITIALIZER, 
                timerMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t pyrkonHostSem, pyrkonStartSem, everyoneGetsTicketsInfoSem, pyrkonTicketSem, pyrkonNumberIncrementedSem, workshopTicketSem;
pthread_t communicationThread, ticketsThread;

extern void initializeHandlers();
extern void * comFunc(void *);

MPI_Datatype MPI_PACKET_T;

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
}

void initializeSemaphores() {
    sem_init(&pyrkonHostSem, 0, 0);
    sem_init(&pyrkonStartSem, 0, 0);
    sem_init(&everyoneGetsTicketsInfoSem, 0, 0);
    sem_init(&pyrkonTicketSem, 0, 0);
    sem_init(&pyrkonNumberIncrementedSem, 0, 0);
    sem_init(&workshopTicketSem, 0, 0);
}

void initialize(int argc, char *argv[]) {
    
    initializeMPI(argc, argv);
    createMPIDataTypes();
    initializeLamportTimer();
    initializeHandlers();
    srand(processId); //for every process set unique rand seed
    // srand(time(NULL));
    initializeSemaphores();  //musi być przed inicjalizacją wątków
    println("Process initialized");
    runThreads();
}

void finalize(void) {
    pthread_mutex_destroy(&timerMutex);
    pthread_join(communicationThread, NULL);
    pthread_join(ticketsThread, NULL);
    sem_destroy(&pyrkonStartSem);
    sem_destroy(&pyrkonTicketSem);

    MPI_Type_free(&MPI_PACKET_T);
    MPI_Finalize();
}
