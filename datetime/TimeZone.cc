#include "TimeZone.h"

#include <boost/noncopyable.hpp>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

//#define _BSD_SOURCE
#include <endian.h>

#include <stdint.h>
#include <stdio.h>

namespace muduo
{
namespace detail
{

struct Transition
{
  time_t time;
  int localtime;
  Transition(time_t t, int local)
    : time(t), localtime(local)
  { }

  bool operator<(const Transition& rhs) const
  {
    return time < rhs.time;
  }
};

struct Localtime
{
  time_t gmtOffset;
  bool isDst;
  int arrbIdx;

  Localtime(time_t offset, bool dst, int arrb)
    : gmtOffset(offset), isDst(dst), arrbIdx(arrb)
  { }
};

}
}

using namespace muduo;
using namespace std;

struct TimeZone::Data
{
  vector<detail::Transition> transitions;
  //vector<time_t> transitions;
  //vector<uint8_t> localtimeIdxs;
  vector<detail::Localtime> localtimes;
  vector<string> names;
  string abbreviation;
};

namespace muduo
{
namespace detail
{

class File : boost::noncopyable
{
 public:
  File(const char* file)
    : fp_(::fopen(file, "rb"))
  {
  }

  ~File()
  {
    if (fp_)
    {
      ::fclose(fp_);
    }
  }

  bool valid() const { return fp_; }

  string readBytes(int n)
  {
    char buf[n];
    ssize_t nr = ::fread(buf, 1, n, fp_);
    if (nr != n)
      throw logic_error("no enough data");
    return string(buf, n);
  }

  int32_t readInt32()
  {
    int32_t x = 0;
    ssize_t nr = ::fread(&x, 1, sizeof(int32_t), fp_);
    if (nr != sizeof(int32_t))
      throw logic_error("bad int32_t data");
    return be32toh(x);
  }

  uint8_t readUInt8()
  {
    uint8_t x = 0;
    ssize_t nr = ::fread(&x, 1, sizeof(uint8_t), fp_);
    if (nr != sizeof(uint8_t))
      throw logic_error("bad uint8_t data");
    return x;
  }

 private:
  FILE* fp_;
};

bool readTimeZoneFile(const char* zonefile, struct TimeZone::Data* data)
{
  File f(zonefile);
  if (f.valid())
  {
    try
    {
      string head = f.readBytes(4);
      if (head != "TZif")
        throw logic_error("bad head");
      string version = f.readBytes(1);
      f.readBytes(15);

      int32_t isgmtcnt = f.readInt32();
      int32_t isstdcnt = f.readInt32();
      int32_t leapcnt = f.readInt32();
      int32_t timecnt = f.readInt32();
      int32_t typecnt = f.readInt32();
      int32_t charcnt = f.readInt32();

      vector<int32_t> trans;
      trans.reserve(timecnt);
      for (int i = 0; i < timecnt; ++i)
      {
        trans.push_back(f.readInt32());
      }

      for (int i = 0; i < timecnt; ++i)
      {
        uint8_t local = f.readUInt8();
        data->transitions.push_back(Transition(trans[i], local));
      }

      for (int i = 0; i < typecnt; ++i)
      {
        int32_t gmtoff = f.readInt32();
        uint8_t isdst = f.readUInt8();
        uint8_t abbrind = f.readUInt8();

        data->localtimes.push_back(Localtime(gmtoff, isdst, abbrind));
      }

      data->abbreviation = f.readBytes(charcnt);

      // leapcnt
      for (int i = 0; i < leapcnt; ++i)
      {
        int32_t leaptime = f.readInt32();
        int32_t cumleap = f.readInt32();
      }
      // isstdcnt
      // isgmtcnt
    }
    catch (logic_error& e)
    {
      fprintf(stderr, "%s\n", e.what());
    }
  }
  return true;
}
}
}


TimeZone::TimeZone(const char* zonefile)
  : data_(new TimeZone::Data)
{
  if (!detail::readTimeZoneFile(zonefile, data_.get()))
  {
    data_.reset();
  }
}

struct tm TimeZone::toLocalTime(time_t seconds) const
{
  struct tm localTime = { 0, };
  assert(data_ != NULL);
  const Data& data(*data_);
  const detail::Localtime* local = NULL;
  if (data.transitions.empty() || seconds < data.transitions.front().time)
  {
    local = &data.localtimes.front();
  }
  else
  {
    vector<detail::Transition>::const_iterator transI
      = lower_bound(data.transitions.begin(), data.transitions.end(), detail::Transition(seconds, 0));
    if (transI != data.transitions.end())
    {
      if (transI->time != seconds)
      {
        assert(transI != data.transitions.begin());
        --transI;
      }
      local = &data.localtimes[transI->localtime];
    }
    else
    {
      // FIXME: use TZ-env
      local = &data.localtimes.back();
    }
    if (local)
    {
      time_t localSeconds = seconds + local->gmtOffset;
      ::gmtime_r(&localSeconds, &localTime);
      localTime.tm_isdst = local->isDst;
      localTime.tm_gmtoff = local->gmtOffset;
      localTime.tm_zone = &data.abbreviation[local->arrbIdx];
    }
  }

  return localTime;
}

time_t TimeZone::fromLocalTime(const struct tm&) const
{
  assert(data_ != NULL);
  return 0;
}


