#pragma once
#include <coroutine>
enum Error {
  CollectionNotFound,
  /// The system has been used in an unsupported way.
  Unsupported,
  /// An unexpected bug has happened. Please open an issue on github!
  ReportableBug,
  /// A read or write error has happened when interacting with the file
  /// system.
  Io,
  /// Corruption has been detected in the storage file.
  Corruption,
};