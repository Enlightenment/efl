#ifndef EINA_BENCH_H_
#define EINA_BENCH_H_

typedef struct _Eina_Bench Eina_Bench;
typedef void (*Eina_Bench_Specimens)(int request);
#define EINA_BENCH(Function) ((Eina_Bench_Specimens)Function)

void eina_bench_register(Eina_Bench *bench, const char *name, Eina_Bench_Specimens bench_cb,
			 int count_start, int count_end, int count_set);

void eina_bench_hash(Eina_Bench *bench);

#endif
