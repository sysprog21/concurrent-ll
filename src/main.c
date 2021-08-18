#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "list.h"
#include "utils.h"

#define XSTR(s) STR(s)
#define STR(s) #s

/* default percentage of reads */
#define DEFAULT_READS 80
#define DEFAULT_UPDATES 20

/* default number of threads */
#define DEFAULT_NUM_THREADS 1

/* default experiment duration in miliseconds */
#define DEFAULT_DURATION 1000

/* the maximum value the key stored in the list can take; defines key range */
#define DEFAULT_RANGE 2048

static uint32_t finds;
static uint32_t max_key;

/* used to signal the threads when to stop */
static ALIGNED(64) uint8_t running[64];

/* per-thread seeds for the custom random function */
__thread uint64_t *seeds;

static list_t *the_list;

/* a simple barrier implementation, used to make sure all threads start the
 * experiment at the same time.
 */
typedef struct barrier {
    pthread_cond_t complete;
    pthread_mutex_t mutex;
    int count;
    int crossing;
} barrier_t;

void barrier_init(barrier_t *b, int n)
{
    pthread_cond_init(&b->complete, NULL);
    pthread_mutex_init(&b->mutex, NULL);
    b->count = n;
    b->crossing = 0;
}

void barrier_cross(barrier_t *b)
{
    pthread_mutex_lock(&b->mutex);
    b->crossing++;                /* One more thread through */
    if (b->crossing < b->count) { /* If not all here, wait */
        pthread_cond_wait(&b->complete, &b->mutex);
    } else {
        pthread_cond_broadcast(&b->complete);
        b->crossing = 0; /* Reset for next time */
    }
    pthread_mutex_unlock(&b->mutex);
}

/* data structure through which we send parameters to and get results from the
 * worker threads.
 */
typedef ALIGNED(64) struct thread_data {
    barrier_t *barrier;  /* pointer to the global barrier */
    unsigned long n_ops; /* operations each thread performs */
    uint64_t n_add; /* elements each thread should add at beginning of exec */
    unsigned long n_insert; /* number of inserts a thread performs */
    unsigned long n_remove; /* number of removes a thread performs */
    unsigned long n_search; /* number of searches a thread performs */
    int id; /* the id of the thread (used for thread placement on cores) */
} thread_data_t;

void *test(void *data)
{
    thread_data_t *d = (thread_data_t *) data; /* per-thread data */
    /* scale percentages of the various operations to the range 0..255.
     * this saves us a floating point operation during the benchmark
     * e.g instead of random()%100 to determine the next operation we will do,
     * we can simply do random() & 256
     */
    uint32_t read_thresh = 256 * finds / 100;
    seeds = seed_rand(); /* the custom random number generator */
    uint32_t rand_max = max_key;
    val_t the_value;
    int last = -1;

    /* before starting the test, we insert a number of elements in the data
     * structure.
     * we do this at each thread to avoid the situation where the entire data
     * structure resides in the same memory node.
     */
    for (int i = 0; i < d->n_add; ++i) {
        the_value =
            (val_t) my_random(&seeds[0], &seeds[1], &seeds[2]) & rand_max;
        /* we make sure the insert was effective (as opposed to just updating an
         * existing entry).
         */
        if (list_add(the_list, the_value) == 0)
            i--;
    }

    /* Wait on barrier */
    barrier_cross(d->barrier);
    while (*running) { /* start the test */
        /* generate value (node that rand_max is expected to be power of 2) */
        the_value = my_random(&seeds[0], &seeds[1], &seeds[2]) & rand_max;
        /* generate the operation */
        uint32_t op = my_random(&seeds[0], &seeds[1], &seeds[2]) & 0xff;
        if (op < read_thresh) { /* do a find operation */
            list_contains(the_list, the_value);
        } else if (last == -1) { /* do a write operation */
            if (list_add(the_list, the_value)) {
                d->n_insert++;
                last = 1;
            }
        } else {
            if (list_remove(the_list, the_value)) { /* do a delete operation */
                d->n_remove++;
                last = -1;
            }
        }
        d->n_ops++;
    }
    return NULL;
}

void catcher(int sig)
{
    static int nb = 0;
    printf("CAUGHT SIGNAL %d\n", sig);
    if (++nb >= 3)
        exit(1);
}

