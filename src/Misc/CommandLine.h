#pragma once

#include <map>
#include <string>
#include <vector>

class CommandLineParser {
 public:
  CommandLineParser(std::wstring cmdline);

  bool Contains(const std::wstring& arg);
  std::wstring Value(const std::wstring& arg);
  std::map<std::wstring, std::wstring> Args();

 private:
  void TrimBinary(std::wstring& cmdline);
  std::wstring& TrimQuote(std::wstring& val);
  std::vector<std::wstring> ParseArguments(std::wstring& cmdline);

  std::map<std::wstring, std::wstring> m_args;
};