eina_bench_src = files(
'eina_bench.c',
'eina_bench_sort.c',
'eina_bench_hash.c',
'eina_bench_crc_hash.c',
'eina_bench_stringshare.c',
'eina_bench_convert.c',
'eina_bench_mempool.c',
'eina_bench_stringshare_e17.c',
'eina_bench_array.c',
'eina_bench_rectangle_pool.c',
'ecore_list.c',
'ecore_strings.c',
'ecore_hash.c',
'ecore_sheap.c',
'evas_hash.c',
'evas_list.c',
'evas_mempool.c',
'evas_object_list.c',
'evas_stringshare.c',
'eina_bench_quad.c',
'eina_bench.h',
'Ecore_Data.h',
'Evas_Data.h',
'evas_mempool.h')

city = shared_library('city',
    sources : ['city.cc','city.h'],
)

eina_bench = executable('eina_bench',
	sources : eina_bench_src,
	dependencies : eina,
    c_args : ['-fPIC','-DCITYHASH_BENCH', '-DEINA_ENABLE_BENCH_E17'],
	include_directories : config_dir,
	link_with : city,
)

benchmark('eina_bench', eina_bench,
  args : 'libcity.la',
)
