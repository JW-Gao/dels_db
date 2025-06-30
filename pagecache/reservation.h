#pragma once
#include "logger.h"
#include "disk_pointer.h"
#include "iobuf.h"
#include <memory>

class Reservation {
  Log log;
  std::shared_ptr<IoBuf> buf;
  DiskPtr disk_ptr;
};