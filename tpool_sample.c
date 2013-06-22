#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>


#include "tpool.h"

#define N_ELEMS(array) (sizeof array / sizeof array[0])

typedef struct {
        pthread_mutex_t lock;
        pthread_cond_t cond;

        int n_done;
        int n_remain;
        int n_error;

        int value;
} tshared_data;

/* example */
typedef struct {
        ttask task; /* at the beginning of the structure: mandatory*/

        int foo;
        tshared_data *shared;
} tfoo_task;

static void
foo_task(tfoo_task *task)
{
        int v;
        int p;
        int i;

        /* basically, if an error occurred, increment the n_error here */

        pthread_mutex_lock(&task->shared->lock);
        v = task->foo;
        p = ++task->shared->n_done;
        i = task->shared->value;
        fprintf(stderr, "value: %d, n_done: %d, shared_int: %d\n", v, p, i);
        pthread_cond_signal(&task->shared->cond);
        pthread_mutex_unlock(&task->shared->lock);

}

int
main(void)
{
        int ret;
        int i;
        int vals[5] = {10, 42, 1337, 31337, 1979};
        tshared_data *shared = NULL;
        ttask_pool *pool = NULL;
        int shared_locked = 0;

        pool = ttask_pool_create(DEFAULT_N_WORKERS);
        if (! pool) {
                perror("allocation");
                goto end;
        }

        shared = calloc(1, sizeof *shared);
        if (! shared) {
                perror("calloc");
                goto end;
        }

        shared->value = 42;

        assert(0 == pthread_mutex_init(&shared->lock, NULL));
        assert(0 == pthread_cond_init(&shared->cond, NULL));

        pthread_mutex_lock(&shared->lock);
        shared_locked = 1;

        for (i = 0; i < N_ELEMS(vals); i++) {
                tfoo_task *task = calloc(1, sizeof *task);
                assert(NULL != task);

                task->shared = shared;
                task->task.func = (ttask_func) foo_task;
                task->shared->n_remain++;

                ttask_pool_put(pool, (ttask *) task);
	}

        if (! shared->n_remain) {
                ret = 0;
                goto end;
        }

  retry:
        assert(0 == pthread_cond_wait(&shared->cond, &shared->lock));

        if (shared->n_done != shared->n_remain)
                goto retry;

        if (shared->n_error) {
                ret = 0;
                goto end;
        }

        ret = 0;
  end:

        if (shared) {
                if (shared_locked)
                        pthread_mutex_unlock(&shared->lock);

                pthread_cond_destroy(&shared->cond);
                pthread_mutex_destroy(&shared->lock);

                free(shared);
        }

        if (pool)
          ttask_pool_free(pool);

        return ret;

}
