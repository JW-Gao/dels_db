#pragma once


#include <cstring>

#include "def_types.h"
#include "constant.h"
#include "../3rd/log/tlog.h"
#include "../util/pcrc.h"



struct SegmentHeader {
  Lsn lsn;
  Lsn max_stable_lsn;
  bool ok;
  

  void to_char(unsigned char *buf /* buf_len = SEG_HEADER_LEN*/) {
    tlog_info << "Segment to char";
    uint64_t xor_lsn = (lsn ^  0x7FFFFFFFFFFFFFFF);
    const unsigned char *lsn_arr = reinterpret_cast<const unsigned char*>(&xor_lsn); // 8 bytes

    uint64_t xor_max_stable_lsn = (max_stable_lsn ^ 0x7FFFFFFFFFFFFFFF);
    const unsigned char *highest_stable_lsn_arr = reinterpret_cast<const unsigned char *>(xor_max_stable_lsn); // 8bytes

    // copy
    std::memcpy(buf + 4, lsn_arr, 8);
    std::memcpy(buf + 12, highest_stable_lsn_arr, 8);

    //calculate crc32
    // buf[0..4]是crc32
    auto crc_res = crc32_buf(buf + 4, SEG_HEADER_LEN - 4);
    std::memcpy(buf, &crc_res, 4);
  }
};



class Log {

};