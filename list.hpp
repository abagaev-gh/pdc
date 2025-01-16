#pragma once

#include "persistent_structure.hpp"
#include "exception.hpp"

#include <list>
#include <memory>
#include <iostream>
#include <mutex>
#include <atomic>

namespace pdc {

/*! \brief Partially persistent double-linked list. */
template <typename T>
class List : public Persisent<List<T>> {
  struct Node {
    T value;
    std::size_t version;
    bool is_deleted = false;
    std::unique_ptr<std::mutex> mutex;
    Node(std::size_t ver, T val) 
      : value(val), version(ver), mutex(std::make_unique<std::mutex>()) { }
  };
  mutable std::shared_ptr<std::list<Node>> list_;
  std::size_t version_ = 0;
  mutable std::shared_ptr<std::atomic<std::size_t>> max_version_;
  mutable std::shared_ptr<std::mutex> mutex_;
public:
  /*! \brief Iterator for list bypass. */
  class Iterator {
    friend class List<T>;
    const List<T>* master_;
    using list_iterator = typename std::list<Node>::iterator;
    list_iterator it_;
    Iterator(const list_iterator& it, const List<T>* master) 
      : master_(master), it_(SkipUnavailable(it, true)) { }
  public:
    Iterator operator++();
    Iterator operator--();
    const T& operator*() const;
    bool operator==(const Iterator& rhs) { return it_ == rhs.it_; }
    bool operator!=(const Iterator& rhs) { return it_ != rhs.it_; }
  private:
    list_iterator SkipUnavailable(const list_iterator& it, bool forward) const;
  };
  friend class Iterator;
  
  /*! \brief Default constructor. Create empty List. */
  List();

  /*! \brief List empty? 
   *
   * \return true if the List is empty, otherwise false.
   */
  bool IsEmpty() const { std::lock_guard<std::mutex> l(*mutex_); return begin() == end(); }

  /*! \brief Size of List. 
   *
   * Complexity: O(N).
   * \return List size. 
   */
  std::size_t Size() const;

  /*! \brief Add a value at the end of the List.
   *
   * \param value Value to add.
   * \return New version of the List with changed state.
   * \exception IncorrectVersionException 
   *            If the method is not called on the latest version of the list.
   */
  List<T> PushBack(T value) const;

  /*! \brief Add a value at the front of the List.
   *
   * \param value Value to add.
   * \return New version of the List with changed state.
   * \exception IncorrectVersionException 
   *            If the method is not called on the latest version of the list.
   */
  List<T> PushFront(T value) const;

  /*! \brief Insert element at the specified location in the List. 
   *
   * \param pos The position before which you want to insert a new value.
   * \param value Value to insert.
   * \return New version of the List with changed state.
   * \exception IncorrectVersionException 
   *            If the method is not called on the latest version of the list.
   */
  List<T> Insert(const Iterator& pos, T value) const;

  /*! \brief Remove element at the specified location in the List. 
   *
   * \param pos Iterator indicating the element to be removed.
   * \return New version of the List with changed state.
   * \exception IncorrectVersionException 
   *            If the method is not called on the latest version of the list.
   */
  List<T> Remove(const Iterator& pos) const;
  
  /*! \brief STL-based begin(). 
   *
   * Returns the iterator pointing to the begin of the List.
   * \return Iterator pointing to the begin of the List.
   */
  Iterator begin() const { return Iterator(list_->begin(), this); }

  /*! \brief STL-based end(). 
   *
   * Returns the iterator pointing to the end of the List.
   * \return Iterator pointing to the end of the List.
   */
  Iterator end() const { return Iterator(list_->end(), this); }

  /*! \brief Returns the previous version of the List.
   *
   * Returns the same version of the List if the version is minimal.
   * \return Previous version of the List.
   */
  List<T> Undo() const override
    { return List<T>(*this, version_ > 0 ? version_ - 1 : version_); }

