g++ LogStream_test.cc LogStream.cc -o test -Wall -g -O0 -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework

g++ LogStream_bench.cc LogStream.cc ../datetime/Timestamp.cc -o bench -Wall -g -O2
g++ LogStream_bench.cc LogStream.cc ../datetime/Timestamp.cc -o bench-dbg -Wall -g -O0

g++ Logging_test.cc Logging.cc LogStream.cc LogFile.cc ../datetime/Timestamp.cc ../thread/Thread.cc -O2 -DNDEBUG -I.. -o test_log -Wall -Wextra -Wno-type-limits -g -lpthread
g++ Logging_test.cc Logging.cc LogStream.cc LogFile.cc ../datetime/Timestamp.cc ../thread/Thread.cc -O0 -I.. -o test_log-dbg -Wall -Wextra -Wno-type-limits -g -lpthread

g++ LogFile_test.cc Logging.cc LogStream.cc LogFile.cc ../datetime/Timestamp.cc ../thread/Thread.cc -O2 -DNDEBUG -I.. -o file_test -g -lpthread
g++ LogFile_test.cc Logging.cc LogStream.cc LogFile.cc ../datetime/Timestamp.cc ../thread/Thread.cc -O0 -I.. -o file_test-dbg -g -lpthread

g++ AsyncLogging_test.cc Logging.cc LogStream.cc LogFile.cc ../datetime/Timestamp.cc ../thread/Thread.cc -O2 -DNDEBUG -I.. -o async_test -g -lpthread -lrt
g++ AsyncLogging_test.cc Logging.cc LogStream.cc LogFile.cc ../datetime/Timestamp.cc ../thread/Thread.cc -O0 -I.. -o async_test-dbg -g -lpthread -lrt
