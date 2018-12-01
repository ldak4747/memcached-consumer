#include "FirehoseConsumer.h"

//extern Logger logger;
std::string FirehoseConsumer::consumergroup = "weibo_ad_aitao";

FirehoseConsumer::FirehoseConsumer (std::string _consumer,
                                    std::string _brokers,
                                    std::string _type,
                                    std::string _event,
                                    std::string _object): 
                                    consumer(_consumer),
                                    brokerservers(_brokers),
                                    type(_type),
                                    event(_event),
                                    object(_object) {
}

FirehoseConsumer::~FirehoseConsumer () {
    if (loopthread->joinable()) {
        loopthread->join();
    }

    memcached_server_list_free(brokers);
    memcached_free(handle);
}

void FirehoseConsumer::startloop () {
    running = true;
    loopthread.reset(new std::thread([this] () {
        reconnect();
        while (running) {
            memcached_return res;
            size_t valuelen;
            unsigned int flag(0);

            std::string key = makeKey().c_str();
            char *result = memcached_get(handle, key.c_str(), key.length(), &valuelen, &flag, &res);
            std::string value(result);
            free(result);
            if (res == MEMCACHED_SUCCESS && !value.empty()) {
                weibo::TriggerMessageBatch batch;
                batch.ParseFromString(value);

                if (batch.messages_size()) {
                    emptycnt = 0;
                    for (int i = 0; i < batch.messages_size(); i++) {
                        weibo::TriggerMessage msg = batch.messages(i);
                        std::string topic = msg.topic();
                        std::string type = msg.type();
                        std::string event = msg.event();
                        uint64_t offset = msg.offset();
                        updateOffset(offset);

                        std::shared_ptr<weibo::TriggerMessageBody> body = std::make_shared<weibo::TriggerMessageBody>();
                        body->ParseFromString(msg.bodybytes());
                        Parse(body);
                        while(!workers->AddTask(std::bind(&FirehoseConsumer::Process, this, std::placeholders::_1), body));
                    }
                } else {
                    ++emptycnt;
                    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            } else {
                ++emptycnt;
                std::cout << "memcached_get fail with retcode " << res << std::endl;
            }
            
            if (emptycnt > 1000) {
                reconnect();
                emptycnt = 0;
            }
        }
    }));
}

void FirehoseConsumer::reconnect () {
    memcached_return res;
    do {
        if (brokers) {
            memcached_server_list_free(brokers);
            brokers = nullptr;
        }
        if (handle) {
            memcached_free(handle);
        }

        brokers = memcached_server_list_append(brokers, brokerservers.c_str(), brokerport, &res);

        handle = memcached_create(nullptr);
        res = memcached_server_push(handle, brokers);
        if (res != MEMCACHED_SUCCESS) {
            std::cout << "memcached connect fail with retcode " << res << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } while (res != MEMCACHED_SUCCESS);

    std::cout << "memcached connect success" << std::endl;
}

/*
 *  status、comment、relation、direct_message、other
 * */
void StatusCallback (Result result) {
    static int count = 0;
    static std::chrono::steady_clock::time_point st = std::chrono::steady_clock::now(), ed = std::chrono::steady_clock::now();
    ++count;
    ed = std::chrono::steady_clock::now();
    std::cout << "cost " << std::chrono::duration_cast<std::chrono::milliseconds>(ed - st).count() << std::endl;
    st = std::chrono::steady_clock::now();
}

FirehoseStatusConsumer::~FirehoseStatusConsumer () {
}
void FirehoseStatusConsumer::Parse (std::shared_ptr<weibo::TriggerMessageBody> body) {
    //TODO maybe useless
    //to modify body, if need.
}

std::string FirehoseStatusConsumer::makeKey () {
    std::string key = topic;
    key += "$";
    key += consumergroup;
    key += "$";
    key += consumer;
    key += "$";
    key += std::to_string(maxsize);
    key += "$";
    if (lastoffset == UNDEFINEDOFFSET) {
        key += "##";
    } else {
        key += std::to_string(lastoffset);
        key += "##";
    }
    
    key += type;
    key += "#";
    key += event;
    key += "#";
    key += object;
    key += "#";
    
    return key;
}

Result FirehoseStatusConsumer::Process (std::shared_ptr<weibo::TriggerMessageBody> body) {
    Result result(std::make_tuple(true, "", body));
    weibo::Status msg;
    msg.ParseFromString(body->body());

    std::string type = body->type(), event = body->event();
    uint64_t id = msg.id(), mid = msg.mid(), created_at = msg.created_at();
    std::string text = msg.text();
    
    std::cout << "id: " << id << ", text: " << text << ", mid: " << mid << ", create_at: " << created_at << ", type: " << type << ", event: " << event << std::endl;
    return result;
}

