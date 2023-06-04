# NaiveFS

## 注意事项

- PR 前使用 `sh ./scripts/format.sh` 来格式化所有代码
- 合并 PR 时使用 Squash Merge

## 测试

### Linux 环境测试

```bash
sh scripts/test.sh
```

### Docker 环境测试

```bash
sh scripts/docker_test.sh
```

## 垃圾回收策略

使用一个独立的线程来执行垃圾回收。我们在内存中维护一个 `SegmentUsageTable`，存储每个 `Segment` 写入的时间，以及这个 `Segment` 中空闲的字节数。当我们的 FS 启动时，在分配新的 `Segment` 时需要从磁盘头部向后扫描，直到遇到第一个空闲的块。在这个过程中，我们将所有的扫描到的非空的 `Segment` 加入 `SegmentUsageTable`。我们设置一个阈值 `kFreeSegmentsLowerbound` 来表示需要开始执行 GC 的最小的空闲 `Segment` 数，使用 `kFreeSegmentsUpperbound` 来表示剩余多少空闲 `Segments` 后 GC 就可以停止。

当我们需要执行 GC 时，就去 `SegmentUsageTable` 中取写入的时间和空闲的字节数乘积最大的 `kSegmentsMergingBudget` 个 `Segment` 执行合并操作，直到空闲的 `Segment` 数量超过我们设定饿上界。