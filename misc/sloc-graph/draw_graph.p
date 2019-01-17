# Gnuplot script file
# Usage: gnuplot> load 'draw_graph.p'
set title 'Chester SLOC Line Graph'
set nokey
set xdata time
set timefmt "%Y-%m-%d"
set xrange ["2017-09-01":"2019-06-01"]
set yrange [0:10000]
set format x "%b"
plot "xdate_ysloc.dat" using 1:2 with lines
