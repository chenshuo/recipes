g++ LogStream_test.cc LogStream.cc -o test -Wall -g -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework
g++ LogStream_bench.cc LogStream.cc ../datetime/Timestamp.cc -o bench -Wall -g -O2
g++ Logging_test.cc Logging.cc LogStream.cc ../datetime/Timestamp.cc ../thread/Thread.cc -O2 -I.. -o test_log -Wall -g -lpthread
