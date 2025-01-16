#pragma once

#include "persistent_structure.hpp"
#include "fat_nodes.hpp"
#include "exception.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <exception>
#include <mutex>

namespace pdc {

using namespace internal;

/*! \brief Partially persistent array. */
template <typename T>
class Array : public Persisent<Array<T>> {
  mutable std::shared_ptr<std::vector<FatNodes<T>>> array_;
  std::size_t version_ = 0;
  mutable std::shared_ptr<std::size_t> max_version_;
  mutable std::shared_ptr<FatNodes<std::size_t>> size_;
  mutable std::shared_ptr<std::mutex> mutex_;
public:
  /*! \brief Default constructor. Create empty Array. */
  Array();

  /*! \brief Constructor with count of elements.
   *
   * \param count Count elements with default value.
   */
  Array(std::size_t count);

  /*! \brief Constructor with count of elements of a certain value.
   *
   * \param count Count of elements.
   * \param value The value to use for initialization.
   */
  Array(std::size_t count, T value);
  
  /*! \brief Size of array. 
   *
   * \return Array size. 
   */
  std::size_t Size() const 
    { std::lock_guard<std::mutex> lk(*mutex_); return GetSize(version_); }

  /*! \brief Array empty? 
   *
   * \return true if the Array is empty, otherwise false.
   */
  bool IsEmpty() const { return Size() == 0; }

  /*! \brief Updates the value of the Array element.
   *
   * \param idx The index of the element to be changed. 
   * \param value The new value of the element.
   * \return New version of the Array with changed state.
   * \exception IncorrectVersionException 
   *            If the method is not called on the latest version of the Array.
   */
  Array<T> Update(std::size_t idx, T value) const;

  /*! \brief Add a value at the end of the Array.
   *
   * \param value Value to add.
   * \return New version of the Array with changed state.
   * \exception IncorrectVersionException 
   *            If the method is not called on the latest version of the Array.
   */
  Array<T> PushBack(T value) const;

  /*! \brief Access the item for reading.
   *
   * \param idx The index of the element.
   * \return Element to reading.
   */
  T operator[](std::size_t idx) const 
    { std::lock_guard<std::mutex> lk(*mutex_); return (*array_)[idx].Get(version_).value; }

  /*! \brief Returns the previous version of the Array.
   *
   * Returns the same version of the Array if the version is minimal.
   * \return Previous version of the Array.
   */
  Array<T> Undo() const override
    { return Array<T>(*this, version_ > 0 ? version_ - 1 : version_); }

  /*! \brief Returns the next version of the Array.
   *
   * Returns the same version of the Array if the version is maximum.
   * \return Next version of the Array.
   */
  Array<T> Redo() const override
    { return Array<T>(*this, version_ < *max_version_ ? version_ + 1 : version_); }

private:
  Array(const Array<T>& other, std::size_t version);
  std::size_t GetSize(std::size_t version) const 
    { return size_->Get(version).value; }
  void CheckVersion() const 
    { if (version_ != *max_version_) throw IncorrectVersionException(); }
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
  , mutex_(std::make_shared<std::mutex>())
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
  std::lock_guard<std::mutex> lk(*mutex_);
  CheckVersion();
  if (idx < 0 || idx >= Size()) {
    throw std::out_of_range("Update");
  }
  (*array_)[idx].Add(++(*max_version_), std::move(value));
  return Array<T>(*this, *max_version_);
}

template <typename T>
Array<T> Array<T>::PushBack(T value) const
{
  std::lock_guard<std::mutex> lk(*mutex_);
  CheckVersion();
  ++(*max_version_);
  array_->emplace_back(*max_version_, value);
  size_->Add((*max_version_), GetSize((*max_version_)) + 1);
  return Array<T>(*this, *max_version_);
}

} // namespace pdc
