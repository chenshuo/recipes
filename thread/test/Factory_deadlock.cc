// reproduce dead lock if compiled with -DREPRODUCE_BUG

#include "../Mutex.h"

#include <boost/noncopyable.hpp>

#include <memory>
#include <unordered_map>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using std::string;

void sleepMs(int ms)
{
  usleep(ms * 1000);
}

class Stock : boost::noncopyable
{
 public:
  Stock(const string& name)
    : name_(name)
  {
    printf("%s: Stock[%p] %s\n", muduo::CurrentThread::name(), this, name_.c_str());
  }

  ~Stock()
  {
    printf("%s: ~Stock[%p] %s\n", muduo::CurrentThread::name(), this, name_.c_str());
  }

  const string& key() const { return name_; }

 private:
  string name_;
};


class StockFactory : boost::noncopyable
{
 public:

  std::shared_ptr<Stock> get(const string& key)
  {
    std::shared_ptr<Stock> pStock;
    muduo::MutexLockGuard lock(mutex_);
    std::weak_ptr<Stock>& wkStock = stocks_[key];
    pStock = wkStock.lock();
    if (!pStock)
    {
      pStock.reset(new Stock(key),
                   [this] (Stock* stock) { deleteStock(stock); });
      wkStock = pStock;
    }
    return pStock;
  }

 private:

  void deleteStock(Stock* stock)
  {
    printf("%s: deleteStock[%p]\n", muduo::CurrentThread::name(), stock);
    if (stock)
    {
      sleepMs(500);
      if (mutex_.isLockedByThisThread())
      {
        printf("WARNING: mutex_ is already locked by this thread, deadlock will happen.\n");
      }
      muduo::MutexLockGuard lock(mutex_);
      auto it = stocks_.find(stock->key());
      assert(it != stocks_.end());
#ifdef REPRODUCE_BUG
      if (auto x = it->second.lock())
      {
        assert(stock != x.get());
        printf("use_count = %ld\n", x.use_count());
        sleepMs(500);
        printf("use_count = %ld\n", x.use_count());
      }
      else
      {
        stocks_.erase(it);
      }
#else
      if (it->second.expired())
      {
        stocks_.erase(it);
      }
      else
      {
        printf("%s: %s is not expired\n", muduo::CurrentThread::name(), stock->key().c_str());
      }
#endif
    }
    delete stock;  // sorry, I lied
  }

  mutable muduo::MutexLock mutex_;
  std::unordered_map<string, std::weak_ptr<Stock> > stocks_;
};

void threadB(StockFactory* factory)
{
  sleepMs(250);
  auto stock = factory->get("MS");
  printf("%s: stockB %p\n", muduo::CurrentThread::name(), stock.get());

  sleepMs(500);
  printf("%s: stockB destructs\n", muduo::CurrentThread::name());
}

int main()
{
  StockFactory factory;
  muduo::Thread thr([&factory] { threadB(&factory); }, "thrB");
  thr.start();
  {
  auto stock = factory.get("MS");
  printf("%s: stock %p\n", muduo::CurrentThread::name(), stock.get());
  }
  thr.join();
}
