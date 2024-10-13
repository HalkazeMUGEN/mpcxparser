/**
 * @file mugenpcx.cpp
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
#include <bitset>
#include <fstream>
#include <ios>
#include <memory>
#include <ostream>

namespace mugen {
namespace pcx {
namespace internal {

static constexpr std::uint8_t LEN_MARKER = 0xC0;
static constexpr std::uint8_t PAL_MARKER = 0x0C;

MPCXPARSER_PACK(struct IcoHeader {
  std::uint16_t reserved1;
  std::uint16_t type;
  std::uint16_t count;
  std::uint8_t width;
  std::uint8_t height;
  std::uint8_t colorCount;
  std::uint8_t reserved2;
  std::uint16_t planes;
  std::uint16_t colorDepth;
  std::uint32_t sizeOfImage;
  std::uint32_t offset;
});

MPCXPARSER_PACK(struct BmpFileHeader {
  char signature[2];
  std::uint32_t size;
  std::uint16_t reserved1;
  std::uint16_t reserved2;
  std::uint32_t offset;
});

MPCXPARSER_PACK(struct BmpInfoHeader {
  std::uint32_t sizeOfHeader;
  std::uint32_t width;
  std::uint32_t height;
  std::uint16_t planes;
  std::uint16_t colorDepth;
  std::uint32_t compressionType;
  std::uint32_t sizeOfImage;
  std::uint32_t hPixelsPerMeter;
  std::uint32_t vPixelsPerMeter;
  std::uint32_t palleteColors;
  std::uint32_t palleteImportantColors;
});

MPCXPARSER_PACK(struct BmpV4InfoHeader {
  std::uint32_t sizeOfHeader;
  std::uint32_t width;
  std::uint32_t height;
  std::uint16_t planes;
  std::uint16_t colorDepth;
  std::uint32_t compressionType;
  std::uint32_t sizeOfImage;
  std::uint32_t hPixelsPerMeter;
  std::uint32_t vPixelsPerMeter;
  std::uint32_t palleteColors;
  std::uint32_t palleteImportantColors;
  std::uint32_t bitmaskR;
  std::uint32_t bitmaskG;
  std::uint32_t bitmaskB;
  std::uint32_t bitmaskA;
  char colorSpace[4];
  std::uint8_t colorSpaceCoordinates[36];
  std::uint32_t gammaR;
  std::uint32_t gammaG;
  std::uint32_t gammaB;
});

MPCXPARSER_PACK(struct PcxRGB {
  std::uint8_t red;
  std::uint8_t green;
  std::uint8_t blue;
});

MPCXPARSER_PACK(struct PcxPallete {
  std::uint8_t marker;
  PcxRGB pal[256];
});

static inline std::uint8_t getc(std::vector<std::uint8_t>::const_iterator& begin, std::vector<std::uint8_t>::const_iterator& end) noexcept {
  if (begin == end) {
    return 0xFF;
  } else {
    auto result = *begin;
    ++begin;
    return result;
  }
}

static inline void pcx_encode(std::vector<std::uint8_t>::const_iterator& begin,
                              std::vector<std::uint8_t>::const_iterator& end,
                              std::size_t maxLength,
                              std::size_t& length,
                              std::uint8_t& value) noexcept {
  if (begin == end) {
    length = 0;
    return;
  }

  if (maxLength > 0x3F) {
    maxLength = 0x3F;
  }

  length = 1;
  value = getc(begin, end);

  while (begin != end && length < maxLength) {
    auto next = getc(begin, end);
    if (next != value) {
      --begin;
      break;
    }
    ++length;
  }
}

static inline void write_as_pcx8(std::ostream& os,
                                 const PcxHeader& header,
                                 const std::array<Pcx::Pixel, 256>& pallete,
                                 const std::vector<std::uint8_t>& indexes) {
  os.write(std::bit_cast<char*>(&header), sizeof(header));

  std::size_t maxLength = 0;

  auto begin = indexes.cbegin();
  auto end = indexes.cend();
  while (begin != end) {
    std::size_t len;
    std::uint8_t value;

    if (maxLength == 0) {
      maxLength = std::min<std::size_t>(header.hRes, header.bytesPerLine);
    }
    internal::pcx_encode(begin, end, maxLength, len, value);
    maxLength -= len;

    if (len <= 2 && value < LEN_MARKER) {
      if (len == 1) {
        os.write(std::bit_cast<char*>(&value), 1);
      } else {
        char buf[] = {static_cast<char>(value), static_cast<char>(value)};
        os.write(buf, 2);
      }
    } else {
      char buf[] = {static_cast<char>(len | LEN_MARKER), static_cast<char>(value)};
      os.write(buf, 2);
    }
  }

  internal::PcxPallete pcxPallete{.marker = PAL_MARKER};
  for (std::size_t i = 0; i < pallete.size(); ++i) {
    pcxPallete.pal[i].red = pallete[i].red;
    pcxPallete.pal[i].green = pallete[i].green;
    pcxPallete.pal[i].blue = pallete[i].blue;
  }
  os.write(std::bit_cast<char*>(&pcxPallete), sizeof(pcxPallete));
}

static inline void write_as_pcx32(std::ostream& os, const PcxHeader& header, const std::vector<Pcx::Pixel>& data) {
  os.write(std::bit_cast<char*>(&header), sizeof(header));

  std::size_t maxLength = 0;

  auto pcxData = std::vector<std::uint8_t>(header.hRes * header.vRes * 3);
  for (std::size_t y = 0; y < header.vRes; ++y) {
    for (std::size_t x = 0; x < header.hRes; ++x) {
      pcxData[y * header.hRes * 3 + x + 0 * header.hRes] = data[y * header.hRes + x].red;
      pcxData[y * header.hRes * 3 + x + 1 * header.hRes] = data[y * header.hRes + x].green;
      pcxData[y * header.hRes * 3 + x + 2 * header.hRes] = data[y * header.hRes + x].blue;
    }
  }

  auto begin = pcxData.cbegin();
  auto end = pcxData.cend();
  while (begin != end) {
    std::size_t len;
    std::uint8_t value;

    if (maxLength == 0) {
      maxLength = std::min<std::size_t>(header.hRes, header.bytesPerLine) * 3;
    }
    internal::pcx_encode(begin, end, maxLength, len, value);
    maxLength -= len;

    if (len <= 2 && value < LEN_MARKER) {
      if (len == 1) {
        os.write(std::bit_cast<char*>(&value), 1);
      } else {
        char buf[] = {static_cast<char>(value), static_cast<char>(value)};
        os.write(buf, 2);
      }
    } else {
      char buf[] = {static_cast<char>(len | LEN_MARKER), static_cast<char>(value)};
      os.write(buf, 2);
    }
  }
}

static inline void write_as_ico8(std::ostream& os,
                                 std::size_t width,
                                 std::size_t height,
                                 const std::array<Pcx::Pixel, 256>& pallete,
                                 const std::vector<std::uint8_t>& indexes) {
  // ビットマップ情報のサイズ
  //  = BMPヘッダーサイズ + パレットサイズ（= 256 * 4） + XORマスクのサイズ + ANDマスクのサイズ

  // XORマスクのサイズ計算
  // 1. 一行を4Byteにアライメント調整
  std::size_t lineSizeOfXorMask = (width + 3) & (~3);
  // 2. XORマスクのサイズ = 一行に必要なバイト数 * height
  std::size_t sizeOfXorMask = lineSizeOfXorMask * height;

  // ANDマスクのサイズ計算
  // 1. 一行に必要なバイトを算出
  //    1 Pixel につき 1bit なので、単純に width / 8 の切り上げとなる
  //    ここで、width は高々256なので、オーバーフローは気にしなくてよい
  std::size_t sizeOfAndMask = (width + 7) / 8;
  // 2. マスクを4Byteにアライメント調整
  sizeOfAndMask = (sizeOfAndMask + 3) & (~3);
  std::size_t lineSizeOfAndMask = sizeOfAndMask * 8;
  // 3. ANDマスクのサイズ = 一行に必要なバイト数 * height
  sizeOfAndMask *= height;

  internal::IcoHeader icoHeader{
      .type = 1,
      .count = 1,
      .width = static_cast<std::uint8_t>(width & 0xFF),
      .height = static_cast<std::uint8_t>(height & 0xFF),
      .planes = 1,
      .colorDepth = 8,
      .sizeOfImage = static_cast<std::uint32_t>(sizeof(internal::BmpInfoHeader) + (256 * 4) + sizeOfXorMask + sizeOfAndMask),
      .offset = sizeof(internal::IcoHeader),
  };

  internal::BmpInfoHeader bmpHeader{
      .sizeOfHeader = sizeof(internal::BmpInfoHeader),
      .width = static_cast<std::uint32_t>(width),
      .height = static_cast<std::uint32_t>(height * 2),
      .planes = 1,
      .colorDepth = 8,
      .sizeOfImage = static_cast<std::uint32_t>(sizeOfXorMask),
      .palleteColors = 256,
  };

  os.write(std::bit_cast<char*>(&icoHeader), sizeof(icoHeader));
  os.write(std::bit_cast<char*>(&bmpHeader), sizeof(bmpHeader));

  for (std::size_t i = 0; i < 256; ++i) {
    char BGR_[] = {static_cast<char>(pallete[i].blue), static_cast<char>(pallete[i].green), static_cast<char>(pallete[i].red), 0};
    os.write(BGR_, 4);
  }

  auto andMask = std::unique_ptr<char[]>(new char[sizeOfAndMask]());
  auto line = std::unique_ptr<char[]>(new char[lineSizeOfXorMask]());
  for (std::size_t y = height - 1; y < height; --y) {
    for (std::size_t x = 0; x < width; ++x) {
      // 0番目の色を使用しているピクセルを透過
      if ((line[x] = indexes[y * width + x]) == 0) {
        // indexesの並びとandMaskの並びではyが逆順であることに注意して
        // indexをandMaskにおけるindexを計算
        std::size_t index = ((height - 1) - y) * lineSizeOfAndMask + x;

        // bit単位で値を書き換えるため、char -> bitsetに変更
        // indexはビット単位なので、index / 8でバイト単位のindexを算出
        std::bitset<8> maskBits{static_cast<unsigned long>(andMask[index / 8])};

        // 1バイト単位でbitsetにしているため、0-7のindexでビット単位の値を書き換え
        // bitset -> char はリトルエンディアンであることに注意
        maskBits[7 - index % 8] = 1;
        andMask[index / 8] = static_cast<char>(maskBits.to_ulong());
      }
    }
    os.write(line.get(), lineSizeOfXorMask);
  }

  os.write(std::bit_cast<char*>(andMask.get()), sizeOfAndMask);
}

static inline void write_as_ico32(std::ostream& os, std::size_t width, std::size_t height, std::vector<Pcx::Pixel> data) {
  // ビットマップ情報のサイズ
  //  = BMPヘッダーサイズ + XORマスクのサイズ（= height * width * 4） + ANDマスクのサイズ

  // ANDマスクのサイズ計算
  // 1. 一行に必要なバイトを算出
  //    1 Pixel につき 1bit なので、単純に width / 8 の切り上げとなる
  //    ここで、width は高々256なので、オーバーフローは気にしなくてよい
  std::size_t sizeOfAndMask = (width + 7) / 8;
  // 2. マスクを4Byteにアライメント調整
  sizeOfAndMask = (sizeOfAndMask + 3) & (~3);
  // 3. ANDマスクのサイズ = 一行に必要なバイト数 * height
  sizeOfAndMask *= height;

  internal::IcoHeader icoHeader{
      .type = 1,
      .count = 1,
      .width = static_cast<std::uint8_t>(width & 0xFF),
      .height = static_cast<std::uint8_t>(height & 0xFF),
      .planes = 1,
      .colorDepth = 32,
      .sizeOfImage = static_cast<std::uint32_t>(sizeof(internal::BmpInfoHeader) + (width * height * 4) + sizeOfAndMask),
      .offset = sizeof(internal::IcoHeader),
  };

  internal::BmpInfoHeader bmpHeader{
      .sizeOfHeader = sizeof(internal::BmpInfoHeader),
      .width = static_cast<std::uint32_t>(width),
      .height = static_cast<std::uint32_t>(height * 2),
      .planes = 1,
      .colorDepth = 32,
      .sizeOfImage = static_cast<std::uint32_t>(width * height * 4),
  };

  os.write(std::bit_cast<char*>(&icoHeader), sizeof(icoHeader));
  os.write(std::bit_cast<char*>(&bmpHeader), sizeof(bmpHeader));

  for (std::size_t y = height - 1; y < height; --y) {
    for (std::size_t x = 0; x < width; ++x) {
      const auto& pixel = data[y * width + x];

      char BGRA[] = {static_cast<char>(pixel.blue), static_cast<char>(pixel.green), static_cast<char>(pixel.red), static_cast<char>(pixel.alpha)};
      os.write(BGRA, 4);
    }
  }

  auto andMask = std::unique_ptr<char[]>(new char[sizeOfAndMask]());
  os.write(andMask.get(), sizeOfAndMask);
}

};  // namespace internal
};  // namespace pcx
};  // namespace mugen

MPCXPARSER_INLINE void mugen::pcx::Pcx::write_as_pcx(const std::filesystem::path& path) const {
  static constexpr std::uint8_t PCX_SIGNATURE = 0x0A;

  std::ofstream ofs{path, std::ios_base::binary};

  internal::PcxHeader header{
      .signature = PCX_SIGNATURE,
      .version = 5,
      .encoding = 1,
      .bitsPerPixel = 8,
      .endX = static_cast<std::uint16_t>(width_ - 1),
      .endY = static_cast<std::uint16_t>(height_ - 1),
      .hRes = static_cast<std::uint16_t>(width_),
      .vRes = static_cast<std::uint16_t>(height_),
      .bytesPerLine = static_cast<std::uint16_t>(bytesPerLine_),
      .palleteMode = 1,
  };

  if (pallete_ && indexes_) {
    header.colorPlanes = 1;
    internal::write_as_pcx8(ofs, header, *pallete_, *indexes_);
  } else {
    header.colorPlanes = 3;
    internal::write_as_pcx32(ofs, header, data_);
  }
}

MPCXPARSER_INLINE void mugen::pcx::Pcx::write_as_ico(const std::filesystem::path& path) const {
  if (width_ > 256 || height_ > 256) {
    throw IllegalFormatError{"The PCX is too large for icon."};
  }

  std::ofstream ofs{path, std::ios_base::binary};

  // パレット情報とインデックス情報を持っている場合、
  // 出力されるファイルサイズがより小さくなる形式で出力
  //
  // width * height * 4 >= width * height + 256 * 4 => ico 8bit index with 256 pallete
  // width * height * 4 < width * height + 256 * 4  => ico 32bit color
  if (pallete_ && indexes_ && width_ * height_ >= (256 * 4) / 3) {
    internal::write_as_ico8(ofs, width_, height_, *pallete_, *indexes_);
  } else {
    internal::write_as_ico32(ofs, width_, height_, data_);
  }
}

MPCXPARSER_INLINE void mugen::pcx::Pcx::write_as_bmp(const std::filesystem::path& path) const {
  static constexpr char BMP_SIGNATURE[2] = {'B', 'M'};

  std::ofstream ofs{path, std::ios_base::binary};

  internal::BmpFileHeader fileHeader{
      .signature = {BMP_SIGNATURE[0], BMP_SIGNATURE[1]},
      .size = 0,
      .offset = sizeof(internal::BmpFileHeader) + sizeof(internal::BmpInfoHeader),
  };

  internal::BmpInfoHeader infoHeader{
      .sizeOfHeader = sizeof(internal::BmpInfoHeader),
      .width = static_cast<std::uint32_t>(width_),
      .height = static_cast<std::uint32_t>(height_),
      .planes = 1,
      .colorDepth = 32,
      .sizeOfImage = static_cast<std::uint32_t>(width_ * height_ * 4),
  };

  ofs.write(std::bit_cast<char*>(&fileHeader), sizeof(fileHeader));
  ofs.write(std::bit_cast<char*>(&infoHeader), sizeof(infoHeader));

  for (std::size_t y = height_ - 1; y < height_; --y) {
    for (std::size_t x = 0; x < width_; ++x) {
      const auto& pixel = data_[y * width_ + x];

      char BGRA[] = {static_cast<char>(pixel.blue), static_cast<char>(pixel.green), static_cast<char>(pixel.red), static_cast<char>(pixel.alpha)};
      ofs.write(BGRA, 4);
    }
  }
}

MPCXPARSER_INLINE void mugen::pcx::Pcx::write_as_abmp(const std::filesystem::path& path) const {
  static constexpr char BMP_SIGNATURE[2] = {'B', 'M'};

  std::ofstream ofs{path, std::ios_base::binary};

  internal::BmpFileHeader fileHeader{
      .signature = {BMP_SIGNATURE[0], BMP_SIGNATURE[1]},
      .size = static_cast<std::uint32_t>(width_ * height_ * 4 + sizeof(internal::BmpFileHeader) + sizeof(internal::BmpV4InfoHeader)),
      .offset = sizeof(internal::BmpFileHeader) + sizeof(internal::BmpV4InfoHeader),
  };

  internal::BmpV4InfoHeader infoHeader{
      .sizeOfHeader = sizeof(internal::BmpV4InfoHeader),
      .width = static_cast<std::uint32_t>(width_),
      .height = static_cast<std::uint32_t>(height_),
      .planes = 1,
      .colorDepth = 32,
      .compressionType = 3,
      .bitmaskR = 0x00FF0000,
      .bitmaskG = 0x0000FF00,
      .bitmaskB = 0x000000FF,
      .bitmaskA = 0xFF000000,
  };

  ofs.write(std::bit_cast<char*>(&fileHeader), sizeof(fileHeader));
  ofs.write(std::bit_cast<char*>(&infoHeader), sizeof(infoHeader));

  for (std::size_t y = height_ - 1; y < height_; --y) {
    for (std::size_t x = 0; x < width_; ++x) {
      const auto& pixel = data_[y * width_ + x];

      char BGRA[] = {static_cast<char>(pixel.blue), static_cast<char>(pixel.green), static_cast<char>(pixel.red), static_cast<char>(pixel.alpha)};
      ofs.write(BGRA, 4);
    }
  }
}
