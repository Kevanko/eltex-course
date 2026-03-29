#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define STALL_COUNT 5
#define CUSTOMER_COUNT 3
#define STALL_MIN_STOCK 900
#define STALL_MAX_STOCK 1100
#define NEED_MIN 9900
#define NEED_MAX 10100
#define LOADER_ADD 200

typedef struct {
    int id;
    int stock;
    pthread_mutex_t mutex;
} Stall;

typedef struct {
    int id;
    int need;
    unsigned int seed;
} Customer;

static Stall stalls[STALL_COUNT];
static pthread_t customers[CUSTOMER_COUNT];
static pthread_t loader_thread;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
static int stop_loader = 0;

static void log_message(const char *fmt, ...)
{
    va_list args;

    pthread_mutex_lock(&log_mutex);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}

static int random_range(unsigned int *seed, int min, int max)
{
    return min + (int)(rand_r(seed) % (unsigned int)(max - min + 1));
}

static void *customer_routine(void *arg)
{
    Customer *customer = (Customer *)arg;

    log_message("Покупатель %d проснулся, потребность = %d\n",
                customer->id, customer->need);

    while (customer->need > 0) {
        int stall_index = random_range(&customer->seed, 0, STALL_COUNT - 1);
        Stall *stall = &stalls[stall_index];

        pthread_mutex_lock(&stall->mutex);
        log_message("Покупатель %d зашел в ларек %d, товаров = %d\n",
                    customer->id, stall->id, stall->stock);

        if (stall->stock > 0) {
            int taken = stall->stock;

            if (taken > customer->need) {
                taken = customer->need;
            }

            stall->stock -= taken;
            customer->need -= taken;

            log_message("Покупатель %d купил %d, осталось потребности = %d\n",
                        customer->id, taken, customer->need);
        } else {
            log_message("Покупатель %d увидел пустой ларек %d\n",
                        customer->id, stall->id);
        }

        pthread_mutex_unlock(&stall->mutex);

        if (customer->need <= 0) {
            log_message("Покупатель %d насытился и ушел\n", customer->id);
            break;
        }

        log_message("Покупатель %d уснул на 2 секунды\n", customer->id);
        sleep(2);
    }

    return NULL;
}

static void *loader_routine(void *arg)
{
    (void)arg;

    unsigned int seed = (unsigned int)time(NULL);

    while (1) {
        pthread_mutex_lock(&state_mutex);
        if (stop_loader) {
            pthread_mutex_unlock(&state_mutex);
            break;
        }
        pthread_mutex_unlock(&state_mutex);

        log_message("Погрузчик проснулся\n");

        int stall_index = random_range(&seed, 0, STALL_COUNT - 1);
        Stall *stall = &stalls[stall_index];

        pthread_mutex_lock(&stall->mutex);
        stall->stock += LOADER_ADD;
        log_message("Погрузчик пополнил ларек %d на %d, теперь товаров = %d\n",
                    stall->id, LOADER_ADD, stall->stock);
        pthread_mutex_unlock(&stall->mutex);

        log_message("Погрузчик уснул на 1 секунду\n");
        sleep(1);
    }

    log_message("Погрузчик завершил работу\n");
    return NULL;
}

int main(void)
{
    unsigned int seed = (unsigned int)time(NULL);
    Customer customer_data[CUSTOMER_COUNT];

    for (int i = 0; i < STALL_COUNT; ++i) {
        stalls[i].id = i + 1;
        stalls[i].stock = random_range(&seed, STALL_MIN_STOCK, STALL_MAX_STOCK);
        pthread_mutex_init(&stalls[i].mutex, NULL);
        log_message("Ларек %d стартует с %d товарами\n",
                    stalls[i].id, stalls[i].stock);
    }

    pthread_create(&loader_thread, NULL, loader_routine, NULL);

    for (int i = 0; i < CUSTOMER_COUNT; ++i) {
        customer_data[i].id = i + 1;
        customer_data[i].need = random_range(&seed, NEED_MIN, NEED_MAX);
        customer_data[i].seed = seed ^ (unsigned int)(i + 1) * 12345U;

        pthread_create(&customers[i], NULL, customer_routine, &customer_data[i]);
    }

    for (int i = 0; i < CUSTOMER_COUNT; ++i) {
        pthread_join(customers[i], NULL);
    }

    pthread_mutex_lock(&state_mutex);
    stop_loader = 1;
    pthread_mutex_unlock(&state_mutex);

    pthread_join(loader_thread, NULL);

    for (int i = 0; i < STALL_COUNT; ++i) {
        pthread_mutex_destroy(&stalls[i].mutex);
    }

    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&state_mutex);

    return 0;
}
