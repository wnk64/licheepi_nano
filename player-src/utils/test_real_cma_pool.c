#include "cedar_cma_pool.h"
#include <memoryAdapter.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    struct ScMemOpsS *ops = MemAdapterGetOpsS();
    int sizes[33], n = 0;
    void *blocks[33];
    sizes[n++] = 1048576;
    sizes[n++] = 135168;
    sizes[n++] = 49152;
    for (int i = 0; i < 10; i++) {
        sizes[n++] = 327680;
        sizes[n++] = 163840;
    }
    for (int i = 0; i < 8; i++) sizes[n++] = 77824;
    sizes[n++] = 16384;
    sizes[n++] = 12288;
    assert(n == 33);
    assert(!cedar_cma_pool_enable(ops, 7 * 1024 * 1024));
    assert(!ops->open());
    for (int i = 0; i < n; i++) {
        blocks[i] = ops->palloc(sizes[i], NULL, NULL);
        assert(blocks[i]);
        memset(blocks[i], 0, (size_t)sizes[i]);
        ops->flush_cache(blocks[i], sizes[i]);
        uintptr_t virtual_delta = (uintptr_t)blocks[i] - (uintptr_t)blocks[0];
        uintptr_t cpu_delta = (uintptr_t)ops->cpu_get_phyaddr(blocks[i]) -
                              (uintptr_t)ops->cpu_get_phyaddr(blocks[0]);
        uintptr_t ve_delta = (uintptr_t)ops->ve_get_phyaddr(blocks[i]) -
                             (uintptr_t)ops->ve_get_phyaddr(blocks[0]);
        assert(cpu_delta == virtual_delta && ve_delta == virtual_delta);
        assert(ops->cpu_get_viraddr(ops->cpu_get_phyaddr(blocks[i])) == blocks[i]);
    }
    puts("REAL_POOL_READY: 33 suballocations, one contiguous ION arena");
    fflush(stdout);
    sleep(20);
    for (int i = 0; i < n; i++) ops->pfree(blocks[i], NULL, NULL);
    void *reused = ops->palloc(1048576, NULL, NULL);
    assert(reused == blocks[0]);
    ops->pfree(reused, NULL, NULL);
    ops->close();
    puts("REAL_POOL_RELEASED: address/cache/reuse checks passed");
    return 0;
}
