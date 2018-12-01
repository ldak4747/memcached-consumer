#pragma once
#include "FirehoseFactory.h"
#include "ThreadPool.h"
#include "Log.h"
#include "TriggerMessageBatch.pb.h"
#include "TriggerMessageBody.pb.h"
#include "TriggerMessage.pb.h"
#include "status.pb.h"
#include <iostream>

const int UNDEFINEDOFFSET = -1;
const int MAXSIZE = 50000000;
const int worker_num = 8;
const int BROKERPORT = 22222;

class FirehoseConsumer {
    memcached_st *handle = nullptr;
    memcached_server_st *brokers = nullptr;
    std::string brokerservers;
    
    std::unique_ptr<std::thread> loopthread;

protected:
    std::shared_ptr<ThreadPool> workers;
    static std::string consumergroup;
    std::string consumer, type, event, object;
    int emptycnt = 0;

    static const int maxsize = MAXSIZE;
    static const int brokerport = BROKERPORT;
    uint64_t lastoffset = UNDEFINEDOFFSET;
    void updateOffset(uint64_t setup) { lastoffset = setup; }

    void reconnect();
    void startloop();
    bool running = false;

public:
    FirehoseConsumer(std::string _consumers,
                     std::string _brokers,
                     std::string _type = "",
                     std::string _event = "",
                     std::string _object = "");

    virtual ~FirehoseConsumer();
    virtual std::string makeKey() = 0;
    virtual void Parse(std::shared_ptr<weibo::TriggerMessageBody> body) = 0;
    virtual Result Process(std::shared_ptr<weibo::TriggerMessageBody> body) = 0;

    void set_consumer(const std::string &setup) { consumer = setup; }
    void set_type(const std::string &setup) { type = setup; }
    void set_event(const std::string &setup) { event = setup; }
    void set_object(const std::string &setup) { object = setup; }
    std::string get_consumer() const { return consumer; }
    std::string get_type() const { return type; }
    std::string get_event() const { return event; }
    std::string get_object() const { return object; }
};



/*
 *  status、comment、relation、direct_message、other
 * */
void StatusCallback (Result result);
class FirehoseStatusConsumer : public FirehoseConsumer {
    const std::string topic = "status";
    
    FirehoseStatusConsumer( std::string _consumer,
                            std::string _brokers,
                            std::string _type = "",
                            std::string _event = "",
                            std::string _object = "") : FirehoseConsumer(_consumer, _brokers, _type, _event, _object) {

    workers.reset(new ThreadPool(StatusCallback));
    startloop();
}
    friend class FirehoseConsumerFactory<FirehoseStatusConsumer>;
public:
    virtual ~FirehoseStatusConsumer();
    virtual std::string makeKey() override;
    virtual void Parse(std::shared_ptr<weibo::TriggerMessageBody> body) override;
    virtual Result Process(std::shared_ptr<weibo::TriggerMessageBody> body) override;
};

class FirehoseCommentConsumer : public FirehoseConsumer {
    const std::string topic = "comment";
    
    FirehoseCommentConsumer( std::string _consumer,
                             std::string _brokers,
                             std::string _type = "",
                             std::string _event = "",
                             std::string _object = "") : FirehoseConsumer(_consumer, _brokers, _type, _event, _object) {}
    friend class FirehoseConsumerFactory<FirehoseCommentConsumer>;
public:
    virtual ~FirehoseCommentConsumer();
    virtual std::string makeKey() override;
    virtual void Parse(std::shared_ptr<weibo::TriggerMessageBody> body) override;
    virtual Result Process(std::shared_ptr<weibo::TriggerMessageBody> body) override;
};

class FirehoseRelationConsumer : public FirehoseConsumer {
    const std::string topic = "relation";
    
    FirehoseRelationConsumer( std::string _consumer,
                              std::string _brokers,
                              std::string _type = "",
                              std::string _event = "",
                              std::string _object = "") : FirehoseConsumer(_consumer, _brokers, _type, _event, _object) {}
    friend class FirehoseConsumerFactory<FirehoseRelationConsumer>;
public:
    virtual ~FirehoseRelationConsumer();
    virtual std::string makeKey() override;
    virtual void Parse(std::shared_ptr<weibo::TriggerMessageBody> body) override;
    virtual Result Process(std::shared_ptr<weibo::TriggerMessageBody> body) override;
};

class FirehoseDmConsumer : public FirehoseConsumer {
    const std::string topic = "direct_message";
    
    FirehoseDmConsumer( std::string _consumer,
                        std::string _brokers,
                        std::string _type = "",
                        std::string _event = "",
                        std::string _object = "") : FirehoseConsumer(_consumer, _brokers, _type, _event, _object) {}
    friend class FirehoseConsumerFactory<FirehoseDmConsumer>;
public:
    virtual ~FirehoseDmConsumer();
    virtual std::string makeKey() override;
    virtual void Parse(std::shared_ptr<weibo::TriggerMessageBody> body) override;
    virtual Result Process(std::shared_ptr<weibo::TriggerMessageBody> body) override;
};

class FirehoseOtherConsumer : public FirehoseConsumer {
    const std::string topic = "other";
    
    FirehoseOtherConsumer( std::string _consumer,
                           std::string _brokers,
                           std::string _type = "",
                           std::string _event = "",
                           std::string _object = "") : FirehoseConsumer(_consumer, _brokers, _type, _event, _object) {}
    friend class FirehoseConsumerFactory<FirehoseOtherConsumer>;
public:
    virtual ~FirehoseOtherConsumer();
    virtual std::string makeKey() override;
    virtual void Parse(std::shared_ptr<weibo::TriggerMessageBody> body) override;
    virtual Result Process(std::shared_ptr<weibo::TriggerMessageBody> body) override;
};

