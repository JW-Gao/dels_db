#pragma once
#include <zlib.h>
#include <cstdint>
#include <cstddef>

uint32_t crc32_buf(const unsigned char *buf, size_t len) {
  auto crc_res = ::crc32(0, buf, len);
  return (crc_res ^ 0xffffffff);
}