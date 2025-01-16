#include <CppUTest/TestHarness.h>

#include <thread>

#include "../array.hpp"
#include "../list.hpp"


TEST_GROUP(Array)
{
};

TEST(Array, Size)
{
  pdc::Array<int> array;
  UNSIGNED_LONGS_EQUAL(0, array.Size());

  const auto array2 = array.PushBack(0);
  UNSIGNED_LONGS_EQUAL(1, array2.Size());

  const auto array3 = array2.Update(0, 1);
  UNSIGNED_LONGS_EQUAL(1, array3.Size());

  const auto array4 = array2.Undo();
  UNSIGNED_LONGS_EQUAL(0, array4.Size());

  pdc::Array<int> large_array;
  for (int i = 0; i < 1000; ++i) {
    large_array = large_array.PushBack(i);
  }
  UNSIGNED_LONGS_EQUAL(1000, large_array.Size());
}

TEST(Array, IsEmpty)
{
  pdc::Array<int> array;
  CHECK(array.IsEmpty());

  const auto array2 = array.PushBack(1);
  CHECK_FALSE(array2.IsEmpty());

  const auto array3 = array2.Undo();
  CHECK(array3.IsEmpty());
}

TEST(Array, Update)
{
  pdc::Array<int> array(2, 0);
  const auto array2 = array.Update(0, 1);
  LONGS_EQUAL(1, array2[0]);

  const auto array3 = array2.Update(1, 2);
  LONGS_EQUAL(2, array3[1]);

  CHECK_THROWS(std::out_of_range, array3.Update(2, 0));
  CHECK_THROWS(std::out_of_range, array3.Update(-1, 0));

  pdc::Array<int> large_array(10, 0);
  for (std::size_t i = 0; i < large_array.Size(); ++i) {
    large_array = large_array.Update(i, i);
    LONGS_EQUAL(i, large_array[i]);
  }
}

TEST(Array, PushBack)
{
  pdc::Array<int> array;
  for (int i = 0; i < 10; ++i) {
    array = array.PushBack(i);
    LONGS_EQUAL(i, array[i]);
    UNSIGNED_LONGS_EQUAL(i+1, array.Size());
  }
}

TEST(Array, Operator)
{
  pdc::Array<int> array(2, 1);
  LONGS_EQUAL(1, array[0]);
  LONGS_EQUAL(1, array[1]);

  array = array.Update(0, 2);
  LONGS_EQUAL(2, array[0]);

  array = array.Undo();
  LONGS_EQUAL(1, array[0]);

  array = array.Redo();
  LONGS_EQUAL(2, array[0]);

  array = array.PushBack(3);
  LONGS_EQUAL(3, array[2]);
}

TEST(Array, Undo)
{
  pdc::Array<int> array;
  array = array.Undo();
  CHECK(array.IsEmpty());

  array = array.PushBack(0);
  CHECK_FALSE(array.IsEmpty());

  array = array.Undo();
  CHECK(array.IsEmpty());

  array = array.Redo();

  array = array.PushBack(1);
  UNSIGNED_LONGS_EQUAL(2, array.Size());
  array = array.PushBack(2);
  UNSIGNED_LONGS_EQUAL(3, array.Size());
  LONGS_EQUAL(0, array[0]);
  LONGS_EQUAL(1, array[1]);
  LONGS_EQUAL(2, array[2]);

  array = array.Undo();
  UNSIGNED_LONGS_EQUAL(2, array.Size());
  LONGS_EQUAL(0, array[0]);
  LONGS_EQUAL(1, array[1]);

  array = array.Undo();
  UNSIGNED_LONGS_EQUAL(1, array.Size());
  LONGS_EQUAL(0, array[0]);
}

TEST(Array, Redo)
{
  pdc::Array<int> array;
  array = array.Redo();
  CHECK(array.IsEmpty());

  array = array.PushBack(0);
  array = array.Undo();
  array = array.Redo();
  UNSIGNED_LONGS_EQUAL(1, array.Size());

  array = array.PushBack(1);
  array = array.Undo();
  array = array.Undo();

  array = array.Redo();
  UNSIGNED_LONGS_EQUAL(1, array.Size());
  LONGS_EQUAL(0, array[0]);

  array = array.Redo();
  UNSIGNED_LONGS_EQUAL(2, array.Size());
  LONGS_EQUAL(0, array[0]);
  LONGS_EQUAL(1, array[1]);
}

