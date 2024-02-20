#include "StringWrap.h"

#include <jsfriendapi.h>
#include <js/CharacterEncoding.h>

StringWrap::StringWrap() : m_str(nullptr), m_length(0) {
}

StringWrap::StringWrap(JSContext* ctx, JSString* str) : m_length(0) {
  if (!str) {
    JS_ReportErrorUTF8(ctx, "cannot flatten null string");
    return;
  }
  JSLinearString* linear = JS_EnsureLinearString(ctx, str);
  if (!linear) {
    JS_ReportErrorUTF8(ctx, "failed to flatten string");
    return;
  }
  m_length = JS::GetDeflatedUTF8StringLength(linear);
  m_str.reset(new char[m_length + 1]);
  JS::DeflateStringToUTF8Buffer(linear, mozilla::Span(m_str.get(), m_length));
  m_str[m_length] = '\0';
}

StringWrap::StringWrap(JSContext* ctx, JS::HandleValue value) : StringWrap(ctx, value.toString()) {
}
