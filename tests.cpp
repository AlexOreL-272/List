#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <tuple>
#include <type_traits>
#include <string>

#include "list.hpp"
//#include "memory_utils.hpp"
#include "utils.hpp"

template <typename T, typename U>
void EXPECT_EQ(T l, U r) {
  std::cout << std::boolalpha << (l == r) << '\n';
}

void EXPECT_TRUE(bool a) { std::cout << std::boolalpha << a << '\n'; }

void EXPECT_FALSE(bool a) { std::cout << std::boolalpha << !a << '\n'; }

void ASSERT_TRUE(bool a) { std::cout << std::boolalpha << a << '\n'; }  // че

size_t MemoryManager::type_new_allocated = 0;
size_t MemoryManager::type_new_deleted = 0;
size_t MemoryManager::allocator_allocated = 0;
size_t MemoryManager::allocator_deallocated = 0;
size_t MemoryManager::allocator_constructed = 0;
size_t MemoryManager::allocator_destroyed = 0;

template <typename T, bool PropagateOnConstruct, bool PropagateOnAssign>
size_t WhimsicalAllocator<T, PropagateOnConstruct, PropagateOnAssign>::counter =
    0;

size_t Accountant::ctor_calls = 0;
size_t Accountant::dtor_calls = 0;

bool ThrowingAccountant::need_throw = false;

void SetupTest() {
  MemoryManager::type_new_allocated = 0;
  MemoryManager::type_new_deleted = 0;
  MemoryManager::allocator_allocated = 0;
  MemoryManager::allocator_deallocated = 0;
  MemoryManager::allocator_constructed = 0;
  MemoryManager::allocator_destroyed = 0;
}

template <typename Iterator, typename T>

void IteratorTest() {
  std::cout << "Checking iterators: \n";
  using traits = std::iterator_traits<Iterator>;

  {
    auto test = std::is_same_v<std::remove_cv_t<typename traits::value_type>,
                               std::remove_cv_t<T>>;
    EXPECT_TRUE(test);
  }

  {
    auto test = std::is_same_v<typename traits::pointer, T*>;
    EXPECT_TRUE(test);
  }

  {
    auto test = std::is_same_v<typename traits::reference, T&>;
    EXPECT_TRUE(test);
  }

  {
    auto test = std::is_same_v<typename traits::iterator_category,
                               std::bidirectional_iterator_tag>;
    EXPECT_TRUE(test);
  }

  {
    auto test = std::is_same_v<decltype(std::declval<Iterator>()++), Iterator>;
    EXPECT_TRUE(test);
  }

  {
    auto test = std::is_same_v<decltype(++std::declval<Iterator>()), Iterator&>;
    EXPECT_TRUE(test);
  }

  {
    auto test = std::is_same_v<decltype(*std::declval<Iterator>()), T&>;
    EXPECT_TRUE(test);
  }

  {
    auto test = std::is_same_v<
        decltype(std::declval<Iterator>() == std::declval<Iterator>()), bool>;
    EXPECT_TRUE(test);
  }

  {
    auto test = std::is_same_v<
        decltype(std::declval<Iterator>() != std::declval<Iterator>()), bool>;
    EXPECT_TRUE(test);
  }
}

void OTHER() {
  {
    IteratorTest<List<int>::iterator, int>();
    IteratorTest<decltype(std::declval<List<int>>().rbegin()), int>();
    IteratorTest<decltype(std::declval<List<int>>().cbegin()), const int>();
  }
}

void STATIC_ASSERTS() {
  std::cout << "Checking asserts: \n";
  {
    auto test = std::is_same_v<List<int>::value_type, int>;
    EXPECT_TRUE(test);
  }

  {
    auto test =
        std::is_same_v<List<int, AllocatorWithCount<int>>::allocator_type,
                       AllocatorWithCount<int>>;
    EXPECT_TRUE(test);
  }

  {
    auto test = std::is_same_v<List<int>::allocator_type, std::allocator<int>>;
    EXPECT_TRUE(test);
  }
}

