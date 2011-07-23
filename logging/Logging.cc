#include "Logging.h"

#include <datetime/Timestamp.h>
#include <thread/Thread.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace muduo
{

/*
class LoggerImpl
{
 public:
  typedef Logger::LogLevel LogLevel;
  LoggerImpl(LogLevel level, int old_errno, const char* file, int line);
  void finish();

  Timestamp time_;
  LogStream stream_;
  LogLevel level_;
  const char* fullname_;
  int line_;
  const char* basename_;
  const char* function_;
};
*/

__thread char t_errnobuf[512];

const char* strerror_tl(int savedErrno)
{
  return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel initLogLevel()
{
  if (::getenv("MUDUO_LOG_TRACE"))
    return Logger::TRACE;
  else
    return Logger::DEBUG;
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "TRACE",
  "DEBUG",
  "INFO",
  "WARN",
  "ERROR",
  "FATAL",
};

}

using namespace muduo;

Logger::Impl::Impl(LogLevel level, int savedErrno, const char* file, int line)
  : time_(Timestamp::now()),
    stream_(),
    level_(level),
    fullname_(file),
    line_(line),
    basename_(NULL),
    function_(NULL)
{
  const char* path_sep_pos = strrchr(fullname_, '/');
  basename_ = (path_sep_pos != NULL) ? path_sep_pos + 1 : fullname_;

  stream_ << time_.toFormattedString() << ' '
          << Fmt("%5d ", CurrentThread::tid())
          << LogLevelName[level] << ' ';
  if (savedErrno != 0)
  {
    stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
  }
}

void Logger::Impl::finish()
{
  stream_ << " - " << basename_ << ":" << line_ << '\n';
}

Logger::Logger(const char* file, int line)
  : impl_(INFO, 0, file, line)
{
}

Logger::Logger(const char* file, int line, LogLevel level, const char* func)
  : impl_(level, 0, file, line)
{
  impl_.stream_ << func << ' ';
}

Logger::Logger(const char* file, int line, LogLevel level)
  : impl_(level, 0, file, line)
{
}

Logger::Logger(const char* file, int line, bool toAbort)
  : impl_(toAbort?FATAL:ERROR, errno, file, line)
{
}

Logger::~Logger()
{
  impl_.finish();
  const LogStream::Buffer& buf(stream().buffer());
  // ssize_t n = ::write(1, buf.data(), buf.length());
  size_t n = fwrite(buf.data(), 1, buf.length(), stdout);
  //FIXME check n
  (void)n;
  if (impl_.level_ == FATAL)
  {
    abort();
  }
}

Logger::LogLevel Logger::logLevel()
{
  return g_logLevel;
}

void Logger::setLogLevel(Logger::LogLevel level)
{
  g_logLevel = level;
}

