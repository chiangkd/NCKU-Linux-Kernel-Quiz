set title "Performance"
set xlabel "Buffer Size"
set ylabel "Ticks"
set terminal png font "Times_New_Roman, 12"
set output "performance.png"

plot \
"performance.txt" using 2:xtic(1) with linespoints linewidth 2 title "without swar", \
"performance.txt" using 3:xtic(1) with linespoints linewidth 2 title "with swar"