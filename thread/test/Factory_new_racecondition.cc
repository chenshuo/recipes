// reproduce race condition of Factory.cc if compiled with -DREPRODUCE_BUG

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
      muduo::MutexLockGuard lock(mutex_);
#ifdef REPRODUCE_BUG
      auto it = stocks_.find(stock->key());
      assert(it != stocks_.end());
      if (it->second.expired())
      {
        stocks_.erase(it);
      }
      else
      {
        printf("%s: %s is not expired\n", muduo::CurrentThread::name(), stock->key().c_str());
      }
#else
      auto it = stocks_.find(stock->key());
      if (it == stocks_.end())
      {
        printf("%s: %s had been deleted\n", muduo::CurrentThread::name(), stock->key().c_str());
      }
      else 
      {
        if (it->second.expired())
        {
          stocks_.erase(it);
        }
        else
        {
          printf("%s: %s is not expired\n", muduo::CurrentThread::name(), stock->key().c_str());
        }
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
  printf("%s: stock %p\n", muduo::CurrentThread::name(), stock.get());
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
