#include "cedar_cma_pool.h"
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

static int allocs, frees, opens, closes, fail_open, fail_alloc;
static int native_open(void) { opens++; return fail_open ? -1 : 0; }
static void native_close(void) { closes++; }
static void *native_alloc(int size, void *a, void *b)
{
    void *p = NULL;
    (void)a; (void)b;
    allocs++;
    if (fail_alloc) { fail_alloc = 0; return NULL; }
    assert(!posix_memalign(&p, 4096, (size_t)size));
    return p;
}
static void native_free(void *p, void *a, void *b)
{
    (void)a; (void)b;
    frees++;
    free(p);
}
static void *native_phy(void *p) { return p; }
static struct ScMemOpsS ops = {
    .open = native_open, .close = native_close,
    .palloc = native_alloc, .pfree = native_free,
    .cpu_get_phyaddr = native_phy
};
static void reset(void)
{
    assert(ops.open == native_open && ops.palloc == native_alloc);
    allocs = frees = opens = closes = fail_open = fail_alloc = 0;
}
static void *worker(void *unused)
{
    (void)unused;
    for (int i = 0; i < 500; i++) {
        void *p = ops.palloc(113, NULL, NULL);
        assert(p && !((uintptr_t)p % 4096));
        ops.pfree(p, NULL, NULL);
    }
    return NULL;
}
int main(void)
{
    void *a, *b, *c, *d;
    assert(cedar_cma_pool_enable(NULL, 4096) == -1);
    assert(cedar_cma_pool_enable(&ops, 1) == -1);
    assert(cedar_cma_pool_enable(&ops, 17 * 1024 * 1024) == -1);
    assert(!cedar_cma_pool_enable(&ops, 3 * 4096));
    assert(cedar_cma_pool_enable(&ops, 4096) == -1);
    assert(!ops.open() && !ops.open());
    assert(ops.cpu_get_phyaddr == native_phy);
    assert(!ops.palloc(0, NULL, NULL));
    assert(!ops.palloc(-1, NULL, NULL));
    a = ops.palloc(1, NULL, NULL);
    b = ops.palloc(4097, NULL, NULL);
    assert((uintptr_t)b - (uintptr_t)a == 4096 && allocs == 1);
    c = ops.palloc(1, NULL, NULL);
    assert(c && allocs == 2);
    ops.pfree(c, NULL, NULL);
    ops.pfree(a, NULL, NULL);
    d = ops.palloc(4096, NULL, NULL);
    assert(d == a && allocs == 2);
    ops.pfree(d, NULL, NULL);
    ops.pfree(d, NULL, NULL); /* Must not free the entire parent allocation. */
    ops.pfree((unsigned char *)b + 1, NULL, NULL);
    assert(frees == 1);
    ops.pfree(b, NULL, NULL);
    ops.close();
    assert(frees == 1);
    ops.close();
    assert(frees == 2 && opens == 2 && closes == 2);
    reset();

    assert(!cedar_cma_pool_enable(&ops, 4096));
    fail_open = 1;
    assert(ops.open() == -1 && ops.open == native_open);
    reset();

    assert(!cedar_cma_pool_enable(&ops, 4096));
    assert(!ops.open());
    fail_alloc = 1;
    a = ops.palloc(1, NULL, NULL);
    assert(a && allocs == 2);
    ops.pfree(a, NULL, NULL);
    ops.close();
    assert(frees == 1);
    reset();

    assert(!cedar_cma_pool_enable(&ops, 4096));
    assert(!ops.open());
    a = ops.palloc(1, NULL, NULL);
    b = ops.palloc(1, (void *)1, NULL);
    assert(allocs == 2); /* Distinct VE mapping context must not share arena. */
    ops.pfree(b, (void *)1, NULL);
    ops.pfree(a, NULL, NULL);
    ops.close();
    assert(frees == 2);
    reset();

    assert(!cedar_cma_pool_enable(&ops, 4096 * 128));
    assert(!ops.open());
    pthread_t threads[4];
    for (int i = 0; i < 4; i++) assert(!pthread_create(&threads[i], NULL, worker, NULL));
    for (int i = 0; i < 4; i++) assert(!pthread_join(threads[i], NULL));
    assert(allocs == 1);
    ops.close();
    assert(frees == 1);
    reset();
    puts("CMA pool tests passed");
    return 0;
}
