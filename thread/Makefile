CXXFLAGS=-g -Wall -rdynamic -march=native
CXXFLAGS+=-O2

TESTS= atomic_unittest \
       exception_test \
       blockingqueue_test \
       singleton_test \
       singleton_threadlocal_test \
       threadlocal_test \
       threadlocalsingleton_test \
       thread_test \
       threadpool_test

all: $(TESTS)

atomic_unittest: test/Atomic_unittest.cc
	g++ $(CXXFLAGS) -o $@ $^

blockingqueue_test: test/BlockingQueue_test.cc CountDownLatch.cc Thread.cc
	g++ $(CXXFLAGS) -o $@ $^ -lpthread

exception_test: test/Exception_test.cc Exception.cc
	g++ $(CXXFLAGS) -o $@ $^

singleton_test: test/Singleton_test.cc Thread.cc
	g++ $(CXXFLAGS) -o $@ $^ -lpthread

singleton_threadlocal_test: test/SingletonThreadLocal_test.cc Thread.cc
	g++ $(CXXFLAGS) -o $@ $^ -lpthread

thread_test: test/Thread_test.cc Thread.cc
	g++ $(CXXFLAGS) -o $@ $^ -lpthread

threadlocal_test: test/ThreadLocal_test.cc Thread.cc
	g++ $(CXXFLAGS) -o $@ $^ -lpthread

threadlocalsingleton_test: test/ThreadLocalSingleton_test.cc Thread.cc
	g++ $(CXXFLAGS) -o $@ $^ -lpthread

threadpool_test: test/ThreadPool_test.cc Thread.cc ThreadPool.cc CountDownLatch.cc Exception.cc
	g++ $(CXXFLAGS) -o $@ $^ -lpthread

clean:
	rm -f $(TESTS)
