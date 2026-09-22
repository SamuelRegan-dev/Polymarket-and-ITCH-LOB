# Third-party code

## itchcpp

NASDAQ TotalView-ITCH 5.0 binary parsing is done entirely by **itchcpp**.

- Source: https://github.com/bbalouki/itchcpp
- Author: bbalouki
- Licence: MIT
- Used via CMake `FetchContent`; no source is vendored into this repo.

`My Code/adapters/itch_adapter.*` contains no parsing logic. It only translates
`itch::Message` into our own `MarketEvent`, so that nothing downstream depends
on itchcpp's types. Swapping the parser would touch that one file.

Requires C++20, which is why `CMAKE_CXX_STANDARD` is 20 rather than 17.

## simdjson

JSON tokenising for the Polymarket adapter.

- Source: https://github.com/simdjson/simdjson
- Licence: Apache-2.0
- Present both as a vendored amalgamation (`My Code/simdjson.{h,cpp}`) and as a
  `FetchContent` dependency. Those duplicate each other and should be reconciled.
