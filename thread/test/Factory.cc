#include <map>

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "../Mutex.h"

#include <assert.h>
#include <stdio.h>

using std::string;

class Stock : boost::noncopyable
{
 public:
  Stock(const string& name)
    : name_(name)
  {
    printf(" Stock[%p] %s\n", this, name_.c_str());
  }

  ~Stock()
  {
    printf("~Stock[%p] %s\n", this, name_.c_str());
  }

  const string& key() const { return name_; }

 private:
  string name_;
};

namespace version1
{

// questionable code
class StockFactory : boost::noncopyable
{
 public:

  boost::shared_ptr<Stock> get(const string& key)
  {
    muduo::MutexLockGuard lock(mutex_);
    boost::shared_ptr<Stock>& pStock = stocks_[key];
    if (!pStock)
    {
      pStock.reset(new Stock(key));
    }
    return pStock;
  }


 private:
  mutable muduo::MutexLock mutex_;
  std::map<string, boost::shared_ptr<Stock> > stocks_;
};

}

namespace version2
{

class StockFactory : boost::noncopyable
{
 public:
  boost::shared_ptr<Stock> get(const string& key)
  {
    boost::shared_ptr<Stock> pStock;
    muduo::MutexLockGuard lock(mutex_);
    boost::weak_ptr<Stock>& wkStock = stocks_[key];
    pStock = wkStock.lock();
    if (!pStock)
    {
      pStock.reset(new Stock(key));
      wkStock = pStock;
    }
    return pStock;
  }

 private:
  mutable muduo::MutexLock mutex_;
  std::map<string, boost::weak_ptr<Stock> > stocks_;
};

}

namespace version3
{

class StockFactory : boost::noncopyable
{
 public:

  boost::shared_ptr<Stock> get(const string& key)
  {
    boost::shared_ptr<Stock> pStock;
    muduo::MutexLockGuard lock(mutex_);
    boost::weak_ptr<Stock>& wkStock = stocks_[key];
    pStock = wkStock.lock();
    if (!pStock)
    {
      pStock.reset(new Stock(key),
                   boost::bind(&StockFactory::deleteStock, this, _1));
      wkStock = pStock;
    }
    return pStock;
  }

 private:

  void deleteStock(Stock* stock)
  {
    printf("deleteStock[%p]\n", stock);
    if (stock)
    {
      muduo::MutexLockGuard lock(mutex_);
      stocks_.erase(stock->key());  // This is wrong, see removeStock below for correct implementation.
    }
    delete stock;  // sorry, I lied
  }
  mutable muduo::MutexLock mutex_;
  std::map<string, boost::weak_ptr<Stock> > stocks_;
};

}

namespace version4
{

class StockFactory : public boost::enable_shared_from_this<StockFactory>,
                     boost::noncopyable
{
 public:

  boost::shared_ptr<Stock> get(const string& key)
  {
    boost::shared_ptr<Stock> pStock;
    muduo::MutexLockGuard lock(mutex_);
    boost::weak_ptr<Stock>& wkStock = stocks_[key];
    pStock = wkStock.lock();
    if (!pStock)
    {
      pStock.reset(new Stock(key),
                   boost::bind(&StockFactory::deleteStock,
                               shared_from_this(),
                               _1));
      wkStock = pStock;
    }
    return pStock;
  }

 private:

  void deleteStock(Stock* stock)
  {
    printf("deleteStock[%p]\n", stock);
    if (stock)
    {
      muduo::MutexLockGuard lock(mutex_);
      stocks_.erase(stock->key());  // This is wrong, see removeStock below for correct implementation.
    }
    delete stock;  // sorry, I lied
  }
  mutable muduo::MutexLock mutex_;
  std::map<string, boost::weak_ptr<Stock> > stocks_;
};

}

class StockFactory : public boost::enable_shared_from_this<StockFactory>,
                     boost::noncopyable
{
 public:
  boost::shared_ptr<Stock> get(const string& key)
  {
    boost::shared_ptr<Stock> pStock;
    muduo::MutexLockGuard lock(mutex_);
    boost::weak_ptr<Stock>& wkStock = stocks_[key];
    pStock = wkStock.lock();
    if (!pStock)
    {
      pStock.reset(new Stock(key),
                   boost::bind(&StockFactory::weakDeleteCallback,
                               boost::weak_ptr<StockFactory>(shared_from_this()),
                               _1));
      wkStock = pStock;
    }
    return pStock;
  }

 private:
  static void weakDeleteCallback(const boost::weak_ptr<StockFactory>& wkFactory,
                                 Stock* stock)
  {
    printf("weakDeleteStock[%p]\n", stock);
    boost::shared_ptr<StockFactory> factory(wkFactory.lock());
    if (factory)
    {
      factory->removeStock(stock);
    }
    else
    {
      printf("factory died.\n");
    }
    delete stock;  // sorry, I lied
  }

  void removeStock(Stock* stock)
  {
    if (stock)
    {
      muduo::MutexLockGuard lock(mutex_);
      auto it = stocks_.find(stock->key());
      assert(it != stocks_.end());
      if (it->second.expired())
      {
        stocks_.erase(stock->key());
      }
    }
  }

 private:
  mutable muduo::MutexLock mutex_;
  std::map<string, boost::weak_ptr<Stock> > stocks_;
};

void testLongLifeFactory()
{
  boost::shared_ptr<StockFactory> factory(new StockFactory);
  {
    boost::shared_ptr<Stock> stock = factory->get("NYSE:IBM");
    boost::shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
    assert(stock == stock2);
    // stock destructs here
  }
  // factory destructs here
}

void testShortLifeFactory()
{
  boost::shared_ptr<Stock> stock;
  {
    boost::shared_ptr<StockFactory> factory(new StockFactory);
    stock = factory->get("NYSE:IBM");
    boost::shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
    assert(stock == stock2);
    // factory destructs here
  }
  // stock destructs here
}

int main()
{
  version1::StockFactory sf1;
  version2::StockFactory sf2;
  version3::StockFactory sf3;
  boost::shared_ptr<version3::StockFactory> sf4(new version3::StockFactory);
  boost::shared_ptr<StockFactory> sf5(new StockFactory);

  {
  boost::shared_ptr<Stock> s1 = sf1.get("stock1");
  }

  {
  boost::shared_ptr<Stock> s2 = sf2.get("stock2");
  }

  {
  boost::shared_ptr<Stock> s3 = sf3.get("stock3");
  }

  {
  boost::shared_ptr<Stock> s4 = sf4->get("stock4");
  }

  {
  boost::shared_ptr<Stock> s5 = sf5->get("stock5");
  }

  testLongLifeFactory();
  testShortLifeFactory();
}
