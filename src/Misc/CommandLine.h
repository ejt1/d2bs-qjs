#pragma once

#include <map>
#include <string>
#include <vector>

class CommandLineParser {
 public:
  CommandLineParser(std::string cmdline);

  bool Contains(const std::string& arg);
  std::string Value(const std::string& arg);
  std::map<std::string, std::string> Args();

 private:
  void TrimBinary(std::string& cmdline);
  std::string& TrimQuote(std::string& val);
  std::vector<std::string> ParseArguments(std::string& cmdline);

  std::map<std::string, std::string> m_args;
};