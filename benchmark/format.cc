#include "format.h"

/*
 Format a number with 5 characters, including SI units.
 [0,     999]
 [1.00k, 999k]
 [1.00M, 999M]
 [1.00G, 999G]
 [1.00T, 999T]
 [1.00P, 999P]
 [1.00E, inf)
*/
std::string formatSI(size_t s)
{
  char buf[64];
  if (s < 1000)
    snprintf(buf, sizeof(buf), "%zd", s);
  else if (s < 9995)
    snprintf(buf, sizeof(buf), "%.2fk", s/1e3);
  else if (s < 99950)
    snprintf(buf, sizeof(buf), "%.1fk", s/1e3);
  else if (s < 999500)
    snprintf(buf, sizeof(buf), "%.0fk", s/1e3);
  else if (s < 9995000)
    snprintf(buf, sizeof(buf), "%.2fM", s/1e6);
  else if (s < 99950000)
    snprintf(buf, sizeof(buf), "%.1fM", s/1e6);
  else if (s < 999500000)
    snprintf(buf, sizeof(buf), "%.0fM", s/1e6);
  else if (s < 9995000000)
    snprintf(buf, sizeof(buf), "%.2fG", s/1e9);
  else if (s < 99950000000)
    snprintf(buf, sizeof(buf), "%.1fG", s/1e9);
  else if (s < 999500000000)
    snprintf(buf, sizeof(buf), "%.0fG", s/1e9);
  else if (s < 9995000000000)
    snprintf(buf, sizeof(buf), "%.2fT", s/1e12);
  else if (s < 99950000000000)
    snprintf(buf, sizeof(buf), "%.1fT", s/1e12);
  else if (s < 999500000000000)
    snprintf(buf, sizeof(buf), "%.0fT", s/1e12);
  else if (s < 9995000000000000)
    snprintf(buf, sizeof(buf), "%.2fP", s/1e15);
  else if (s < 99950000000000000)
    snprintf(buf, sizeof(buf), "%.1fP", s/1e15);
  else if (s < 999500000000000000)
    snprintf(buf, sizeof(buf), "%.0fP", s/1e15);
  else if (s < 9995000000000000000ULL)
    snprintf(buf, sizeof(buf), "%.2fE", s/1e18);
  else
    snprintf(buf, sizeof(buf), "%.1fE", s/1e18);
  return buf;
}

/*
std::string formatSI(size_t s)
{
  double n = s;
  char buf[64] = "";
  if (s < 100000)           // [0, 99999]
    snprintf(buf, sizeof(buf), "%zd", s);
  else if (s < 9999500)     // [ 100k, 9999k]
    snprintf(buf, sizeof(buf), "%zdk", (s+500)/1000);
  else if (s < 99950000)    // [10.0M, 99.9M]
    snprintf(buf, sizeof(buf), "%.1fM", s/1e6);
  else if (s < 9999500000)  // [ 100M, 9999M]
    snprintf(buf, sizeof(buf), "%zdM", (s+500000)/1000000);
  else if (s < 99950000000) // [10.0G, 99.9G]
    snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
  else if (n < 1e13)        // [ 100G, 9999G]
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
*/

/*
 [0, 1023]
 [1.00Ki, 9.99Ki]
 [10.0Ki, 99.9Ki]
 [ 100Ki, 1023Ki]
 [1.00Mi, 9.99Mi]
*/
std::string formatIEC(size_t s)
{
  double n = s;
  char buf[64];
  const double Ki = 1024.0;
  const double Mi = Ki * 1024.0;
  const double Gi = Mi * 1024.0;
  const double Ti = Gi * 1024.0;
  const double Pi = Ti * 1024.0;
  const double Ei = Pi * 1024.0;

  if (n < Ki)
    snprintf(buf, sizeof buf, "%zd", s);
  else if (s < Ki*9.995)
    snprintf(buf, sizeof buf, "%.2fKi", s / Ki);
  else if (s < Ki*99.95)
    snprintf(buf, sizeof buf, "%.1fKi", s / Ki);
  else if (s < Ki*1023.5)
    snprintf(buf, sizeof buf, "%.0fKi", s / Ki);

  else if (s < Mi*9.995)
    snprintf(buf, sizeof buf, "%.2fMi", s / Mi);
  else if (s < Mi*99.95)
    snprintf(buf, sizeof buf, "%.1fMi", s / Mi);
  else if (s < Mi*1023.5)
    snprintf(buf, sizeof buf, "%.0fMi", s / Mi);

  else if (s < Gi*9.995)
    snprintf(buf, sizeof buf, "%.2fGi", s / Gi);
  else if (s < Gi*99.95)
    snprintf(buf, sizeof buf, "%.1fGi", s / Gi);
  else if (s < Gi*1023.5)
    snprintf(buf, sizeof buf, "%.0fGi", s / Gi);

  else if (s < Ti*9.995)
    snprintf(buf, sizeof buf, "%.2fTi", s / Ti);
  else if (s < Ti*99.95)
    snprintf(buf, sizeof buf, "%.1fTi", s / Ti);
  else if (s < Ti*1023.5)
    snprintf(buf, sizeof buf, "%.0fTi", s / Ti);

  else if (s < Pi*9.995)
    snprintf(buf, sizeof buf, "%.2fPi", s / Pi);
  else if (s < Pi*99.95)
    snprintf(buf, sizeof buf, "%.1fPi", s / Pi);
  else if (s < Pi*1023.5)
    snprintf(buf, sizeof buf, "%.0fPi", s / Pi);

  else if (s < Ei*9.995)
    snprintf(buf, sizeof buf, "%.2fEi", s / Ei );
  else
    snprintf(buf, sizeof buf, "%.1fEi", s / Ei );
  return buf;
}
