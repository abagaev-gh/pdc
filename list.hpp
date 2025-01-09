#pragma once

#include "persistent_structure.hpp"
#include "fat_nodes.hpp"

#include <list>
#include <memory>
#include <iostream>

namespace pdc {

using namespace internal;

template <typename T>
class List : public Persisent<List<T>> {
  mutable std::shared_ptr<std::list<FatNodes<T>>> list_;
  std::size_t version_ = 0;
  mutable std::shared_ptr<std::size_t> max_version_;
  mutable std::shared_ptr<FatNodes<std::size_t>> size_;
public:
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using difference_type = typename std::list<FatNodes<T>>::difference_type;
  using size_type = typename std::list<FatNodes<T>>::size_type;

  class Iterator {
    friend class List<T>;
    const List<T>* master_;
    using list_iterator = typename std::list<FatNodes<T>>::iterator;
    list_iterator it_;
    Iterator(const list_iterator& it, const List<T>* master) 
      : master_(master), it_(SkipUnavailable(it)) { }
  public:
    Iterator operator++();
    const_reference operator*();
    bool operator==(const Iterator& rhs) { return it_ == rhs.it_; }
    bool operator!=(const Iterator& rhs) { return it_ != rhs.it_; }
  private:
    list_iterator SkipUnavailable(const list_iterator& it) const;
  };
  friend class Iterator;
  
  using iterator = Iterator;

  List();
  std::size_t Size() const { return size_->Get(version_).value; }
  List<T> PushBack(T value) const;
  List<T> PushFront(T value) const;
  List<T> Insert(iterator pos, T value) const;
  
  iterator begin() const { return iterator(list_->begin(), this); }
  iterator end() const { return iterator(list_->end(), this); }

  List<T> Undo() const override
    { return List<T>(*this, version_ > 0 ? version_ - 1 : version_); }
  List<T> Redo() const override
    { return List<T>(*this, version_ < *max_version_ ? version_ + 1 : version_); }

private:
  List(const List<T>& other, std::size_t version);
};

template <typename T>
List<T>::List()
  : list_(std::make_shared<std::list<FatNodes<T>>>())
  , max_version_(std::make_shared<std::size_t>(0))
  , size_(std::make_shared<FatNodes<std::size_t>>(0, 0))
{
}

template <typename T>
List<T>::List(const List<T>& other, std::size_t version)
  : list_(other.list_)
  , version_(version)
  , max_version_(other.max_version_)
  , size_(other.size_)
{
}

template <typename T>
List<T> List<T>::PushBack(T value) const
{
  ++(*max_version_);
  list_->emplace_back(*max_version_, std::move(value));
  size_->Add((*max_version_), Size() + 1);
  return List<T>(*this, *max_version_);
}

template <typename T>
List<T> List<T>::PushFront(T value) const
{
  ++(*max_version_);
  list_->emplace_front(*max_version_, std::move(value));
  size_->Add((*max_version_), Size() + 1);
  return List<T>(*this, *max_version_);
}

template <typename T>
List<T> List<T>::Insert(typename List<T>::iterator pos, T value) const
{
  ++(*max_version_);
  list_->insert(pos.it_, FatNodes<T>{*max_version_, std::move(value)});
  size_->Add((*max_version_), Size() + 1);
  return List<T>(*this, *max_version_);
}

/////////////////////////////////////////////

template <typename T>
typename List<T>::Iterator List<T>::Iterator::operator++()
{
  it_ = SkipUnavailable(++it_);
  return *this;
}

template <typename T>
typename List<T>::const_reference List<T>::Iterator::operator*()
{
  return it_->Get(master_->version_).value;
}

template <typename T>
typename List<T>::Iterator::list_iterator 
List<T>::Iterator::SkipUnavailable(const list_iterator& it) const
{
  auto out = it;
  while (out != master_->list_->end()) {
    if (out->HasItem(master_->version_)) {
      break;
    }
    ++out;
  }
  return out;
}

} // namespace pdc