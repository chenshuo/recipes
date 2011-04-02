#include "codec.h"

#include "query.pb.h"

#include <iostream>
#include <typeinfo>

#include <assert.h>
#include <stdio.h>

using std::cout;
using std::endl;

template<typename T>
void testDescriptor()
{
  std::string type_name = T::descriptor()->full_name();
  cout << type_name << endl;

  const google::protobuf::Descriptor* descriptor =
    google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
  assert(descriptor == T::descriptor());
  cout << "FindMessageTypeByName() = " << descriptor << endl;
  cout << "T::descriptor()         = " << T::descriptor() << endl;
  cout << endl;

  const google::protobuf::Message* prototype =
    google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
  assert(prototype == &T::default_instance());
  cout << "GetPrototype()        = " << prototype << endl;
  cout << "T::default_instance() = " << &T::default_instance() << endl;
  cout << endl;

  T* new_obj = dynamic_cast<T*>(prototype->New());
  assert(new_obj != NULL);
  assert(new_obj != prototype);
  assert(typeid(*new_obj) == typeid(T::default_instance()));
  cout << "prototype->New() = " << new_obj << endl;
  cout << endl;
  delete new_obj;
}

int main()
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  testDescriptor<muduo::Query>();
  testDescriptor<muduo::Answer>();

  google::protobuf::Message* newQuery = createMessage("muduo.Query");
  assert(newQuery != NULL);
  assert(typeid(*newQuery) == typeid(muduo::Query::default_instance()));
  cout << "createMessage(\"muduo.Query\") = " << newQuery << endl;

  google::protobuf::Message* newAnswer = createMessage("muduo.Answer");
  assert(newAnswer != NULL);
  assert(typeid(*newAnswer) == typeid(muduo::Answer::default_instance()));
  cout << "createMessage(\"muduo.Answer\") = " << newAnswer << endl;

  delete newQuery;
  delete newAnswer;

  puts("All pass!!!");

  google::protobuf::ShutdownProtobufLibrary();
}

