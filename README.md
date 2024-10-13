# Mugen Pcx Parser

**Mugen Pcx Parser** is an open-source library for parsing PCX of MUGEN for C++.

## Platforms

- Windows (MSVC / clang / MinGW clang)
- Linux (clang)

## Install

### with CMake (recommended)

Add the following to your `CMakeLists.txt`.

```cmake
include(FetchContent)
FetchContent_Declare(
  mpcxparser
  GIT_REPOSITORY https://github.com/HalkazeMUGEN/mpcxparser.git
  GIT_TAG v1.0.1
)
FetchContent_MakeAvailable(mpcxparser)

target_link_libraries(<target> mpcxparser)
```

### without any build tools

Copy the include [folder](https://github.com/HalkazeMUGEN/mpcxparser/tree/main/include) to your include folder.

Then, define the macro as follows when you include it.

```c
#define MPCXPARSER_HEADER_ONLY
#include <mpcxparser/mpcxparser.h>
```

See also [examples](https://github.com/HalkazeMUGEN/mpcxparser/tree/main/example).

## Usage samples

### Parse PCX file

```cpp
#include <mpcxparser/mpcxparser.h>

void parse_example(const std::filesystem::path& path) {
  try {
    auto parser = mugen::pcx::PcxParserWin{};
    auto pcx = parser.parse(path);

    std::cout << pcx.width() << std::endl;
    std::cout << (*(pcx.indexes()))[0] << std::endl;
    std::cout << pcx.data()[0].red << std::endl;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
```

### Write as other format

```cpp
#include <mpcxparser/mpcxparser.h>

void write_example(const std::filesystem::path& src, const std::filesystem::path& dest) {
  try {
    auto parser = mugen::pcx::PcxParserWin{};
    auto pcx = parser.parse(src);

    pcx.write_as_pcx(dest);
    pcx.write_as_ico(dest);
    pcx.write_as_bmp(dest);
    pcx.write_as_abmp(dest);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
```

See also [examples](https://github.com/HalkazeMUGEN/mpcxparser/tree/main/example).

## License

This project is licensed under the terms of the [GNU General Public License v3.0 or later](https://www.gnu.org/licenses/gpl-3.0.html).

You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
