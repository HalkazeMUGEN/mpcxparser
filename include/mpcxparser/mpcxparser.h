/**
 * @file mpcxparser.h
 * @author Halkaze
 * @date 2024-10-14
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

#ifndef MPCXPARSER_H__
#define MPCXPARSER_H__

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <istream>
#include <optional>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef MPCXPARSER_PACK
#if defined(_MSC_VER)
#define MPCXPARSER_PACK(declaration) __pragma(pack(push, 1)) declaration __pragma(pack(pop))
#elif defined(__GNUC__)
#define MPCXPARSER_PACK(declaration) declaration __attribute__((__packed__))
#endif
#endif

namespace mugen {
namespace pcx {

enum class MugenVersion {
  Win,
  // Latest, 必要になったら実装できるように
};

class Pcx;

template <MugenVersion Version>
class PcxParser {
 public:
  PcxParser(const PcxParser&) = delete;
  PcxParser& operator=(const PcxParser&) = delete;

  PcxParser(PcxParser&&) = default;
  PcxParser& operator=(PcxParser&&) = default;

  explicit PcxParser() noexcept;

  Pcx parse(std::istream& is) const;

  Pcx parse(const std::filesystem::path& pcx) const;

  template <std::size_t Extent>
  Pcx parse(std::span<std::uint8_t, Extent> mem) const;

  Pcx parse(const std::uint8_t* mem, std::size_t length) const;
};

using PcxParserWin = PcxParser<MugenVersion::Win>;

};  // namespace pcx
};  // namespace mugen

#include "mpcxparser/exception.hpp"
#include "mpcxparser/mugenpcx.hpp"

#ifdef MPCXPARSER_HEADER_ONLY
#include "mpcxparser/impl/mpcxparser.cpp"
#include "mpcxparser/impl/mugenpcx.cpp"
#endif

#endif  // MPCXPARSER_H__
