CXXFLAGS=-g -Wall -rdynamic -march=native
CXXFLAGS+=-O2

HEADERS=$(wildcard *.h)
TESTS= atomic_unittest \
       blockingqueue_test \
       boundedblockingqueue_test \
       copyonwrite_test \
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
       nonrecursivemutex_test \
       requestinventory_test \
       requestinventory_test2 \
       self_deadlock \
       weakcallback_test \
       signalslot_test \
       waiter_test

all: $(TESTS)

$(TESTS): $(HEADERS)

$(TESTS):
	g++ $(CXXFLAGS) -o $@ $(filter %.cc,$^) $(LDFLAGS) -lpthread

atomic_unittest: test/Atomic_unittest.cc

blockingqueue_test: test/BlockingQueue_test.cc Thread.cc

boundedblockingqueue_test: test/BoundedBlockingQueue_test.cc Thread.cc

copyonwrite_test: test/CopyOnWrite_test.cc Thread.cc

exit_deadlock: test/ExitDeadLock.cc Thread.cc

mutual_deadlock: test/MutualDeadLock.cc Thread.cc

nonrecursivemutex_test: test/NonRecursiveMutex_test.cc Thread.cc

requestinventory_test: test/RequestInventory_test.cc Thread.cc
requestinventory_test2: test/RequestInventory_test2.cc Thread.cc

self_deadlock: test/SelfDeadLock.cc Thread.cc

exception_test: test/Exception_test.cc Exception.cc

fork_test: test/Fork_test.cc Thread.cc

singleton_test: test/Singleton_test.cc Thread.cc
singleton_test: CXXFLAGS += -ftemplate-depth-1000

singleton_threadlocal_test: test/SingletonThreadLocal_test.cc Thread.cc

thread_test: test/Thread_test.cc Thread.cc

threadlocal_test: test/ThreadLocal_test.cc Thread.cc

threadlocalsingleton_test: test/ThreadLocalSingleton_test.cc Thread.cc

threadpool_test: test/ThreadPool_test.cc Thread.cc ThreadPool.cc Exception.cc

weakcallback_test: test/WeakCallback_test.cc
weakcallback_test: CXXFLAGS += -Wextra --std=c++0x -DBOOST_TEST_DYN_LINK
weakcallback_test: LDFLAGS += -lboost_unit_test_framework

signalslot_test: test/SignalSlot_test.cc Thread.cc
signalslot_test: CXXFLAGS += -Wextra --std=c++0x -DBOOST_TEST_DYN_LINK
signalslot_test: LDFLAGS += -lboost_unit_test_framework

waiter_test: test/Waiter_test.cc Thread.cc
waiter_test: CXXFLAGS += --std=c++0x

clean:
	rm -f $(TESTS)
