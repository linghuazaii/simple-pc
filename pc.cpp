#include "pc.h"

int main(int argc, char **argv) {
    pool_t *pool = pool_init();
    start_producer(pool);
    start_consumer(pool);

    sleep(86400);

    return 0;
}
