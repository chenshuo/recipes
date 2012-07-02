CXXFLAGS=-g -Wall -rdynamic -march=native
CXXFLAGS+=-O2

HEADERS=$(wildcard *.h)
TESTS= atomic_unittest \
       blockingqueue_test \
       boundedblockingqueue_test \
       exception_test \
       fork_test \
       singleton_test \
       singleton_threadlocal_test \
       threadlocal_test \
       threadlocalsingleton_test \
       thread_test \
       threadpool_test \
       exit_deadlock \
       mutual_deadlock \
       self_deadlock

all: $(TESTS)

$(TESTS): $(HEADERS)

$(TESTS):
	g++ $(CXXFLAGS) -o $@ $(filter %.cc,$^) -lpthread

atomic_unittest: test/Atomic_unittest.cc

blockingqueue_test: test/BlockingQueue_test.cc Thread.cc

boundedblockingqueue_test: test/BoundedBlockingQueue_test.cc Thread.cc

exit_deadlock: test/ExitDeadLock.cc Thread.cc

mutual_deadlock: test/MutualDeadLock.cc Thread.cc

self_deadlock: test/SelfDeadLock.cc Thread.cc

exception_test: test/Exception_test.cc Exception.cc

fork_test: test/Fork_test.cc Thread.cc

singleton_test: test/Singleton_test.cc Thread.cc
singleton_test: CXXFLAGS += -ftemplate-depth-2500

singleton_threadlocal_test: test/SingletonThreadLocal_test.cc Thread.cc

thread_test: test/Thread_test.cc Thread.cc

threadlocal_test: test/ThreadLocal_test.cc Thread.cc

threadlocalsingleton_test: test/ThreadLocalSingleton_test.cc Thread.cc

threadpool_test: test/ThreadPool_test.cc Thread.cc ThreadPool.cc Exception.cc

clean:
	rm -f $(TESTS)
