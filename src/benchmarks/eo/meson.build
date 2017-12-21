eo_benchmark_src = [
  'class_simple.c',
  'class_simple.h',
  'eo_bench.c',
  'eo_bench.h',
  'eo_bench_callbacks.c',
  'eo_bench_eo_do.c',
  'eo_bench_eo_add.c'
]

eo_bench = executable('eo_bench',
  eo_benchmark_src,
  dependencies: [eo, eina],
)

benchmark('eo', eo_bench)
