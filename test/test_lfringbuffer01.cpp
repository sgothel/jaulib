#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <memory>

#define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
#include <jau/test/catch2_ext.hpp>

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
class TestRingbuffer01 {
  private:

    std::shared_ptr<SharedTypeRingbuffer> createEmpty(jau::nsize_t initialCapacity) {
        std::shared_ptr<SharedTypeRingbuffer> rb = std::shared_ptr<SharedTypeRingbuffer>(new SharedTypeRingbuffer(initialCapacity));
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());
        return rb;
    }
    std::shared_ptr<SharedTypeRingbuffer> createFull(const std::vector<std::shared_ptr<Integer>> & source) {
        std::shared_ptr<SharedTypeRingbuffer> rb = std::shared_ptr<SharedTypeRingbuffer>(new SharedTypeRingbuffer(source));
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());
        return rb;
    }

    std::vector<SharedType> createIntArray(const jau::nsize_t capacity, const jau::nsize_t startValue) {
        std::vector<SharedType> array(capacity);
        for(jau::nsize_t i=0; i<capacity; i++) {
            array[i] = SharedType(new Integer(startValue+i));
        }
        return array;
    }

    void readTestImpl(SharedTypeRingbuffer &rb, bool clearRef, jau::nsize_t capacity, jau::nsize_t len, jau::nsize_t startValue) {
        (void) clearRef;

        jau::nsize_t preSize = rb.getSize();
        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at read "+std::to_string(len)+" elems: "+rb.toString(), capacity >= len);
        REQUIRE_MSG("size at read "+std::to_string(len)+" elems: "+rb.toString(), preSize >= len);
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

        for(jau::nsize_t i=0; i<len; i++) {
            SharedType svI = rb.get();
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb.toString(), svI!=nullptr);
            REQUIRE_MSG("value at read #"+std::to_string(i+1)+": "+rb.toString(), startValue+i == svI->intValue());
        }

        REQUIRE_MSG("size "+rb.toString(), preSize-len == rb.getSize());
        REQUIRE_MSG("free slots after reading "+std::to_string(len)+": "+rb.toString(), rb.getFreeSlots()>= len);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());
    }

    void writeTestImpl(SharedTypeRingbuffer &rb, jau::nsize_t capacity, jau::nsize_t len, jau::nsize_t startValue) {
        jau::nsize_t preSize = rb.getSize();

        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at write "+std::to_string(len)+" elems: "+rb.toString(), capacity >= len);
        REQUIRE_MSG("size at write "+std::to_string(len)+" elems: "+rb.toString(), preSize+len <= capacity);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());

        for(jau::nsize_t i=0; i<len; i++) {
            std::string m = "buffer put #"+std::to_string(i)+": "+rb.toString();
            REQUIRE_MSG(m, rb.put( SharedType( new Integer(startValue+i) ) ) );
        }

        REQUIRE_MSG("size "+rb.toString(), preSize+len == rb.getSize());
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
    }

    void moveGetPutImpl(SharedTypeRingbuffer &rb, jau::nsize_t pos) {
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
        for(jau::nsize_t i=0; i<pos; i++) {
            REQUIRE_MSG("moveFull.get "+rb.toString(), i == rb.get()->intValue());
            REQUIRE_MSG("moveFull.put "+rb.toString(), rb.put( SharedType( new Integer(i) ) ) );
        }
    }

    void movePutGetImpl(SharedTypeRingbuffer &rb, jau::nsize_t pos) {
        REQUIRE_MSG("RB is full "+rb.toString(), !rb.isFull());
        for(jau::nsize_t i=0; i<pos; i++) {
            REQUIRE_MSG("moveEmpty.put "+rb.toString(), rb.put( SharedType( new Integer(600+i) ) ) );
            REQUIRE_MSG("moveEmpty.get "+rb.toString(), 600+i == rb.get()->intValue());
        }
    }

  public:

    void test01_FullRead() {
        jau::nsize_t capacity = 11;
        std::vector<SharedType> source = createIntArray(capacity, 0);
        std::shared_ptr<SharedTypeRingbuffer> rb = createFull(source);
        INFO_STR("test01_FullRead: Created / "+ rb->toString());
        REQUIRE_MSG("full size "+rb->toString(), capacity == rb->getSize());
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        readTestImpl(*rb, true, capacity, capacity, 0);
        INFO_STR("test01_FullRead: PostRead / " + rb->toString());
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());
    }

    void test02_EmptyWrite() {
        jau::nsize_t capacity = 11;
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
        INFO( std::string("test01_EmptyWrite: Created / ") + rb->toString().c_str());
        REQUIRE_MSG("zero size "+rb->toString(), 0 == rb->getSize());
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());

        writeTestImpl(*rb, capacity, capacity, 0);
        INFO( std::string("test01_EmptyWrite: PostWrite / ") + rb->toString().c_str());
        REQUIRE_MSG("full size "+rb->toString(), capacity == rb->getSize());
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        readTestImpl(*rb, true, capacity, capacity, 0);
        INFO( std::string("test01_EmptyWrite: PostRead / ") + rb->toString().c_str());
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());
    }

    void test03_FullReadReset() {
        jau::nsize_t capacity = 11;
        std::vector<SharedType> source = createIntArray(capacity, 0);
        std::shared_ptr<SharedTypeRingbuffer> rb = createFull(source);
        INFO_STR("test01_FullReadReset: Created / " + rb->toString());
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        rb->reset(source);
        INFO_STR("test01_FullReadReset: Post Reset w/ source / " + rb->toString());
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        readTestImpl(*rb, false, capacity, capacity, 0);
        INFO_STR("test01_FullReadReset: Post Read / " + rb->toString());
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());

        rb->reset(source);
        INFO_STR("test01_FullReadReset: Post Reset w/ source / " + rb->toString());
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        readTestImpl(*rb, false, capacity, capacity, 0);
        INFO_STR("test01_FullReadReset: Post Read / " + rb->toString());
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());
    }

    void test04_EmptyWriteClear() {
        jau::nsize_t capacity = 11;
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());

        rb->clear();
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());

        writeTestImpl(*rb, capacity, capacity, 0);
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        readTestImpl(*rb, false, capacity, capacity, 0);
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());

        rb->clear();
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());

        writeTestImpl(*rb, capacity, capacity, 0);
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        readTestImpl(*rb, false, capacity, capacity, 0);
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());
    }

    void test05_ReadResetMid01() {
        jau::nsize_t capacity = 11;
        std::vector<SharedType> source = createIntArray(capacity, 0);
        std::shared_ptr<SharedTypeRingbuffer> rb = createFull(source);
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        rb->reset(source);
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        readTestImpl(*rb, false, capacity, 5, 0);
        REQUIRE_MSG("not empty "+rb->toString(), !rb->isEmpty());
        REQUIRE_MSG("not Full "+rb->toString(), !rb->isFull());

        rb->reset(source);
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        readTestImpl(*rb, false, capacity, capacity, 0);
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());
    }

    void test06_ReadResetMid02() {
        jau::nsize_t capacity = 11;
        std::vector<SharedType> source = createIntArray(capacity, 0);
        std::shared_ptr<SharedTypeRingbuffer> rb = createFull(source);
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        rb->reset(source);
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        moveGetPutImpl(*rb, 5);
        readTestImpl(*rb, false, capacity, 5, 5);
        REQUIRE_MSG("not empty "+rb->toString(), !rb->isEmpty());
        REQUIRE_MSG("not Full "+rb->toString(), !rb->isFull());

        rb->reset(source);
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        readTestImpl(*rb, false, capacity, capacity, 0);
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());
    }

  private:

    void test_GrowFullImpl(jau::nsize_t initialCapacity, jau::nsize_t pos) {
        jau::nsize_t growAmount = 5;
        jau::nsize_t grownCapacity = initialCapacity+growAmount;
        std::vector<SharedType> source = createIntArray(initialCapacity, 0);
        std::shared_ptr<SharedTypeRingbuffer> rb = createFull(source);

        for(jau::nsize_t i=0; i<initialCapacity; i++) {
            SharedType svI = rb->get();
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb->toString(), svI!=nullptr);
            REQUIRE_MSG("value at read #"+std::to_string(i+1)+": "+rb->toString(), (0+i)%initialCapacity == svI->intValue());
        }
        REQUIRE_MSG("zero size "+rb->toString(), 0 == rb->getSize());

        rb->reset(source);
        REQUIRE_MSG("orig size "+rb->toString(), initialCapacity == rb->getSize());

        moveGetPutImpl(*rb, pos);
        // PRINTM("X02 "+rb->toString());
        // rb->dump(stderr, "X02");

        rb->recapacity(grownCapacity);
        REQUIRE_MSG("capacity "+rb->toString(), grownCapacity == rb->capacity());
        REQUIRE_MSG("orig size "+rb->toString(), initialCapacity == rb->getSize());
        REQUIRE_MSG("not full "+rb->toString(), !rb->isFull());
        REQUIRE_MSG("not empty "+rb->toString(), !rb->isEmpty());
        // PRINTM("X03 "+rb->toString());
        // rb->dump(stderr, "X03");

        for(jau::nsize_t i=0; i<growAmount; i++) {
            REQUIRE_MSG("buffer not full at put #"+std::to_string(i)+": "+rb->toString(), rb->put( SharedType( new Integer(100+i) ) ) );
        }
        REQUIRE_MSG("new size "+rb->toString(), grownCapacity == rb->getSize());
        REQUIRE_MSG("full-1 "+rb->toString(), rb->isFull());
        REQUIRE_MSG("full-2 "+rb->toString(), rb->isFull2());

        for(jau::nsize_t i=0; i<initialCapacity; i++) {
            SharedType svI = rb->get();
            // PRINTM("X05["+std::to_string(i)+"]: "+rb->toString()+", svI-null: "+std::to_string(svI==nullptr));
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb->toString(), svI!=nullptr);
            REQUIRE_MSG("value at read #"+std::to_string(i+1)+": "+rb->toString(), (pos+i)%initialCapacity == svI->intValue());
        }

        for(jau::nsize_t i=0; i<growAmount; i++) {
            SharedType svI = rb->get();
            // PRINTM("X07["+std::to_string(i)+"]: "+rb->toString()+", svI-null: "+std::to_string(svI==nullptr));
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb->toString(), svI!=nullptr);
            REQUIRE_MSG("value at read #"+std::to_string(i+1)+": "+rb->toString(), 100+i == svI->intValue());
        }

        REQUIRE_MSG("zero size "+rb->toString(), 0 == rb->getSize());
        REQUIRE_MSG("empty-1 "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty-2 "+rb->toString(), rb->isEmpty2());

        REQUIRE_MSG("not full "+rb->toString(), !rb->isFull());
    }

  public:

    void test20_GrowFull01_Begin() {
        test_GrowFullImpl(11, 0);
    }
    void test21_GrowFull02_Begin1() {
        test_GrowFullImpl(11, 0+1);
    }
    void test22_GrowFull03_Begin2() {
        test_GrowFullImpl(11, 0+2);
    }
    void test23_GrowFull04_Begin3() {
        test_GrowFullImpl(11, 0+3);
    }
    void test24_GrowFull05_End() {
        test_GrowFullImpl(11, 11-1);
    }
    void test25_GrowFull11_End1() {
        test_GrowFullImpl(11, 11-1-1);
    }
    void test26_GrowFull12_End2() {
        test_GrowFullImpl(11, 11-1-2);
    }
    void test27_GrowFull13_End3() {
        test_GrowFullImpl(11, 11-1-3);
    }

};