void CONSTRUCTOR() {
  std::cout << "Checking constructors: \n";
  {
    SetupTest();
    {
      List<TypeWithFancyNewDeleteOperators> l;

      ASSERT_TRUE(l.size() == 0);
      ASSERT_TRUE(l.empty());
      ASSERT_TRUE(MemoryManager::type_new_allocated == 0);
    }
    ASSERT_TRUE(MemoryManager::type_new_deleted == 0);
  }
  {
    SetupTest();
    {
      constexpr size_t kSize = 10;
      List<TypeWithFancyNewDeleteOperators> l(kSize);
      ASSERT_TRUE(l.size() == kSize);
      ASSERT_TRUE(MemoryManager::type_new_allocated == 0);
    }
    ASSERT_TRUE(MemoryManager::type_new_deleted == 0);
  }

  {
    SetupTest();
    AllocatorWithCount<TypeWithFancyNewDeleteOperators> alloc;
    constexpr size_t kSize = 10;
    {
      List<TypeWithFancyNewDeleteOperators,
           AllocatorWithCount<TypeWithFancyNewDeleteOperators>>
          l(kSize, alloc);
      ASSERT_TRUE(l.size() == kSize);
      ASSERT_TRUE(MemoryManager::type_new_allocated == 0);
      ASSERT_TRUE(MemoryManager::type_new_deleted == 0);
      ASSERT_TRUE(MemoryManager::allocator_allocated != 0);
      ASSERT_TRUE(MemoryManager::allocator_constructed == kSize);
      ASSERT_TRUE(alloc.allocator_allocated == 0);
      ASSERT_TRUE(alloc.allocator_constructed == 0);
    }
    ASSERT_TRUE(MemoryManager::allocator_deallocated != 0);
    ASSERT_TRUE(MemoryManager::allocator_destroyed == kSize);
    ASSERT_TRUE(alloc.allocator_destroyed == 0);
    ASSERT_TRUE(alloc.allocator_deallocated == 0);
  }

  {
    SetupTest();
    constexpr size_t kSize = 10;
    AllocatorWithCount<TypeWithFancyNewDeleteOperators> alloc;
    TypeWithFancyNewDeleteOperators default_value(1);
    List<TypeWithFancyNewDeleteOperators,
         AllocatorWithCount<TypeWithFancyNewDeleteOperators>>
        l(kSize, default_value, alloc);
    ASSERT_TRUE(l.size() == kSize);
    ASSERT_TRUE(MemoryManager::type_new_allocated == 0);
    ASSERT_TRUE(MemoryManager::type_new_deleted == 0);
    ASSERT_TRUE(MemoryManager::allocator_allocated != 0);
    ASSERT_TRUE(MemoryManager::allocator_constructed == kSize);
    ASSERT_TRUE(alloc.allocator_allocated == 0);
    ASSERT_TRUE(alloc.allocator_constructed == 0);

    for (auto it = l.begin(); it != l.end(); ++it) {
      ASSERT_TRUE(it->value == default_value.value);
    }
  }
  {
    SetupTest();
    AllocatorWithCount<int> alloc;
    List<int, AllocatorWithCount<int>> l({1, 2, 3, 4, 5}, alloc);
    ASSERT_TRUE(l.size() == 5);
    ASSERT_TRUE(MemoryManager::allocator_allocated != 0);
    ASSERT_TRUE(MemoryManager::allocator_constructed == 5);
    ASSERT_TRUE(alloc.allocator_allocated == 0);
    ASSERT_TRUE(alloc.allocator_constructed == 0);
  }

  {
    SetupTest();
    const size_t size = 5;
    List<TypeWithCounts, AllocatorWithCount<TypeWithCounts>> l1 = {1, 2, 3, 4, 5};

    List<TypeWithCounts, AllocatorWithCount<TypeWithCounts>> l2(l1);

    ASSERT_TRUE(l2.size() == size);
    ASSERT_TRUE(MemoryManager::allocator_allocated != 0);
    ASSERT_TRUE(MemoryManager::allocator_constructed == size * 2);

    ASSERT_TRUE(AreListsEqual(l1, l2));
    for (auto& value : l1) {
      ASSERT_TRUE(*value.copy_c == 2);
    }
  }
}

void OPERATORS() {
  std::cout << "Checking operators: \n";
  {
    SetupTest();
    const size_t size = 5;
    List<TypeWithCounts> l1 = {1, 2, 3, 4, 5};
    List<TypeWithCounts> l2;
    l2 = l1;
    ASSERT_TRUE(l2.size() == size);
    ASSERT_TRUE(AreListsEqual(l1, l2));
    for (auto& value : l1) {
      ASSERT_TRUE(*value.copy_c == 2);
    }
  }
}

