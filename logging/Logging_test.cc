#include "Logging.h"

int main()
{
  getppid();
  LOG_INFO << "Hello";
  LOG_WARN << "World";
}
