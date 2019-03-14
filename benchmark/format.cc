#include "format.h"

std::string formatSI(size_t s)
{
  double n = s;
  char buf[64] = "";
  if (s < 100000)
    snprintf(buf, sizeof(buf), "%zd", s);
  else if (n < 1e7) // FIXME: - 1e3/2
    snprintf(buf, sizeof(buf), "%.0fk", n/1e3);
  else if (n < 1e8)
    snprintf(buf, sizeof(buf), "%.1fM", n/1e6);
  else if (n < 1e10)
    snprintf(buf, sizeof(buf), "%.0fM", n/1e6);
  else if (n < 1e11)
    snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
  else if (n < 1e13)
    snprintf(buf, sizeof(buf), "%.0fG", n/1e9);
  else if (n < 1e14)
    snprintf(buf, sizeof(buf), "%.1fT", n/1e12);
  else if (n < 1e16)
    snprintf(buf, sizeof(buf), "%.0fT", n/1e12);
  else if (n < 1e17)
    snprintf(buf, sizeof(buf), "%.1fP", n/1e15);
  else if (n < 1e19)
    snprintf(buf, sizeof(buf), "%.0fP", n/1e15);
  else
    snprintf(buf, sizeof(buf), "%.1fE", n/1e18);
  return buf;
}

std::string formatIEC(size_t s)
{
  char buf[64] = "";
  if (s < 100000)
    snprintf(buf, sizeof buf, "%zd", s);
  else if (s < 1024*10000)
    snprintf(buf, sizeof buf, "%zdKi", s / 1024);
  else if (s < 1024*1024*100)
    snprintf(buf, sizeof buf, "%.1fMi", s / 1024.0 / 1024);
  else if (s < 1024L*1024*10000)
    snprintf(buf, sizeof buf, "%zdMi", s / 1024 / 1024);
  else if (s < 1024L*1024*1024*100)
    snprintf(buf, sizeof buf, "%.1fGi", s / 1024.0 / 1024 / 1024);
  else if (s < 1024L*1024*1024*10000)
    snprintf(buf, sizeof buf, "%zdGi", s / 1024 / 1024 / 1024);
  else if (s < 1024L*1024*1024*1024*100)
    snprintf(buf, sizeof buf, "%.1fTi", s / 1024.0 / 1024 / 1024 / 1024);
  else if (s < 1024L*1024*1024*1024*10000)
    snprintf(buf, sizeof buf, "%zdTi", s / 1024 / 1024 / 1024 / 1024);
  else if (s < 1024L*1024*1024*1024*1024*100)
    snprintf(buf, sizeof buf, "%.1fPi", s / 1024.0 / 1024 / 1024 / 1024 / 1024);
  else if (s < 1024UL*1024*1024*1024*1024*10000)
    snprintf(buf, sizeof buf, "%zdPi", s / 1024 / 1024 / 1024 / 1024 / 1024);
  else
    snprintf(buf, sizeof buf, "%.1fEi", s / 1024.0 / 1024 / 1024 / 1024 / 1024 / 1024);
  return buf;
}
