#include "../SignalSlot.h"
#include "../SignalSlotTrivial.h"

#include <boost/bind.hpp>

#define BOOST_TEST_MAIN
#ifdef BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif

class String
{
 public:
  String(const char* str)
  {
    printf("String ctor this %p\n", this);
  }

  String(const String& rhs)
  {
    printf("String copy ctor this %p, rhs %p\n", this, &rhs);
  }

  String(String&& rhs)
  {
    printf("String move ctor this %p, rhs %p\n", this, &rhs);
  }
};

class Foo : boost::noncopyable
{
 public:
  void zero();
  void zeroc() const;
  void one(int);
  void oner(int&);
  void onec(int) const;
  void oneString(const String& str);
  // void oneStringRR(String&& str);
  static void szero();
  static void sone(int);
  static void soneString(const String& str);
};

void Foo::zero()
{
  printf("Foo::zero()\n");
}

void Foo::zeroc() const
{
  printf("Foo::zeroc()\n");
}

void Foo::szero()
{
  printf("Foo::szero()\n");
}

void Foo::one(int x)
{
  printf("Foo::one() x=%d\n", x);
}

void Foo::onec(int x) const
{
  printf("Foo::onec() x=%d\n", x);
}

void Foo::sone(int x)
{
  printf("Foo::sone() x=%d\n", x);
}

void Foo::oneString(const String& str)
{
  printf("Foo::oneString\n");
}

void Foo::soneString(const String& str)
{
  printf("Foo::soneString\n");
}

BOOST_AUTO_TEST_CASE(testSignalSlotTrivialZero)
{
  SignalTrivial<void()> signal;

  printf("========\n");
  signal.call();

  signal.connect(&Foo::szero);

  printf("========\n");
  signal.call();

  Foo f;
  signal.connect(boost::bind(&Foo::zero, &f));

  printf("========\n");
  signal.call();

  signal.connect(boost::bind(&Foo::one, &f, 42));

  printf("========\n");
  signal.call();

  const Foo cf;
  signal.connect(boost::bind(&Foo::zeroc, &cf));

  printf("========\n");
  signal.call();

  signal.connect(boost::bind(&Foo::onec, &cf, 128));

  printf("========\n");
  signal.call();

  printf("========\n");
  signal.call();
}

BOOST_AUTO_TEST_CASE(testSignalSlotTrivialOne)
{
  SignalTrivial<void(int)> signal;

  printf("========\n");
  signal.call(50);

  signal.connect(&Foo::sone);

  printf("========\n");
  signal.call(51);

  Foo f;
  signal.connect(boost::bind(&Foo::one, &f, _1));

  printf("========\n");
  signal.call(52);

  const Foo cf;
  signal.connect(boost::bind(&Foo::onec, &cf, _1));

  printf("========\n");
  signal.call(53);
}

BOOST_AUTO_TEST_CASE(testSignalSlotTrivialString)
{
  SignalTrivial<void(const String&)> signal;
  signal.call("hello");

  signal.connect(&Foo::soneString);

  printf("========\n");
  signal.call("hello");

  Foo f;
  signal.connect(boost::bind(&Foo::oneString, &f, _1));

  printf("========\n");
  signal.call("hello");
}

BOOST_AUTO_TEST_CASE(testSignalSlotZero)
{
  muduo::Signal<void()> signal;

  printf("==== testSignalSlotZero ====\n");
  signal.call();

  muduo::Slot s1 = signal.connect(&Foo::szero);

  printf("========\n");
  signal.call();

  Foo f;
  muduo::Slot s2 = signal.connect(boost::bind(&Foo::zero, &f));

  printf("========\n");
  signal.call();

  muduo::Slot s3 = signal.connect(boost::bind(&Foo::one, &f, 42));

  printf("========\n");
  signal.call();

  const Foo cf;
  muduo::Slot s4 = signal.connect(boost::bind(&Foo::zeroc, &cf));

  printf("========\n");
  signal.call();

  muduo::Slot s5 = signal.connect(boost::bind(&Foo::onec, &cf, 128));

  printf("========\n");
  signal.call();

  s1 = muduo::Slot();
  printf("========\n");
  signal.call();


  s4 = s3 = s2 = muduo::Slot();
  printf("========\n");
  signal.call();

}

BOOST_AUTO_TEST_CASE(testSignalSlotOne)
{
  muduo::Signal<void(int)> signal;

  printf("========\n");
  signal.call(50);

  muduo::Slot s4;
  {
  muduo::Slot s1 = signal.connect(&Foo::sone);

  printf("========\n");
  signal.call(51);

  Foo f;
  muduo::Slot s2 = signal.connect(boost::bind(&Foo::one, &f, _1));

  printf("========\n");
  signal.call(52);

  const Foo cf;
  muduo::Slot s3 = signal.connect(boost::bind(&Foo::onec, &cf, _1));

  printf("========\n");
  signal.call(53);

  s4 = s3;
  }

  printf("========\n");
  signal.call(54);
}

BOOST_AUTO_TEST_CASE(testSignalSlotLife)
{
  muduo::Slot s1;

  {
  muduo::Signal<void()> signal;
  s1 = signal.connect(&Foo::szero);

  printf("========\n");
  signal.call();

  Foo f;
  boost::function<void()> func = boost::bind(&Foo::zero, &f);

  s1 = signal.connect(func);

  printf("========\n");
  signal.call();
  }

}

