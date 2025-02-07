#ifndef MUDUO_BASE_SIGNALSLOTTRIVIAL_H
#define MUDUO_BASE_SIGNALSLOTTRIVIAL_H

#include <memory>
#include <vector>
#include <functional>

template<typename Signature>
class SignalTrivial;

template <typename RET, typename... ARGS>
class SignalTrivial<RET(ARGS...)>
{
 public:
  typedef std::function<void (ARGS...)> Functor;

  template<typename Func>
  void connect(Func&& func)
  {
    functors_.push_back(std::forward<Func>(func));
  }

  template<typename... Args>
  void call(Args&&... args)
  {
    // gcc 4.6 supports
    //for (const Functor& f: functors_)
    typename std::vector<Functor>::iterator it = functors_.begin();
    for (; it != functors_.end(); ++it)
    {
      (*it)(std::forward<Args>(args)...);
    }
  }

 private:
  std::vector<Functor> functors_;
};

#endif // MUDUO_BASE_SIGNALSLOTTRIVIAL_H
