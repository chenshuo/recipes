g++ LogStream_test.cc LogStream.cc -o test -Wall -g -O0 -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework
g++ LogStream_bench.cc LogStream.cc ../datetime/Timestamp.cc -o bench -Wall -g -O2
g++ Logging_test.cc Logging.cc LogStream.cc LogFile.cc ../datetime/Timestamp.cc ../thread/Thread.cc -O2 -DNDEBUG -I.. -o test_log -Wall -Wextra -Wno-type-limits -g -lpthread
g++ LogFile_test.cc LogFile.cc Logging.cc LogStream.cc ../datetime/Timestamp.cc ../thread/Thread.cc -O2 -DNDEBUG -I.. -o file_test -g -lpthread
g++ AsyncLogging_test.cc LogFile.cc LogStream.cc ../thread/Thread.cc -O2 -I.. -o async_test -g -lpthread -lrt
