set   autoscale                        # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set logscale y
set terminal png size 1024,768
set output "output_Stringshare_2008-08-08_17:14:32.png"
set title "Stringshare 2008-08-08_17:14:32 (superfast vs djb2)"
set xlabel "tests"
set ylabel "time"
plot "bench_Stringshare_2008-08-08_17:14:32.stringshare.data" using 1:2 title 'stringshare-superfast' with line, \
 "bench_Stringshare_2008-08-08_17:09:23.stringshare.data" using 1:2 title 'stringshare-djb2' with line

# "bench_Stringshare_2008-08-08_17:09:23.stringshare.data" using 1:(2*abs($5-$2)/(abs($5)+abs($2))) title "relative error" with line


