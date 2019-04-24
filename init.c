#include "main.h"

int rank;
int size;

pthread_t threadDelay;
//GQueue *delayStack;
pthread_mutex_t packetMut = PTHREAD_MUTEX_INITIALIZER;

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
        printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
        break;
    case MPI_THREAD_SERIALIZED:
        /* Potrzebne zamki wokół wywołań biblioteki MPI*/
        printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
        break;
    case MPI_THREAD_MULTIPLE:
        printf("Pełne wsparcie dla wątków\n");
        break;
    default:
        printf("Nikt nic nie wie\n");
    }
}

/* Nie ruszać, do użytku wewnętrznego przez wątek komunikacyjny */
typedef struct {
    packet_t *newP;
    int type;
    int dst;
} stackEl_t;

/* Wątek wprowadzający sztuczne opóźnienia komunikacyjne */
/*void *delayFunc(void *ptr) {
    while (!end) {
	int percent = (rand()%2 + 1);
        struct timespec t = { 0, percent*5000 };
        struct timespec rem = { 1, 0 };
        if (!rank)
        nanosleep(&t,&rem);
	pthread_mutex_lock( &packetMut );
	stackEl_t *stackEl = g_queue_pop_tail( delayStack );
	pthread_mutex_unlock( &packetMut );
        if (!end && stackEl) {
	//    println(" GOOD %d %p %d\n", end, stackEl, stackEl->type);
	    MPI_Send( stackEl->newP, 1, MPI_PAKIET_T, stackEl->dst, stackEl->type, MPI_COMM_WORLD);
	    free(stackEl->newP);
	    free(stackEl);
        }
    }
    return 0;
}
*/

void inicjuj(int *argc, char ***argv)
{
    int provided;
    //delayStack = g_queue_new();
    MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);

    /* Stworzenie typu */
    const int nitems = FIELDNO;                             // Struktura ma FIELDNO elementów - przy dodaniu pola zwiększ FIELDNO w main.h !
    int blocklengths[FIELDNO] = {1, 1, 1, 1};               /* tu zwiększyć na [4] = {1,1,1,1} gdy dodamy nowe pole */
    MPI_Aint offsets[FIELDNO];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, kasa);
    offsets[2] = offsetof(packet_t, dst);
    offsets[3] = offsetof(packet_t, src);                        
    MPI_Datatype typy[FIELDNO] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};     /* tu dodać typ nowego pola (np MPI_BYTE, MPI_INT) */
    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);


    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);


    pthread_create(&communicationThread, NULL, comFunc, 0);
    //pthread_create( &threadDelay, NULL, delayFunc, 0);
    if (rank == ROOT)
        pthread_create(&threadM, NULL, monitorFunc, 0);
}

void finalizuj(void)
{
    pthread_mutex_destroy(&konto_mut);
    /* Czekamy, aż wątek potomny się zakończy */
    //println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(communicationThread, NULL);
    //println("czekam na wątek \"opóźniający\"\n" );
    //pthread_join(threadDelay,NULL);
    //if (rank==0) pthread_join(threadM,NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
    //g_queue_free(delayStack);
}

void sendPacket(packet_t *data, int dst, int type)
{

    MPI_Send(data, 1, MPI_PAKIET_T, dst, type, MPI_COMM_WORLD);
    /*
    packet_t *newP = (packet_t *)malloc(sizeof(packet_t));
    stackEl_t *stackEl = (stackEl_t *)malloc(sizeof(stackEl_t));
    memcpy(newP,data, sizeof(packet_t));
    stackEl->dst = dst;
    stackEl->type = type;
    stackEl->newP = newP;
    pthread_mutex_lock( &packetMut );
    g_queue_push_head( delayStack, stackEl );
    pthread_mutex_unlock( &packetMut );
    */
}
