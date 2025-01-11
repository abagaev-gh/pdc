#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>
#include <stdexcept>


namespace internal {

template <typename T>
struct Node {
  Node() = default;
  Node(std::size_t ver, const T& val) : version(ver), value(val) { }
  std::size_t version = 0;
  T value;
  bool is_deleted = false;
};

template <typename T>
class FatNodes {  
  std::vector<Node<T>> nodes_;
public:
  FatNodes();
  FatNodes(const T& v);
  FatNodes(std::size_t version, const T& v);
  const Node<T>& Get(std::size_t version) const;
  Node<T>& Get(std::size_t version);
  void Add(std::size_t version, T value);
  void Remove(std::size_t version);
  bool HasItem(std::size_t version) const;
};

template <typename T>
FatNodes<T>::FatNodes()
  : FatNodes(T())
{
}

template <typename T>
FatNodes<T>::FatNodes(const T& v)
  : FatNodes(1, v)
{
}

template <typename T>
FatNodes<T>::FatNodes(std::size_t version, const T& v)
  : nodes_(1, {version, v})
{
}

template <typename T>
const Node<T>& FatNodes<T>::Get(std::size_t version) const
{
  auto it = std::find_if(nodes_.rbegin(), nodes_.rend(), 
    [&version](const Node<T>& node) {
      if (node.version <= version) {
        if (node.is_deleted) {
          throw("Not found  node");
        }
        return true;
      }
      return false;
    });
  if (it == nodes_.rend()) {
    throw std::runtime_error("Not found node");
  }
  return *it;
}

template <typename T>
Node<T>& FatNodes<T>::Get(std::size_t version) 
{
  const auto& item = const_cast<const FatNodes<T>*>(this)->Get(version);
  return const_cast<Node<T>&>(item);
}

template <typename T>
void FatNodes<T>::Add(std::size_t version, T value)
{
  nodes_.emplace_back(version, value);
}

template <typename T>
void FatNodes<T>::Remove(std::size_t version)
{
  if (HasItem(version)) {
    Get(version).is_deleted = true;
  }
}

template <typename T>
bool FatNodes<T>::HasItem(std::size_t version) const
{
  auto it = std::find_if(nodes_.rbegin(), nodes_.rend(), 
    [&version](const Node<T>& node) {
      if (node.version <= version) {
        return !node.is_deleted;
      }
      return false;
    });
  return it != nodes_.rend();
}

}