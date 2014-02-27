#pragma once

// A simple reference counted smart pointer.
// make use of GCC atomic builtins and C++ move semantics
template<typename T>
class counted_ptr
{
  typedef int* counted_ptr::*bool_type;

 public:
  counted_ptr(T* p = nullptr)
    : ptr_(p),
      count_(p ? new int(1) : nullptr)
  { }

  counted_ptr(const counted_ptr& rhs) noexcept
    : ptr_(rhs.ptr_),
      count_(rhs.count_)
  {
    if (count_)
      __atomic_fetch_add(count_, 1, __ATOMIC_SEQ_CST);
  }

  counted_ptr(counted_ptr&& rhs) noexcept
    : ptr_(rhs.ptr_),
      count_(rhs.count_)
  {
    rhs.ptr_ = nullptr;
    rhs.count_ = nullptr;
  }

  ~counted_ptr()
  {
    reset();
  }

  counted_ptr& operator=(counted_ptr rhs)
  {
    swap(rhs);
    return *this;
  }

  T* get() const noexcept
  {
    return ptr_;
  }

  void reset()
  {
    static_assert(sizeof(T) > 0, "T must be complete type");
    if (count_)
    {
      if (__atomic_sub_fetch(count_, 1, __ATOMIC_SEQ_CST) == 0)
      {
        delete ptr_;
        delete count_;
      }
      ptr_ = nullptr;
      count_ = nullptr;
    }
  }

  void swap(counted_ptr& rhs) noexcept
  {
    T* tp = ptr_;
    ptr_ = rhs.ptr_;
    rhs.ptr_ = tp;

    int* tc = count_;
    count_ = rhs.count_;
    rhs.count_ = tc;
  }

  T* operator->() const noexcept
  {
    // assert(ptr_);
    return ptr_;
  }

  T& operator*() const noexcept
  {
    // assert(ptr_);
    return *ptr_;
  }

  operator bool_type() const noexcept
  {
    return ptr_ ? &counted_ptr::count_ : nullptr;
  }

  int use_count() const noexcept
  {
    return count_ ? __atomic_load_n(count_, __ATOMIC_SEQ_CST) : 0;
  }

 private:
  T* ptr_;
  int* count_;
};
