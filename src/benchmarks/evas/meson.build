evas_bench_src = files(
  'evas_bench.c',
  'evas_bench.h',
  'evas_bench_loader.c',
  'evas_bench_saver.c',
  'evas_bench_textgrid.c',
)

evas_bench = executable('evas_bench',
  evas_bench_src,
  dependencies : [evas, eina, m],
  c_args : ['-DEFL_BUILD',
            '-DTESTS_SRC_DIR="'+join_paths(meson.source_root(), 'src', 'tests', 'evas')+'"'],
  include_directories : config_dir,
)

benchmark('evas_bench', evas_bench,
  timeout : 600,
)
