#pragma once

namespace pdc {

template <typename Derived>
class Persisent {
public:
  virtual Derived Undo() const = 0;
  virtual Derived Redo() const = 0;
};

} // namespace pdc