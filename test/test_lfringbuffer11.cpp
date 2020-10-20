#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <memory>
#include <thread>
#include <pthread.h>

#include <cppunit.h>

#include <jau/ringbuffer.hpp>

using namespace jau;

class Integer {
    public:
        jau::nsize_t value;

        Integer(jau::nsize_t v) : value(v) {}

        Integer(const Integer &o) noexcept = default;
        Integer(Integer &&o) noexcept = default;
        Integer& operator=(const Integer &o) noexcept = default;
        Integer& operator=(Integer &&o) noexcept = default;

        operator jau::nsize_t() const {
            return value;
        }
        jau::nsize_t intValue() const { return value; }
        static Integer valueOf(const jau::nsize_t i) { return Integer(i); }
};

std::shared_ptr<Integer> NullInteger = nullptr;

typedef std::shared_ptr<Integer> SharedType;
typedef ringbuffer<SharedType, nullptr, jau::nsize_t> SharedTypeRingbuffer;

// Test examples.
class Cppunit_tests : public Cppunit {
  private:

    std::shared_ptr<SharedTypeRingbuffer> createEmpty(jau::nsize_t initialCapacity) {
        return std::shared_ptr<SharedTypeRingbuffer>(new SharedTypeRingbuffer(initialCapacity));
    }
    std::shared_ptr<SharedTypeRingbuffer> createFull(const std::vector<std::shared_ptr<Integer>> & source) {
        return std::shared_ptr<SharedTypeRingbuffer>(new SharedTypeRingbuffer(source));
    }

    std::vector<SharedType> createIntArray(const jau::nsize_t capacity, const jau::nsize_t startValue) {
        std::vector<SharedType> array(capacity);
        for(jau::nsize_t i=0; i<capacity; i++) {
            array[i] = SharedType(new Integer(startValue+i));
        }
        return array;
    }

    void getThreadType01(const std::string msg, std::shared_ptr<SharedTypeRingbuffer> rb, jau::nsize_t len) {
        // std::thread::id this_id = std::this_thread::get_id();
        // pthread_t this_id = pthread_self();

        fprintf(stderr, "%s: Created / %s\n", msg.c_str(), rb->toString().c_str());
        for(jau::nsize_t i=0; i<len; i++) {
            SharedType svI = rb->getBlocking();
            CHECKTM(msg+": Empty at read #"+std::to_string(i+1)+": "+rb->toString(), svI!=nullptr);
            fprintf(stderr, "%s: Got %u / %s\n",
                    msg.c_str(), svI->intValue(), rb->toString().c_str());
        }
        fprintf(stderr, "%s: Dies / %s\n", msg.c_str(), rb->toString().c_str());
    }

    void putThreadType01(const std::string msg, std::shared_ptr<SharedTypeRingbuffer> rb, jau::nsize_t len, jau::nsize_t startValue) {
        // std::thread::id this_id = std::this_thread::get_id();
        // pthread_t this_id = pthread_self();

        fprintf(stderr, "%s: Created / %s\n", msg.c_str(), rb->toString().c_str());
        jau::nsize_t preSize = rb->getSize();
        (void)preSize;

        for(jau::nsize_t i=0; i<len; i++) {
            Integer * vI = new Integer(startValue+i);
            fprintf(stderr, "%s: Putting %u ... / %s\n",
                    msg.c_str(), vI->intValue(), rb->toString().c_str());
            rb->putBlocking( SharedType( vI ) );
        }
        fprintf(stderr, "%s: Dies / %s\n", msg.c_str(), rb->toString().c_str());
    }

  public:

