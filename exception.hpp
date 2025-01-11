#pragma once

#include <exception>


namespace pdc {

/*! The incorrect version of the collection was excluded.
 *
 * Exception that is dropped by partially unmodified collections when 
 * attempting to modify an object not the latest version. 
 */
class IncorrectVersionException : public std::exception {
public:
  IncorrectVersionException() = default;
  ~IncorrectVersionException() override = default;
  const char* what() const noexcept override {
    return "The modifying operation was not called in the last version.";
  }
};

}