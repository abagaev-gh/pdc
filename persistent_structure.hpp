#pragma once

namespace pdc {

/*! \brief Interface for persistent data structure.
 * 
 * \tparam Derived The type of the interface’s inherit structure. 
 *                 Used as a return value type.
*/
template <typename Derived>
class Persisent {
public:
  /*! \brief Returns the previous version of the structure.
   *
   * \return Previous version of the structure.
   */
  virtual Derived Undo() const = 0;

  /*! \brief Returns the next version of the structure.
   *
   * \return Next version of the structure.
   */
  virtual Derived Redo() const = 0;
};

} // namespace pdc