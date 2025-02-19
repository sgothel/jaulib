POPT="-v normal --benchmark-samples 400"

./test_functional1_perf $POPT 2>&1 | tee f1_perf.txt
./test_functional2_perf $POPT 2>&1 | tee f2_perf.txt
# ./test_functional3_perf $POPT 2>&1 | tee f3_perf.txt
