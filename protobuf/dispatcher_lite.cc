#include "query.pb.h"

#include <boost/function.hpp>

#include <iostream>

using namespace std;

class ProtobufDispatcherLite
{
 public:
  typedef boost::function<void (google::protobuf::Message* message)> ProtobufMessageCallback;

  explicit ProtobufDispatcherLite(const ProtobufMessageCallback& defaultCb)
    : defaultCallback_(defaultCb)
  {
  }

  void onMessage(google::protobuf::Message* message) const
  {
    CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
    if (it != callbacks_.end())
    {
      it->second(message);
    }
    else
    {
      defaultCallback_(message);
    }
  }

  void registerMessageCallback(const google::protobuf::Descriptor* desc, const ProtobufMessageCallback& callback)
  {
    callbacks_[desc] = callback;
  }

 private:
  typedef std::map<const google::protobuf::Descriptor*, ProtobufMessageCallback> CallbackMap;

  CallbackMap callbacks_;
  ProtobufMessageCallback defaultCallback_;
};

void onQuery(google::protobuf::Message* message)
{
  cout << "onQuery: " << message->GetTypeName() << endl;
  muduo::Query* query = dynamic_cast<muduo::Query*>(message);
  assert(query != NULL);
}

void onAnswer(google::protobuf::Message* message)
{
  cout << "onAnswer: " << message->GetTypeName() << endl;
  muduo::Answer* answer = dynamic_cast<muduo::Answer*>(message);
  assert(answer != NULL);
}

void onUnknownMessageType(google::protobuf::Message* message)
{
  cout << "Discarding " << message->GetTypeName() << endl;
}

int main()
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ProtobufDispatcherLite dispatcher(onUnknownMessageType);
  dispatcher.registerMessageCallback(muduo::Query::descriptor(), onQuery);
  dispatcher.registerMessageCallback(muduo::Answer::descriptor(), onAnswer);

  muduo::Query q;
  muduo::Answer a;
  muduo::Empty e;
  dispatcher.onMessage(&q);
  dispatcher.onMessage(&a);
  dispatcher.onMessage(&e);

  google::protobuf::ShutdownProtobufLibrary();
}

