/**
 * @file exception.hpp
 * @author Halkaze
 * @date 2024-10-13
 *
 * @copyright Copyright (c) 2024
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MPCXPARSER_EXCEPTION_HPP__
#define MPCXPARSER_EXCEPTION_HPP__

#include "mpcxparser/mpcxparser.h"

#include <stdexcept>
#include <string>

namespace mugen {
namespace pcx {

class FileIOError : public std::runtime_error {
 public:
  inline explicit FileIOError(const std::string& message) noexcept : std::runtime_error{message} {}
  inline explicit FileIOError(const char* message) noexcept : std::runtime_error{message} {}
};

class IllegalFormatError : public std::logic_error {
 public:
  inline explicit IllegalFormatError(const std::string& message) noexcept : std::logic_error{message} {}
  inline explicit IllegalFormatError(const char* message) noexcept : std::logic_error{message} {}
};

class IncompatibleFormatError : public std::logic_error {
 public:
  inline explicit IncompatibleFormatError(const std::string& message) noexcept : std::logic_error{message} {}
  inline explicit IncompatibleFormatError(const char* message) noexcept : std::logic_error{message} {}
};

};  // namespace pcx
};  // namespace mugen

#endif  // MPCXPARSER_EXCEPTION_HPP__
