set   autoscale                        # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set terminal png size 1024,768
set output "output_Convert_2008-08-29_0000.png"
set title "Convert 2008-08-29_0000
set xlabel "tests"
set ylabel "time"
plot "bench_Convert_2008-08-29_0000.itoa 10.data" using 1:2 title 'itoa 10' with line, \
"bench_Convert_2008-08-29_0000.itoa 16.data" using 1:2 title 'itoa 16' with line, \
"bench_Convert_2008-08-29_0000.snprintf 10.data" using 1:2 title 'snprintf 10' with line, \
"bench_Convert_2008-08-29_0000.snprintf 16.data" using 1:2 title 'snprintf 16' with line, \
"bench_Convert_2008-08-29_0000.snprintf a.data" using 1:2 title 'snprintf a' with line, \
"bench_Convert_2008-08-29_0000.dtoa.data" using 1:2 title 'dtoa' with line, \
"bench_Convert_2008-08-29_0000.gstrtod.data" using 1:2 title 'gstrtod' with line
