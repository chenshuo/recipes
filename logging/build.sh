g++ LogStream_test.cc LogStream.cc -o test -Wall -g -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework
g++ LogStream_bench.cc LogStream.cc ../datetime/Timestamp.cc -o bench -Wall -g -O2
