#ifndef EINA_BENCH_H_
#define EINA_BENCH_H_

void eo_bench_eo_do(Eina_Benchmark *bench);
void eo_bench_eo_add(Eina_Benchmark *bench);

#define _EO_BENCH_TIMES(Start, Repeat, Jump) (Start), ((Start) + ((Jump) * (Repeat))), (Jump)

#endif