void BASIC_FUNC() {
  SetupTest();
  List<int> lst;

  ASSERT_TRUE(lst.empty());

  lst.push_back(3);
  lst.push_back(4);
  lst.push_front(2);
  lst.push_back(5);
  lst.push_front(1);

  std::reverse(lst.begin(), lst.end());

  ASSERT_TRUE(lst.size() == 5);

  std::string s;
  for (int x : lst) {
    s += std::to_string(x);
  }
  ASSERT_TRUE(s == "54321");
}

void PROPAGATE() {
  {
    SetupTest();
    List<int, WhimsicalAllocator<int, true, true>> lst;

    lst.push_back(1);
    lst.push_back(2);

//    std::cout << "After push back: " <<  lst << '\n';

    auto copy = lst;

//    std::cout << "Copy: " <<  copy << '\n';

    EXPECT_TRUE(copy.get_allocator() != lst.get_allocator());

    lst = copy;

//    std::cout << "After lst = copy: " <<  lst << '\n';
    EXPECT_TRUE(copy.get_allocator() == lst.get_allocator());
  }

  {
    SetupTest();
    List<int, WhimsicalAllocator<int, false, false>> lst;

    lst.push_back(1);
    lst.push_back(2);

    auto copy = lst;
    EXPECT_TRUE(copy.get_allocator() == lst.get_allocator());

    lst = copy;
    EXPECT_TRUE(copy.get_allocator() == lst.get_allocator());
  }

  {
    SetupTest();
    List<int, WhimsicalAllocator<int, true, false>> lst;

    lst.push_back(1);
    lst.push_back(2);

    auto copy = lst;
    EXPECT_TRUE(copy.get_allocator() != lst.get_allocator());

    lst = copy;
    EXPECT_TRUE(copy.get_allocator() != lst.get_allocator());
  }
}

void ACCOUNTANT() {
  {
    Accountant::reset();
    {
      List<Accountant> lst(5);
      ASSERT_TRUE(lst.size() == 5);
      ASSERT_TRUE(Accountant::ctor_calls == 5);

      List<Accountant> another = lst;
      ASSERT_TRUE(another.size() == 5);
      ASSERT_TRUE(Accountant::ctor_calls == 10);
      ASSERT_TRUE(Accountant::dtor_calls == 0);

      another.pop_back();
      another.pop_front();
      ASSERT_TRUE(Accountant::dtor_calls == 2);

      lst = another;  // dtor_calls += 5, ctor_calls += 3
      ASSERT_TRUE(another.size() == 3);
      ASSERT_TRUE(lst.size() == 3);

      ASSERT_TRUE(Accountant::ctor_calls == 13);
      ASSERT_TRUE(Accountant::dtor_calls == 7); // TODO

      std::cout << "Destructor calls (7): " << Accountant::dtor_calls << '\n';

    }  // dtor_calls += 6

    ASSERT_TRUE(Accountant::ctor_calls == 13);
    ASSERT_TRUE(Accountant::dtor_calls == 13); // TODO

    std::cout << "Destructor calls (13): " << Accountant::dtor_calls << '\n';
  }
}

void EXCEPTS() {
  Accountant::reset();

  ThrowingAccountant::need_throw = true;

  try {
    List<ThrowingAccountant> lst(8);
  } catch (...) {
    ASSERT_TRUE(Accountant::ctor_calls == 4);
    ASSERT_TRUE(Accountant::dtor_calls == 4);
  }

  ThrowingAccountant::need_throw = false;
  List<ThrowingAccountant> lst(8);

  List<ThrowingAccountant> lst2;
  for (int i = 0; i < 13; ++i) {
    lst2.push_back(i);
  }

  Accountant::reset();
  ThrowingAccountant::need_throw = true;

  try {
    auto lst3 = lst2;
  } catch (...) {
    ASSERT_TRUE(Accountant::ctor_calls == 4);
    ASSERT_TRUE(Accountant::dtor_calls == 4);
  }

  Accountant::reset();

  try {
    lst = lst2;
  } catch (...) {
    ASSERT_TRUE(Accountant::ctor_calls == 4);
    ASSERT_TRUE(Accountant::dtor_calls == 4);

    // Actually it may not be 8 (although de facto it is), but the only thing we
    // can demand here is the abscence of memory leaks

     EXPECT_TRUE(lst.size() == 8);
  }
}
