#pragma once

#include "persistent_structure.hpp"
#include "fat_nodes.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <exception>

namespace pdc {

using namespace internal;


template <typename T>
class Array : public Persisent<Array<T>> {
  mutable std::shared_ptr<std::vector<FatNodes<T>>> array_;
  std::size_t version_ = 0;
  mutable std::shared_ptr<std::size_t> max_version_;
  mutable std::shared_ptr<FatNodes<std::size_t>> size_;
public:
  Array();
  Array(std::size_t count);
  Array(std::size_t count, T value);
  
  std::size_t Size() const { return GetSize(version_); }
  bool IsEmpty() const { return Size() == 0; }
  Array<T> Update(std::size_t idx, T value) const;
  Array<T> PushBack(T value) const;

  const T& operator[](std::size_t idx) const 
    { return (*array_)[idx].Get(version_).value; }

  Array<T> Undo() const override
    { return Array<T>(*this, version_ > 0 ? version_ - 1 : version_); }
  Array<T> Redo() const override
    { return Array<T>(*this, version_ < *max_version_ ? version_ + 1 : version_); }

private:
  Array(const Array<T>& other, std::size_t version);
  std::size_t GetSize(std::size_t version) const { return size_->Get(version).value; }
};

template <typename T>
Array<T>::Array()
  : Array(0)
{
}

template <typename T>
Array<T>::Array(std::size_t count)
  : Array(count, T())
{
}

template <typename T>
Array<T>::Array(std::size_t count, T value)
  : array_(std::make_shared<std::vector<FatNodes<T>>>())
  , max_version_(std::make_shared<std::size_t>(0))
  , size_(std::make_shared<FatNodes<std::size_t>>(0, count))
{
  array_->reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    array_->emplace_back(version_, value);
  }
}

template <typename T>
Array<T>::Array(const Array<T>& other, std::size_t version)
  : array_(other.array_)
  , version_(version)
  , max_version_(other.max_version_)
  , size_(other.size_)
{
}

template <typename T>
Array<T> Array<T>::Update(std::size_t idx, T value) const
{
  if (idx < 0 || idx >= Size()) {
    throw std::out_of_range("Update");
  }
  (*array_)[idx].Add(++(*max_version_), std::move(value));
  return Array<T>(*this, *max_version_);
}

template <typename T>
Array<T> Array<T>::PushBack(T value) const
{
  ++(*max_version_);
  array_->emplace_back(*max_version_, value);
  size_->Add((*max_version_), GetSize((*max_version_)) + 1);
  return Array<T>(*this, *max_version_);
}

} // namespace pdc
