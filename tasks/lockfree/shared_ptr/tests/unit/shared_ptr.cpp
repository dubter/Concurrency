#include "../../atomic_shared_ptr.hpp"

#include <wheels/test/test_framework.hpp>

struct TestObject {
  TestObject(int v) : value(v) {}

  int value;
};

TEST_SUITE(SharedPtr) {
  SIMPLE_TEST(Basics) {
    auto sp = MakeShared<TestObject>(7);

    ASSERT_TRUE(sp);
    // Access data
    ASSERT_EQ((*sp).value, 7);
    ASSERT_EQ(sp->value, 7);

    sp.Reset();
    ASSERT_FALSE(sp);
  }

  SIMPLE_TEST(Copy) {
    auto sp1 = MakeShared<TestObject>(3);
    auto sp2 = sp1;

    ASSERT_TRUE(sp2);
    ASSERT_EQ(sp2->value, 3);

    SharedPtr<TestObject> sp3(sp2);

    ASSERT_TRUE(sp3);
    ASSERT_EQ(sp3->value, 3);
  }

  SIMPLE_TEST(Move) {
    auto sp1 = MakeShared<TestObject>(2);
    auto sp2 = std::move(sp1);
    ASSERT_FALSE(sp1);
    ASSERT_TRUE(sp2);
    ASSERT_EQ(sp2->value, 2);
  }

  SIMPLE_TEST(Share) {
    auto sp1 = MakeShared<TestObject>(3);
    auto sp2 = sp1;

    ASSERT_TRUE(sp2);
    ASSERT_EQ(sp2->value, 3);

    auto sp3 = std::move(sp1);

    ASSERT_TRUE(sp3);
    ASSERT_FALSE(sp1);

    ASSERT_EQ(sp3->value, 3);

    auto sp4 = sp3;
    sp3.Reset();

    ASSERT_TRUE(sp4);
    ASSERT_EQ(sp4->value, 3);
  }

  SIMPLE_TEST(NullPtrs) {
    SharedPtr<TestObject> sp0;
    ASSERT_FALSE(sp0);
    auto sp1 = sp0;
    SharedPtr<TestObject> sp2(sp0);
    SharedPtr<TestObject> sp3(std::move(sp0));

    ASSERT_FALSE(sp1);
    ASSERT_FALSE(sp2);
    ASSERT_FALSE(sp3);
  }

  struct ListNode {
    ListNode() = default;

    explicit ListNode(SharedPtr<ListNode> _next) : next(_next) {
    }

    SharedPtr<ListNode> next;
  };

  size_t ListLength(SharedPtr<ListNode> head) {
    size_t length = 0;
    while (head) {
      ++length;
      head = head->next;
    }
    return length;
  }

  SIMPLE_TEST(List) {
    SharedPtr<ListNode> head;
    {
      auto n1 = MakeShared<ListNode>();
      auto n2 = MakeShared<ListNode>(n1);
      auto n3 = MakeShared<ListNode>(n2);
      head = n3;
    }

    ASSERT_EQ(ListLength(head), 3);
  }
}

TEST_SUITE(AtomicSharedPtr) {
  SIMPLE_TEST(JustWorks) {
    AtomicSharedPtr<TestObject> asp;

    auto sp0 = asp.Load();
    ASSERT_FALSE(sp0);

    auto sp1 = MakeShared<TestObject>(8);

    asp.Store(sp1);

    auto sp2 = asp.Load();

    ASSERT_TRUE(sp1);
    ASSERT_TRUE(sp2);

    ASSERT_EQ(sp2->value, 8);

    auto sp3 = MakeShared<TestObject>(9);

    bool success = asp.CompareExchangeWeak(sp1, sp3);
    ASSERT_TRUE(success);

    auto sp4 = asp.Load();
    ASSERT_EQ(sp4->value, sp3->value);

    auto sp5 = MakeShared<TestObject>(7);
    bool failure = asp.CompareExchangeWeak(sp1, sp5);
    ASSERT_FALSE(failure);

    auto sp6 = asp.Load();
    ASSERT_EQ(sp6->value, sp3->value);
  }

  SIMPLE_TEST(Dtor) {
    auto sp = MakeShared<TestObject>(5);
    AtomicSharedPtr<TestObject> asp;
    asp.Store(sp);
    sp.Reset();
    ASSERT_FALSE(sp);
  }

  SIMPLE_TEST(TwoAsps) {
    AtomicSharedPtr<TestObject> asp1;
    AtomicSharedPtr<TestObject> asp2;

    SharedPtr<TestObject> sp0;
    auto sp1 = MakeShared<TestObject>(1);

    asp1.Store(sp1);
    asp2.CompareExchangeWeak(sp0, sp1);

    auto sp2 = asp1.Load();
    auto sp3 = asp2.Load();

    ASSERT_EQ(sp2->value, sp1->value);
    ASSERT_EQ(sp3->value, sp2->value);
  }

  TEST(OverflowStamp, wheels::test::TestOptions().AdaptTLToSanitizer()) {
    auto sp = MakeShared<TestObject>(11);
    AtomicSharedPtr<TestObject> asp;
    asp.Store(sp);

    std::vector<SharedPtr<TestObject>> sps;

    for (size_t i = 0; i < 1'000'000; ++i) {
      sps.push_back(asp.Load());
    }
  }
}
