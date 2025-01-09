#pragma once

#include "persistent_structure.hpp"
#include "fat_nodes.hpp"

#include <map>
#include <memory>
#include <iostream>

namespace pdc {

using namespace internal;

template <typename K, typename V>
class Map : public Persisent<Map<K, V>> {
  mutable std::shared_ptr<std::map<K, FatNodes<V>>> map_;
  std::size_t version_ = 0;
  mutable std::shared_ptr<std::size_t> max_version_;
  mutable std::shared_ptr<FatNodes<std::size_t>> size_;
public:
  class Iterator {
    friend class Map<K,V>;
    const Map<K, V>* master_;
    using map_iterator = typename std::map<K, FatNodes<V>>::iterator;
    map_iterator it_;
    Iterator(const map_iterator& it, const Map<K, V>* master) 
      : master_(master), it_(SkipUnavailable(it)) { }
  public:
    Iterator operator++();
    std::tuple<const K&, const V&> operator*();
    bool operator==(const Iterator& rhs) { return it_ == rhs.it_; }
    bool operator!=(const Iterator& rhs) { return it_ != rhs.it_; }
  private:
    map_iterator SkipUnavailable(const map_iterator& it) const;
  };
  friend class Iterator;

  Map();
  const V& Get(const K& key) const;
  bool Contain(const K& key) const;
  Map<K, V> Insert(const K& key, const V& value) const;

  Iterator begin() const { return Iterator(map_->begin(), this); }
  Iterator end() const { return Iterator(map_->end(), this); }

  Map<K, V> Undo() const override
    { return Map<K, V>(*this, version_ > 0 ? version_ - 1 : version_); }
  Map<K, V> Redo() const override
    { return Map<K, V>(*this, version_ < *max_version_ ? version_ + 1 : version_); }
private:
  Map(const Map<K, V>& other, std::size_t version);
};  

template <typename K, typename V>
Map<K, V>::Map()
  : map_(std::make_shared<std::map<K, FatNodes<V>>>())
  , max_version_(std::make_shared<std::size_t>(0))
  , size_(std::make_shared<FatNodes<std::size_t>>(0, 0))
{
}

template <typename K, typename V>
Map<K, V>::Map(const Map<K, V>& other, std::size_t version)
  : map_(other.map_)
  , version_(version)
  , max_version_(other.max_version_)
  , size_(other.size_)
{
}

template <typename K, typename V>
const V& Map<K, V>::Get(const K& key) const
{
  return (*map_)[key].Get(version_).value;
}

template <typename K, typename V>
bool Map<K, V>::Contain(const K& key) const
{
  auto it = map_->find(key);
  if (it == map_->end()) {
    return false;
  }
  return it->second.HasItem(version_);
}

template <typename K, typename V>
Map<K, V> Map<K, V>::Insert(const K& key, const V& value) const
{
  ++(*max_version_);
  (*map_)[key].Add(*max_version_, value);
  return Map<K, V>(*this, *max_version_);
}

/////////////////////////////////////////////

template <typename K, typename V>
typename Map<K, V>::Iterator Map<K, V>::Iterator::operator++()
{
  it_ = SkipUnavailable(++it_);
  return *this;
}

template <typename K, typename V>
typename std::tuple<const K&, const V&> Map<K, V>::Iterator::operator*()
{
  std::cout << "operator*()" << std::endl;
  if (it_ == master_->map_->end()) {
    std::cout << '\t' << "it_ == end()" << std::endl;
  }
  std::cout << it_->first << std::endl;
  std::cout << it_->second.Get(master_->version_).value << std::endl;
  return std::make_tuple<const K&, const V&>(
    it_->first, it_->second.Get(master_->version_).value);
}

template <typename K, typename V>
typename Map<K, V>::Iterator::map_iterator 
Map<K, V>::Iterator::SkipUnavailable(const map_iterator& it) const
{
  auto out = it;
  while (out != master_->map_->end()) {
    if (out->second.HasItem(master_->version_)) {
      break;
    }
    ++out;
  }
  return out;
}

} // namespace pdc
