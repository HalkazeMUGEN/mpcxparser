/**
 * @file mpcxparser.cpp
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

#ifdef MPCXPARSER_HEADER_ONLY
#define MPCXPARSER_INLINE inline
#else
#define MPCXPARSER_INLINE
#endif

#include "mpcxparser/mpcxparser.h"

#include <bit>
#include <fstream>
#include <ios>
#include <limits>
#include <streambuf>
#include <utility>

namespace mugen {
namespace pcx {
namespace internal {

// C++23 の ispanstream の代替
class SpanStreamBuf : public std::streambuf {
 private:
  const std::span<std::uint8_t> buf_;

 public:
  SpanStreamBuf(const std::span<std::uint8_t>& span) : buf_{span} {
    auto begin = std::bit_cast<char*>(const_cast<std::uint8_t*>(span.data()));
    this->setg(begin, begin, begin + span.size());
  }

 protected:
  pos_type seekoff(off_type offset, std::ios_base::seekdir way, std::ios_base::openmode) override {
    // Borrowed in part from MSVC's spanstream implementation
    switch (way) {
      case std::ios_base::beg:
        if (static_cast<size_t>(offset) > buf_.size()) {
          return pos_type{off_type{-1}};
        }

        break;
      case std::ios_base::end: {
        const auto baseoff = static_cast<off_type>(buf_.size());
        if (offset > std::numeric_limits<off_type>::max() - baseoff) {
          return pos_type{off_type{-1}};
        }

        offset += baseoff;
        if (static_cast<size_t>(offset) > buf_.size()) {
          return pos_type{off_type{-1}};
        }
      } break;
      case std::ios_base::cur: {
        const off_type oldoff = static_cast<off_type>(this->gptr() - this->eback());
        const off_type oldleft = static_cast<off_type>(buf_.size() - oldoff);
        if (offset < -oldoff || offset > oldleft) {
          return pos_type{off_type{-1}};
        }
        offset += oldoff;
      } break;
      default:
        return pos_type{off_type{-1}};
    }

    if (offset != 0 && !this->gptr()) {
      return pos_type{off_type{-1}};
    }

    this->gbump(static_cast<int>(offset - (this->gptr() - this->eback())));

    return pos_type{offset};
  }

  pos_type seekpos(pos_type pos, std::ios_base::openmode mode) override { return seekoff(pos, std::ios_base::beg, mode); }
};

static inline std::uint8_t getc(std::istream& is) noexcept {
  std::uint8_t byte = 0;
  is.read(std::bit_cast<char*>(&byte), 1);
  if (is.fail()) {
    return 0xFF;
  }
  return byte;
}

// EOFに到達した場合はtrueを返す
static inline bool skip_n(std::istream& is, std::size_t n) noexcept {
  if (n == 0) {
    return is.eof();
  }

  is.seekg(n, std::ios_base::cur);
  char p;
  is.read(&p, 1);
  if (is.eof()) {
    return true;
  }

  is.seekg(-1, std::ios_base::cur);
  return false;
}

static inline std::array<Pcx::Pixel, 256> convert_ega_to_pixel(const std::uint8_t (&egaPallete)[16][3]) noexcept {
  std::array<Pcx::Pixel, 256> pallete{};

  pallete[0].red = egaPallete[0][0];
  pallete[0].green = egaPallete[0][1];
  pallete[0].blue = egaPallete[0][2];
  pallete[0].alpha = 0;

  for (std::size_t i = 1; i < 16; ++i) {
    pallete[i].red = egaPallete[i][0];
    pallete[i].green = egaPallete[i][1];
    pallete[i].blue = egaPallete[i][2];
  }

  return pallete;
}

// len または value がEOFであった場合はtrueを返す
static inline bool pcx_decode(std::istream& is, std::size_t& len, std::uint8_t& value) noexcept {
  static constexpr std::uint8_t LEN_MARKER = 0xC0;

  len = 1;
  value = getc(is);

  if (is.eof()) {
    return true;
  }

  if ((value & LEN_MARKER) == LEN_MARKER) {
    len = (value & ~LEN_MARKER);
    value = getc(is);
    if (is.eof()) {
      return true;
    }
  }

  return false;
}

static inline std::vector<std::uint8_t> parse_indexes(std::istream& is,
                                                      std::size_t size,
                                                      std::size_t width,
                                                      std::size_t height,
                                                      std::size_t bytesPerLine) noexcept {
  std::vector<std::uint8_t> indexes(size, 0xFF);

  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < bytesPerLine;) {
      std::size_t len;
      std::uint8_t value;
      if (pcx_decode(is, len, value)) {
        return indexes;
      }

      for (; len != 0; --len) {
        if (x < width) {
          indexes[y * width + x] = value;
        }
        ++x;
      }
    }
  }

  return indexes;
}

static inline std::array<Pcx::Pixel, 256> parse_pallete(std::istream& is, const std::uint8_t (&egaPallete)[16][3]) noexcept {
  static constexpr std::uint8_t PAL_MARKER = 0x0C;

  std::uint8_t markerByte = 0;
  while (true) {
    markerByte = getc(is);
    if (markerByte == PAL_MARKER) {
      break;
    } else if (markerByte != 0) {
      return convert_ega_to_pixel(egaPallete);
    }
  }

  std::array<Pcx::Pixel, 256> pallete{};
  pallete[0].alpha = 0;

  for (std::size_t i = 0; i < pallete.size(); ++i) {
    pallete[i].red = getc(is);
    pallete[i].green = getc(is);
    pallete[i].blue = getc(is);
  }

  return pallete;
}

static inline std::vector<Pcx::Pixel> parse_data(std::istream& is,
                                                 std::size_t size,
                                                 std::size_t width,
                                                 std::size_t height,
                                                 std::size_t bytesPerLine) noexcept {
  std::vector<Pcx::Pixel> data(size);

  std::size_t bytes = bytesPerLine * 3;
  for (std::size_t y = 0; y < height; ++y) {
    std::size_t planeIndex = 0;
    std::size_t planeX = 0;
    for (std::size_t x = 0; x < bytes;) {
      std::size_t len;
      std::uint8_t value;
      pcx_decode(is, len, value);

      for (; len != 0; --len) {
        if (planeX < width) {
          switch (planeIndex) {
            case 0:
              data[y * width + planeX].red = value;
              break;
            case 1:
              data[y * width + planeX].green = value;
              break;
            case 2:
              data[y * width + planeX].blue = value;
              break;
          }
        }
        ++x;
        if (x == bytesPerLine) {
          planeIndex = 1;
          planeX = 0;
        } else if (x == bytesPerLine * 2) {
          planeIndex = 2;
          planeX = 0;
        } else {
          ++planeX;
        }
      }
    }
  }

  return data;
}

static inline Pcx parse_pcx(std::istream& is) {
  PcxHeaderMinimum header{};

  is.read(std::bit_cast<char*>(&header), sizeof(header));
  if (is.fail()) {
    throw IllegalFormatError{"The given PCX structure is too small."};
  }

  auto width = static_cast<std::size_t>(header.endX - header.startX + 1);
  auto height = static_cast<std::size_t>(header.endY - header.startY + 1);
  auto size = width * height;

  if (header.bitsPerPixel != 8 || (header.colorPlanes != 1 && header.colorPlanes != 3) || size == 0) {
    throw IncompatibleFormatError{"The given PCX structure is not available in MUGEN."};
  }

  if (skip_n(is, sizeof(PcxHeader) - sizeof(PcxHeaderMinimum))) {
    return Pcx{width, height, header.bytesPerLine, convert_ega_to_pixel(header.pallete), std::vector<std::uint8_t>(size, 0xFF)};
  }

  if (header.colorPlanes == 1) {
    auto indexes = parse_indexes(is, size, width, height, header.bytesPerLine);
    auto pallete = parse_pallete(is, header.pallete);
    return Pcx{width, height, header.bytesPerLine, std::move(pallete), std::move(indexes)};
  } else {
    auto data = parse_data(is, size, width, height, header.bytesPerLine);
    return Pcx{width, height, header.bytesPerLine, std::move(data)};
  }
}

};  // namespace internal
};  // namespace pcx
};  // namespace mugen

template <>
MPCXPARSER_INLINE mugen::pcx::PcxParserWin::PcxParser() noexcept {}

template <>
MPCXPARSER_INLINE mugen::pcx::Pcx mugen::pcx::PcxParserWin::parse(std::istream& is) const {
  return mugen::pcx::internal::parse_pcx(is);
}

template <>
MPCXPARSER_INLINE mugen::pcx::Pcx mugen::pcx::PcxParserWin::parse(const std::filesystem::path& pcx) const {
  if (!std::filesystem::exists(pcx) || !std::filesystem::is_regular_file(pcx)) {
    throw FileIOError{"The given PCX does not exist."};
  }
  auto ifs = std::ifstream{pcx, std::ios_base::binary};
  return mugen::pcx::internal::parse_pcx(ifs);
}

template <>
template <std::size_t Extent>
MPCXPARSER_INLINE mugen::pcx::Pcx mugen::pcx::PcxParserWin::parse(std::span<std::uint8_t, Extent> mem) const {
  mugen::pcx::internal::SpanStreamBuf sbuf{mem};
  std::istream is{&sbuf};
  return mugen::pcx::internal::parse_pcx(is);
}

template <>
MPCXPARSER_INLINE mugen::pcx::Pcx mugen::pcx::PcxParserWin::parse(const std::uint8_t* mem, std::size_t length) const {
  auto span = std::span(const_cast<std::uint8_t*>(mem), length);
  mugen::pcx::internal::SpanStreamBuf sbuf{span};
  std::istream is{&sbuf};
  return mugen::pcx::internal::parse_pcx(is);
}

#ifndef MPCXPARSER_HEADER_ONLY
template class mugen::pcx::PcxParser<mugen::pcx::MugenVersion::Win>;
template mugen::pcx::Pcx mugen::pcx::PcxParserWin::parse<std::dynamic_extent>(std::span<std::uint8_t, std::dynamic_extent> mem) const;
#endif