    void test01_Read1Write1() {
        fprintf(stderr, "\n\ntest01_Read1Write1\n");
        jau::nsize_t capacity = 100;
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
        CHECKM("Not empty size "+rb->toString(), 0, rb->getSize());
        CHECKTM("Not empty "+rb->toString(), rb->isEmpty());

        std::thread getThread01(&Cppunit_tests::getThreadType01, this, "test01.get01", rb, capacity); // @suppress("Invalid arguments")
        std::thread putThread01(&Cppunit_tests::putThreadType01, this, "test01.put01", rb, capacity, 0); // @suppress("Invalid arguments")
        putThread01.join();
        getThread01.join();

        CHECKTM("Not empty "+rb->toString(), rb->isEmpty());
        CHECKM("Not empty size "+rb->toString(), 0, rb->getSize());
    }

    void test02_Read4Write1() {
        fprintf(stderr, "\n\ntest02_Read4Write1\n");
        jau::nsize_t capacity = 400;
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
        CHECKM("Not empty size "+rb->toString(), 0, rb->getSize());
        CHECKTM("Not empty "+rb->toString(), rb->isEmpty());

        std::thread getThread01(&Cppunit_tests::getThreadType01, this, "test02.get01", rb, capacity/4); // @suppress("Invalid arguments")
        std::thread getThread02(&Cppunit_tests::getThreadType01, this, "test02.get02", rb, capacity/4); // @suppress("Invalid arguments")
        std::thread putThread01(&Cppunit_tests::putThreadType01, this, "test02.put01", rb, capacity, 0); // @suppress("Invalid arguments")
        std::thread getThread03(&Cppunit_tests::getThreadType01, this, "test02.get03", rb, capacity/4); // @suppress("Invalid arguments")
        std::thread getThread04(&Cppunit_tests::getThreadType01, this, "test02.get04", rb, capacity/4); // @suppress("Invalid arguments")
        putThread01.join();
        getThread01.join();
        getThread02.join();
        getThread03.join();
        getThread04.join();

        CHECKTM("Not empty "+rb->toString(), rb->isEmpty());
        CHECKM("Not empty size "+rb->toString(), 0, rb->getSize());
    }

    void test03_Read8Write2() {
        fprintf(stderr, "\n\ntest03_Read8Write2\n");
        jau::nsize_t capacity = 800;
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
        CHECKM("Not empty size "+rb->toString(), 0, rb->getSize());
        CHECKTM("Not empty "+rb->toString(), rb->isEmpty());

        std::thread getThread01(&Cppunit_tests::getThreadType01, this, "test03.get01", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread02(&Cppunit_tests::getThreadType01, this, "test03.get02", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread putThread01(&Cppunit_tests::putThreadType01, this, "test03.put01", rb, capacity/2,  0); // @suppress("Invalid arguments")
        std::thread getThread03(&Cppunit_tests::getThreadType01, this, "test03.get03", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread04(&Cppunit_tests::getThreadType01, this, "test03.get04", rb, capacity/8); // @suppress("Invalid arguments")

        std::thread getThread05(&Cppunit_tests::getThreadType01, this, "test03.get05", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread06(&Cppunit_tests::getThreadType01, this, "test03.get06", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread putThread02(&Cppunit_tests::putThreadType01, this, "test03.put02", rb, capacity/2,  400); // @suppress("Invalid arguments")
        std::thread getThread07(&Cppunit_tests::getThreadType01, this, "test03.get07", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread08(&Cppunit_tests::getThreadType01, this, "test03.get08", rb, capacity/8); // @suppress("Invalid arguments")

        putThread01.join();
        putThread02.join();
        getThread01.join();
        getThread02.join();
        getThread03.join();
        getThread04.join();
        getThread05.join();
        getThread06.join();
        getThread07.join();
        getThread08.join();

        CHECKTM("Not empty "+rb->toString(), rb->isEmpty());
        CHECKM("Not empty size "+rb->toString(), 0, rb->getSize());
    }

    void test_list() override {
        test01_Read1Write1();
        test02_Read4Write1();
        test03_Read8Write2();

        test01_Read1Write1();
        test02_Read4Write1();
        test03_Read8Write2();

        test03_Read8Write2();
        test03_Read8Write2();
        test03_Read8Write2();
    }
};

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    Cppunit_tests test1;
    return test1.run();
}