METHOD_AS_TEST_CASE( TestRingbuffer01::test01_FullRead,          "Test TestRingbuffer 01- 01");
METHOD_AS_TEST_CASE( TestRingbuffer01::test02_EmptyWrite,        "Test TestRingbuffer 01- 02");
METHOD_AS_TEST_CASE( TestRingbuffer01::test03_FullReadReset,     "Test TestRingbuffer 01- 03");
METHOD_AS_TEST_CASE( TestRingbuffer01::test04_EmptyWriteClear,   "Test TestRingbuffer 01- 04");
METHOD_AS_TEST_CASE( TestRingbuffer01::test05_ReadResetMid01,    "Test TestRingbuffer 01- 05");
METHOD_AS_TEST_CASE( TestRingbuffer01::test06_ReadResetMid02,    "Test TestRingbuffer 01- 06");
METHOD_AS_TEST_CASE( TestRingbuffer01::test20_GrowFull01_Begin,  "Test TestRingbuffer 01- 20");
METHOD_AS_TEST_CASE( TestRingbuffer01::test21_GrowFull02_Begin1, "Test TestRingbuffer 01- 21");
METHOD_AS_TEST_CASE( TestRingbuffer01::test22_GrowFull03_Begin2, "Test TestRingbuffer 01- 22");
METHOD_AS_TEST_CASE( TestRingbuffer01::test23_GrowFull04_Begin3, "Test TestRingbuffer 01- 23");
METHOD_AS_TEST_CASE( TestRingbuffer01::test24_GrowFull05_End,    "Test TestRingbuffer 01- 24");
METHOD_AS_TEST_CASE( TestRingbuffer01::test25_GrowFull11_End1,   "Test TestRingbuffer 01- 25");
METHOD_AS_TEST_CASE( TestRingbuffer01::test26_GrowFull12_End2,   "Test TestRingbuffer 01- 26");
METHOD_AS_TEST_CASE( TestRingbuffer01::test27_GrowFull13_End3,   "Test TestRingbuffer 01- 27");

