#include <map>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "../Mutex.h"

using std::string;

class CustomerData : boost::noncopyable
{
 public:
  CustomerData()
    : data_(new Map)
  { }

  int query(const string& customer, const string& stock) const;

 private:
  typedef std::pair<string, int> Entry;
  typedef std::vector<Entry> EntryList;
  typedef std::map<string, EntryList> Map;
  typedef boost::shared_ptr<Map> MapPtr;
  void update(const string& customer, const EntryList& entries);
  void update(const string& message);

  static int findEntry(const EntryList& entries, const string& stock);
  static MapPtr parseData(const string& message);

  MapPtr getData() const
  {
    muduo::MutexLockGuard lock(mutex_);
    return data_;
  }

  mutable muduo::MutexLock mutex_;
  MapPtr data_;
};

int CustomerData::query(const string& customer, const string& stock) const
{
  MapPtr data = getData();

  Map::const_iterator entries = data->find(customer);
  if (entries != data->end())
    return findEntry(entries->second, stock);
  else
    return -1;
}

void CustomerData::update(const string& customer, const EntryList& entries)
{
  muduo::MutexLockGuard lock(mutex_);
  if (!data_.unique())
  {
    MapPtr newData(new Map(*data_));
    data_.swap(newData);
  }
  assert(data_.unique());
  (*data_)[customer] = entries;
}

void CustomerData::update(const string& message)
{
  MapPtr newData = parseData(message);
  if (newData)
  {
    muduo::MutexLockGuard lock(mutex_);
    data_.swap(newData);
  }
}

int main()
{
  CustomerData data;
}
