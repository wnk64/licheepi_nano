# 播放器 CMA 集中分配调查

状态：源码基线复现未通过，暂停功能修改和部署。正在运行的投屏未停止。

原工作树：/home/wnk/f1c200s_display_480x800_candidate_20260914，HEAD0569026。
板端及工作树现存播放器MD5均11386abeb2137bfb3f28c47832b05552。
完整现存源码、Git、对象文件、二进制及工作差异已保存到/home/wnk/F1C200S_archives/player_before_cma_pool_20260930。

## 当前真实占用

板端CMA bitmap used2845页=11.113MiB，count6144页=24MiB，未分配3299页=12.887MiB，maxchunk2648页=10.344MiB。ION解码分配共33对象、6799360字节=6.484MiB，无orphaned allocations；显示占用另有fbcon3072000字节、播放器462848字节。不能把meminfo的CmaFree当成CMA未分配额度。

## 集中分配方向和边界

Cedar的ScMemOpsS提供palloc/pfree及物理地址转换。现有ION实现支持分配块内部地址的虚实转换和缓存范围操作，因此可研究播放器私有的连续arena：一次ION/CMA申请大块，在内部按页对齐管理子分配，保留原参考帧、VBV和硬件旋转策略，不修改共享库/内核。需要验证对应板端libMemAdapter与所查源码确实一致，测试释放、复用、耗尽回退、地址转换、缓存同步和解码动态分辨率。未实现或验证该方向。
固定arena会预留余量，可能增加总占用；集中分配不等于降低RAM。不能直接削减H.264参考帧或承诺更大可用CMA。当前低内存config已有VBV1MiB、额外保持帧为0，进一步削减并非无风险。

## 基线复现发现

完整执行原Makefile的make -B -j4及make strip，产物dd4ba5862cf854f3bb5614dda5d58219，与运行版本不一致。逐对象cmp确认：只有main.o不同，其余五个.o逐字节一致。
旧main.o定义g_drm_warpper大小0x550，当前头文件编译为0x558。历史提交3993c9b在drm_warpper_t中插入display_width/display_height两个int，而main.o没有随头文件更新。Makefile没有头文件依赖，因此形成混合对象的旧二进制。反汇编差异主要为后续BSS地址偏移，不能用此现象宣称完整源码复现通过或当前播放器毫无风险。

复现检查生成的全部对象和二进制已恢复为检查前原样，源码及既有dirty变化均保留。未部署任何新播放器，不改AIC、sink、FIFO、BS网络、内核或DTB。根据变更控制规则，需先明确完整构建一致性基线及其验证，再实现连续内存池；不能在未复现基线上叠加功能候选。
