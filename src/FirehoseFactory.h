#pragma once
#include <libmemcached/memcached.h>
#include <memory>
#include <mutex>

template<class Consumer> class FirehoseConsumerFactory {
    static std::shared_ptr<Consumer> consumer;

    FirehoseConsumerFactory() = default;
    FirehoseConsumerFactory(const FirehoseConsumerFactory &other) = delete;
    FirehoseConsumerFactory &operator= (const FirehoseConsumerFactory &) = delete;

public:
    template<class ...Args> static std::shared_ptr<Consumer> getInstance(Args &&...args) {
        static std::once_flag flag;
        std::call_once(flag, [&] () {
            consumer.reset(new Consumer(std::forward<Args>(args)...));
        });

        return consumer;
    }
};

template<class Consumer> std::shared_ptr<Consumer> FirehoseConsumerFactory<Consumer>::consumer;