  /*! \brief Returns the next version of the List.
   *
   * Returns the same version of the List if the version is maximum.
   * \return Next version of the List.
   */
  List<T> Redo() const override
    { return List<T>(*this, version_ < *max_version_ ? version_ + 1 : version_); }

private:
  List(const List<T>& other, std::size_t version);
  void CheckVersion() const;
};

template <typename T>
List<T>::List()
  : list_(std::make_shared<std::list<Node>>())
  , max_version_(std::make_shared<std::atomic<std::size_t>>(0))
  , mutex_(std::make_shared<std::mutex>())
{
}

template <typename T>
List<T>::List(const List<T>& other, std::size_t version)
  : list_(other.list_)
  , version_(version)
  , max_version_(other.max_version_)
  , mutex_(other.mutex_)
{
}

template <typename T>
std::size_t List<T>::Size() const
{
  std::lock_guard<std::mutex> l(*mutex_);
  std::size_t res = 0;
  for ([[maybe_unused]] const auto& item : *this) {
    ++res;
  }
  return res;
}

template <typename T>
List<T> List<T>::PushBack(T value) const
{
  std::lock_guard<std::mutex> l(*mutex_);
  CheckVersion();
  ++(*max_version_);
  if (!list_->empty()) {
    std::lock_guard<std::mutex> l2(*((--list_->end())->mutex));
    list_->emplace_back(*max_version_, std::move(value));
    return List<T>(*this, *max_version_);
  }
  list_->emplace_back(*max_version_, std::move(value));
  return List<T>(*this, *max_version_);
}

template <typename T>
List<T> List<T>::PushFront(T value) const
{
  std::lock_guard<std::mutex> l(*mutex_);
  CheckVersion();
  ++(*max_version_);
  std::lock_guard<std::mutex> l2(*(list_->begin()->mutex));
  list_->emplace_front(*max_version_, std::move(value));
  return List<T>(*this, *max_version_);
}

template <typename T>
List<T> List<T>::Insert(const List<T>::Iterator& pos, T value) const
{
  std::lock_guard<std::mutex> l(*mutex_);
  CheckVersion();
  ++(*max_version_);
  std::lock_guard<std::mutex> l2(*(pos.it_->mutex));
  list_->insert(pos.it_, List<T>::Node(*max_version_, std::move(value)));
  return List<T>(*this, *max_version_);
}

template <typename T>
List<T> List<T>::Remove(const List<T>::Iterator& pos) const
{
  std::lock_guard<std::mutex> l(*mutex_);
  CheckVersion();
  ++(*max_version_);
  std::lock_guard<std::mutex> l2(*(pos.it_->mutex));
  pos.it_->is_deleted = true;
  return List<T>(*this, *max_version_);
}

template <typename T>
void List<T>::CheckVersion() const
{
  if (version_ != *max_version_) {
    throw IncorrectVersionException();
  }
}

/////////////////////////////////////////////

template <typename T>
typename List<T>::Iterator List<T>::Iterator::operator++()
{
  it_ = SkipUnavailable(++it_, true);
  return *this;
}

template <typename T>
typename List<T>::Iterator List<T>::Iterator::operator--()
{
  it_ = SkipUnavailable(--it_, false);
  return *this;
}

template <typename T>
const T& List<T>::Iterator::operator*() const
{
  return it_->value;
}

template <typename T>
typename List<T>::Iterator::list_iterator
List<T>::Iterator::SkipUnavailable(
  const List<T>::Iterator::list_iterator& it, bool forward) const
{
  auto out = it;
  List<T>::Iterator::list_iterator end = forward ? master_->list_->end() 
                                                 : master_->list_->begin();
  while (out != end) {
    std::lock_guard<std::mutex> l(*(out->mutex));
    if (out->version == master_->version_) { // can't be deleted
      break;
    }
    if (out->version < master_->version_ && !out->is_deleted) {
      break;
    }
    forward ? ++out : --out;
  }
  return out;
}

} // namespace pdc