#ifndef MUDUO_BASE_SIGNALSLOT_H
#define MUDUO_BASE_SIGNALSLOT_H

#include "Mutex.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <vector>

namespace muduo
{

namespace detail
{

template<typename Callback>
struct SlotImpl;

template<typename Callback>
struct SignalImpl : boost::noncopyable
{
  typedef std::vector<boost::weak_ptr<SlotImpl<Callback> > > SlotList;

  SignalImpl()
    : slots_(new SlotList)
  {
  }

  void copyOnWrite()
  {
    mutex_.assertLocked();
    if (!slots_.unique())
    {
      slots_.reset(new SlotList(*slots_));
    }
    assert(slots_.unique());
  }

  void clean()
  {
    MutexLockGuard lock(mutex_);
    copyOnWrite();
    SlotList& list(*slots_);
    typename SlotList::iterator it(list.begin());
    while (it != list.end())
    {
      if (it->expired())
      {
        it = list.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  MutexLock mutex_;
  boost::shared_ptr<SlotList> slots_;
};

template<typename Callback>
struct SlotImpl : boost::noncopyable
{
  typedef SignalImpl<Callback> Data;
  SlotImpl(const boost::shared_ptr<Data>& data, Callback&& cb)
    : data_(data), cb_(cb), tie_(), tied_(false)
  {
  }

  SlotImpl(const boost::shared_ptr<Data>& data, Callback&& cb,
           const boost::shared_ptr<void>& tie)
    : data_(data), cb_(cb), tie_(tie), tied_(true)
  {
  }

  ~SlotImpl()
  {
    boost::shared_ptr<Data> data(data_.lock());
    if (data)
    {
      data->clean();
    }
  }

  boost::weak_ptr<Data> data_;
  Callback cb_;
  boost::weak_ptr<void> tie_;
  bool tied_;
};

}

/// This is the handle for a slot
///
/// The slot will remain connected to the signal fot the life time of the
/// returned Slot object (and its copies).
typedef boost::shared_ptr<void> Slot;

template<typename Signature>
class Signal;

template <typename RET, typename... ARGS>
class Signal<RET(ARGS...)> : boost::noncopyable
{
 public:
  typedef std::function<void (ARGS...)> Callback;
  typedef detail::SignalImpl<Callback> SignalImpl;
  typedef detail::SlotImpl<Callback> SlotImpl;

  Signal()
    : impl_(new SignalImpl)
  {
  }

  ~Signal()
  {
  }

  Slot connect(Callback&& func)
  {
    boost::shared_ptr<SlotImpl> slotImpl(
        new SlotImpl(impl_, std::forward<Callback>(func)));
    add(slotImpl);
    return slotImpl;
  }

  Slot connect(Callback&& func, const boost::shared_ptr<void>& tie)
  {
    boost::shared_ptr<SlotImpl> slotImpl(new SlotImpl(impl_, func, tie));
    add(slotImpl);
    return slotImpl;
  }

  void call(ARGS&&... args)
  {
    SignalImpl& impl(*impl_);
    boost::shared_ptr<typename SignalImpl::SlotList> slots;
    {
      MutexLockGuard lock(impl.mutex_);
      slots = impl.slots_;
    }
    typename SignalImpl::SlotList& s(*slots);
    for (typename SignalImpl::SlotList::const_iterator it = s.begin(); it != s.end(); ++it)
    {
      boost::shared_ptr<SlotImpl> slotImpl = it->lock();
      if (slotImpl)
      {
        boost::shared_ptr<void> guard;
        if (slotImpl->tied_)
        {
          guard = slotImpl->tie_.lock();
          if (guard)
          {
            slotImpl->cb_(args...);
          }
        }
        else
        {
          slotImpl->cb_(args...);
        }
      }
    }
  }

 private:

  void add(const boost::shared_ptr<SlotImpl>& slot)
  {
    SignalImpl& impl(*impl_);
    {
      MutexLockGuard lock(impl.mutex_);
      impl.copyOnWrite();
      impl.slots_->push_back(slot);
    }
  }

  const boost::shared_ptr<SignalImpl> impl_;
};

}

#endif // MUDUO_BASE_SIGNALSLOT_H