TEST_GROUP(List)
{
};

TEST(List, Constructor)
{
  pdc::List<int> list;
  CHECK(list.IsEmpty());
  UNSIGNED_LONGS_EQUAL(0, list.Size());
}

TEST(List, Size)
{
  pdc::List<int> list;
  UNSIGNED_LONGS_EQUAL(0, list.Size());

  list = list.PushBack(1);
  UNSIGNED_LONGS_EQUAL(1, list.Size());

  list = list.PushFront(2);
  UNSIGNED_LONGS_EQUAL(2, list.Size());

  list = list.Insert(++list.begin(), 3);
  UNSIGNED_LONGS_EQUAL(3, list.Size());

  list = list.Undo();
  UNSIGNED_LONGS_EQUAL(2, list.Size());

  list = list.Redo();
  UNSIGNED_LONGS_EQUAL(3, list.Size());
}

TEST(List, PushBack)
{
  pdc::List<int> list;
  list = list.PushBack(0);
  LONGS_EQUAL(0, *list.begin());

  list = list.PushBack(1);
  LONGS_EQUAL(0, *list.begin());
  LONGS_EQUAL(1, *(++list.begin()));
}

TEST(List, PushFront)
{
  pdc::List<int> list;
  list = list.PushFront(0);
  LONGS_EQUAL(0, *list.begin());
  UNSIGNED_LONGS_EQUAL(1, list.Size());

  list = list.PushFront(1);
  LONGS_EQUAL(1, *list.begin());
  LONGS_EQUAL(0, *(++list.begin()));
  UNSIGNED_LONGS_EQUAL(2, list.Size());
}

TEST(List, Insert)
{
  pdc::List<int> list;
  list = list.PushBack(0);
  list = list.PushBack(1);
  list = list.Insert(list.begin(), 2);
  LONGS_EQUAL(2, *list.begin());
  UNSIGNED_LONGS_EQUAL(3, list.Size());

  list = list.Insert(++list.begin(), 3);
  LONGS_EQUAL(3, *(++list.begin()));
  UNSIGNED_LONGS_EQUAL(4, list.Size());
}

TEST(List, Remove)
{
  pdc::List<int> list;
  list = list.PushBack(0);
  list = list.PushBack(1);
  list = list.Remove(list.begin());
  LONGS_EQUAL(1, *list.begin());
  UNSIGNED_LONGS_EQUAL(1, list.Size());

  list = list.PushFront(5);
  list = list.Remove(++list.begin());
  LONGS_EQUAL(5, *list.begin());
  UNSIGNED_LONGS_EQUAL(1, list.Size());
}

TEST(List, Undo)
{
  pdc::List<int> list;
  list = list.PushBack(0);
  list = list.PushBack(1);
  list = list.Undo();
  LONGS_EQUAL(0, *list.begin());
  UNSIGNED_LONGS_EQUAL(1, list.Size());

  list = list.Undo();
  UNSIGNED_LONGS_EQUAL(0, list.Size());
}

TEST(List, Redo)
{
  pdc::List<int> list;
  list = list.PushBack(0);
  list = list.PushBack(1);
  list = list.Undo();
  list = list.Undo();
  list = list.Redo();
  LONGS_EQUAL(0, *list.begin());
  UNSIGNED_LONGS_EQUAL(1, list.Size());

  list = list.Redo();
  UNSIGNED_LONGS_EQUAL(2, list.Size());
  LONGS_EQUAL(0, *list.begin());
  LONGS_EQUAL(1, *(++list.begin()));
}

// TEST(List, Threaded)
// {
//   pdc::List<int> list;

//   bool thread1_throw = false;
//   bool thread2_throw = false;

//   std::thread thread1([list, &thread1_throw]() mutable {
//     try {
//       for (int i = 0; i < 100; i++) {
//         list = list.PushBack(1);
//       }
//     } catch (const pdc::IncorrectVersionException&) {
//       thread1_throw = true;
//     }
//   });

//   std::thread thread2([list, &thread2_throw]() mutable {
//     try {
//       for (int i = 0; i < 100; i++) {
//         list = list.PushBack(1);
//       }
//     } catch (const pdc::IncorrectVersionException&) {
//       thread2_throw = true;
//     }
//   });

//   thread1.join();
//   thread2.join();
  
//   CHECK(thread1_throw || thread2_throw);
// }

