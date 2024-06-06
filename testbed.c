#include <stdio.h>

#define PULSE_IMPLMENTATION
#include "pulse.h"

void* do_work(void* arg)
{
    printf("doing work!\n");
}

int main(int argc, char** argv)
{
    pulse_pool_t pool;
    pulse_pool_init(&pool);

    int32_t res = pulse_pool_next_thread(&pool, "my_thread", do_work, NULL);

    if (res != PULSE_ERROR_NONE)
    {
        fprintf(stderr, "Error: %s\n", pulse_strerror(res));
        return 1;
    }

    pulse_thread_t* thread = pulse_pool_get_thread_by_name(&pool, "my_thread");
    if (thread == NULL)
    {
        printf("null thread!\n");
    }
    else
    {
        printf("joining thread!\n");
        pulse_pool_join_thread(&pool, thread);
    }

    return 0;
}