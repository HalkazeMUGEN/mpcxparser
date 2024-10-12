#include <mpcxparser/mpcxparser.h>

#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <path-to-pcx>" << std::endl;
    return 0;
  }

  auto parser = mugen::pcx::PcxParserWin{};
  // auto parser = mugen::pcx::PcxParser<mugen::pcx::MugenVersion::Win>{};

  std::filesystem::path path{argv[1]};

  // Get PCX's information
  try {
    auto pcx = parser.parse(path);

    // Example.1: get size
    std::cout << "Width : " << std::min<std::size_t>(pcx.width(), pcx.bytes_per_line()) << std::endl;
    std::cout << "Height: " << pcx.height() << std::endl;
    std::cout << std::endl;

    // Example.2: get pallete
    if (pcx.pallete().has_value()) {
      std::cout << "* Pallete" << std::endl;
      auto&& pallete = *(pcx.pallete());
      for (std::size_t i = 0; i < 256; ++i) {
        int r = pallete[i].red;
        int g = pallete[i].green;
        int b = pallete[i].blue;
        int a = pallete[i].alpha;
        std::cout << "pallete[" << i << "]: { R=" << r << ", G=" << g << ", B=" << b << ", A=" << a << " }" << std::endl;
      }
    }
    std::cout << std::endl;

    // Example.3: get indexes
    if (pcx.indexes().has_value()) {
      std::cout << "* Indexes" << std::endl;
      auto&& indexes = *(pcx.indexes());
      for (std::size_t y = 0; y < pcx.height(); ++y) {
        for (std::size_t x = 0; x < pcx.width(); ++x) {
          std::size_t index = indexes[y * pcx.width() + x];
          std::cout << "(" << x << ", " << y << ") = " << index << std::endl;
        }
      }
    }
    std::cout << std::endl;

    // Example.4: get data
    std::cout << "* Data" << std::endl;
    for (std::size_t y = 0; y < pcx.height(); ++y) {
      for (std::size_t x = 0; x < pcx.width(); ++x) {
        auto&& pixel = pcx.data()[y * pcx.width() + x];
        int r = pixel.red;
        int g = pixel.green;
        int b = pixel.blue;
        int a = pixel.alpha;
        std::cout << "(" << x << ", " << y << ") = " << "{ R=" << r << ", G=" << g << ", B=" << b << ", A=" << a << " }" << std::endl;
      }
    }

  } catch (const mugen::pcx::FileIOError& e) {
    // Throw FileIOError if path is not exist
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (const mugen::pcx::IllegalFormatError& e) {
    // Throw IllegalFormatError if pcx is illegal format
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (const mugen::pcx::IncompatibleFormatError& e) {
    // Throw IncompatibleFormatError if pcx is not illegal but Mugen can't recognize
    std::cerr << e.what() << std::endl;
    return 1;
  }
  // All errors are sub class of std::exception

  // Write
  try {
    auto pcx = parser.parse(path);

    pcx.write_as_pcx(path.append(".pcx"));
    pcx.write_as_ico(path.append(".ico"));
    pcx.write_as_bmp(path.append(".bmp"));
    pcx.write_as_abmp(path.append("-alpha.bmp"));
  } catch (const std::logic_error& e) {
    // IllegalFormatError & IncompatibleFormatError is sub class of std::logic_error
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (const std::runtime_error& e) {
    // FileIOError is sub class of std::runtime_error
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
