#pragma once

#include "js32.h"

#include <memory>

class StringWrap {
 public:
  StringWrap() = default;
  StringWrap(JSContext* ctx, JSString* str);
  StringWrap(JSContext* ctx, JS::HandleValue value);

  operator bool() const noexcept {
    return !!m_str.get();
  }

  operator const char*() const noexcept {
    return m_str.get();
  }

  const char* c_str() const noexcept {
    return m_str.get();
  }

  size_t length() const noexcept {
    return m_length;
  }

  size_t size() const noexcept {
    return m_length;
  }

 private:
  // stored as unique pointer instead of std::string to avoid double copy
  std::unique_ptr<char[]> m_str;
  size_t m_length;
};