int main(int argc, char *const argv[])
{
    pthread_t *threads;
    pthread_attr_t attr;
    barrier_t barrier;
    struct timeval start, end;
    struct timespec timeout;

    thread_data_t *data;
    sigset_t block_set;

    /* initially, set parameters to their default values */
    int n_threads = DEFAULT_NUM_THREADS;
    max_key = DEFAULT_RANGE;
    uint32_t updates = DEFAULT_UPDATES;
    finds = DEFAULT_READS;
    int duration = DEFAULT_DURATION;

    /* now read the parameters in case the user provided values for them.
     * we use getopt, the same skeleton may be used for other bechmarks,
     * though the particular parameters may be different.
     */
    struct option long_options[] = {
        /* These options don't set a flag */
        {"help", no_argument, NULL, 'h'},
        {"duration", required_argument, NULL, 'd'},
        {"range", required_argument, NULL, 'r'},
        {"initial", required_argument, NULL, 'i'},
        {"num-threads", required_argument, NULL, 'n'},
        {"updates", required_argument, NULL, 'u'},
        {NULL, 0, NULL, 0}};

    /* actually get the parameters form the command-line */
    while (1) {
        int i = 0;
        int c = getopt_long(argc, argv, "hd:n:l:u:i:r:", long_options, &i);
        if (c == -1)
            break;

        if (c == 0 && long_options[i].flag == 0)
            c = long_options[i].val;

        switch (c) {
        case 0:
            /* Flag is automatically set */
            break;
        case 'h':
            printf("stress test\n"
                   "\n"
                   "Usage:\n"
                   "  %s [options...]\n"
                   "\n"
                   "Options:\n"
                   "  -h, --help\n"
                   "        Print this message\n"
                   "  -d, --duration <int>\n"
                   "        Test duration in milliseconds (0=infinite, default=" XSTR(DEFAULT_DURATION) ")\n"
                   "  -u, --updates <int>\n"
                   "        Percentage of update operations (default=" XSTR(DEFAULT_UPDATES) ")\n"
                   "  -r, --range <int>\n"
                   "        Key range (default=" XSTR(DEFAULT_RANGE) ")\n"
                   "  -n, --num-threads <int>\n"
                   "        Number of threads (default=" XSTR(DEFAULT_NUM_THREADS) ")\n",
		   argv[0]
            );
            exit(0);
        case 'd':
            duration = atoi(optarg);
            break;
        case 'u':
            updates = atoi(optarg);
            finds = 100 - updates;
            break;
        case 'r':
            max_key = atoi(optarg);
            break;
        case 'i':
            break;
        case 'l':
            break;
        case 'n':
            n_threads = atoi(optarg);
            break;
        case '?':
            printf("Use -h or --help for help\n");
            exit(0);
        default:
            exit(1);
        }
    }

    max_key--;
    /* we round the max key up to the nearest power of 2, which makes our random
     * key generation more efficient.
     */
    max_key = pow2roundup(max_key) - 1;

    /* initialization of the list */
    the_list = list_new();

    /* initialize the data which will be passed to the threads */
    if ((data = malloc(n_threads * sizeof(thread_data_t))) == NULL) {
        perror("malloc");
        exit(1);
    }

    if ((threads = malloc(n_threads * sizeof(pthread_t))) == NULL) {
        perror("malloc");
        exit(1);
    }

    /* flag signaling the threads until when to run */
    *running = 1;

    /* global barrier init (used to start the threads at the same time) */
    barrier_init(&barrier, n_threads + 1);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    timeout.tv_sec = duration / 1000;
    timeout.tv_nsec = (duration % 1000) * 1000000;

    /* set the data for each thread and create the threads */
    for (int i = 0; i < n_threads; i++) {
        data[i].id = i;
        data[i].n_ops = 0;
        data[i].n_insert = 0;
        data[i].n_remove = 0;
        data[i].n_search = 0;
        data[i].n_add = max_key / (2 * n_threads);
        if (i < ((max_key / 2) % n_threads))
            data[i].n_add++;
        data[i].barrier = &barrier;
        if (pthread_create(&threads[i], &attr, test, (void *) (&data[i])) !=
            0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }
    pthread_attr_destroy(&attr);

    /* Catch some signals */
    if (signal(SIGHUP, catcher) == SIG_ERR ||
        signal(SIGINT, catcher) == SIG_ERR ||
        signal(SIGTERM, catcher) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    /* Start threads */
    barrier_cross(&barrier);
    gettimeofday(&start, NULL);
    if (duration > 0) {
        /* sleep for the duration of the experiment */
        nanosleep(&timeout, NULL);
    } else {
        sigemptyset(&block_set);
        sigsuspend(&block_set);
    }

    /* signal the threads to stop */
    *running = 0;
    gettimeofday(&end, NULL);

    /* Wait for thread completion */
    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error waiting for thread completion\n");
            exit(1);
        }
    }

    /* compute the exact duration of the experiment */
    duration = (end.tv_sec * 1000 + end.tv_usec / 1000) -
               (start.tv_sec * 1000 + start.tv_usec / 1000);

    unsigned long operations = 0;
    long reported_total = 0;
    /* report some experiment statistics */
    for (int i = 0; i < n_threads; i++) {
        printf("Thread %d\n", i);
        printf("  #operations   : %lu\n", data[i].n_ops);
        printf("  #inserts   : %lu\n", data[i].n_insert);
        printf("  #removes   : %lu\n", data[i].n_remove);
        operations += data[i].n_ops;
        reported_total = reported_total + data[i].n_add + data[i].n_insert -
                         data[i].n_remove;
    }

    printf("Duration      : %d (ms)\n", duration);
    printf("#txs     : %lu (%f / s)\n", operations,
           operations * 1000.0 / duration);
    printf("Expected size: %ld Actual size: %d\n", reported_total,
           list_size(the_list));

    free(threads);
    free(data);

    return 0;
}
