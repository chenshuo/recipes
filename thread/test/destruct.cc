#include "../Mutex.h"
#include <string>

using std::string;

muduo::MutexLock g_mutex;
string g_str = "Hello";
int32_t g_int32 = 123;
int64_t g_int64 = 4321;

string getString()
{
  muduo::MutexLockGuard lock(g_mutex);
  return g_str;
}

int32_t getInt32()
{
  muduo::MutexLockGuard lock(g_mutex);
  return g_int32;
}

int64_t getInt64()
{
  muduo::MutexLockGuard lock(g_mutex);
  return g_int64;
}

int main()
{
  getString();
  getInt32();
  getInt64();
}
