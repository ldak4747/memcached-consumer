#include "FirehoseFactory.h"
#include "FirehoseConsumer.h"
#include <iostream>

void init (int argc, char *argv[], std::string &consumer,
                                   std::string &broker,
                                   std::string &type,
                                   std::string &event,
                                   std::string &object) {
    consumer = argv[1];
    broker = argv[2];
    if (argc >= 4) {
        type = argv[3];
    }
    if (argc >= 5) {
        event = argv[4];
    }
    if (argc == 6) {
        object = argv[5];
    }
}

int main (int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Usage: at least ./main consumerID brokeraddr" << std::endl;
        return 0;
    }

    std::string consumer, broker, type, event, object;
    consumer = broker = type = event = object = "";
    init(argc, argv, consumer, broker, type, event, object);
    //Loginit("FCLogger");

    std::shared_ptr<FirehoseStatusConsumer> status_consumer = FirehoseConsumerFactory<FirehoseStatusConsumer>::getInstance(consumer, broker, type, event, object);
    while(1);
    return 0;
}
