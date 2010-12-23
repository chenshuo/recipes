CXXFLAGS=-g -Wall -rdynamic -march=native
CXXFLAGS+=-O2

HEADERS=$(wildcard *.h)
TESTS= atomic_unittest \
       blockingqueue_test \
       exception_test \
       fork_test \
       singleton_test \
       singleton_threadlocal_test \
       threadlocal_test \
       threadlocalsingleton_test \
       thread_test \
       threadpool_test

all: $(TESTS)

$(TESTS): $(HEADERS)

$(TESTS):
	g++ $(CXXFLAGS) -o $@ $(filter %.cc,$^) -lpthread

atomic_unittest: test/Atomic_unittest.cc

blockingqueue_test: test/BlockingQueue_test.cc CountDownLatch.cc Thread.cc

exception_test: test/Exception_test.cc Exception.cc

fork_test: test/Fork_test.cc Thread.cc

singleton_test: test/Singleton_test.cc Thread.cc

singleton_threadlocal_test: test/SingletonThreadLocal_test.cc Thread.cc

thread_test: test/Thread_test.cc Thread.cc

threadlocal_test: test/ThreadLocal_test.cc Thread.cc

threadlocalsingleton_test: test/ThreadLocalSingleton_test.cc Thread.cc

threadpool_test: test/ThreadPool_test.cc Thread.cc ThreadPool.cc CountDownLatch.cc Exception.cc

clean:
	rm -f $(TESTS)
