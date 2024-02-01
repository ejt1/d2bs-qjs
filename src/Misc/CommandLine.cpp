#include "CommandLine.h"

CommandLineParser::CommandLineParser(std::wstring cmdline) {
  // example input C:\Program Files (x86)\Diablo II\Game.exe -profile "test" -handle "4325261" -cachefix -multi
  // -title "some title" -w -ns -sleepy -ftj
  TrimBinary(cmdline);
  auto args = ParseArguments(cmdline);

  auto it = args.begin();
  while (it != args.end()) {
    std::wstring arg = (*it);
    std::wstring val;
    if (++it != args.end() && (*it)[0] != L'-') {
      val = (*it);
      ++it;
    }
    m_args[arg] = TrimQuote(val);
  }
}

bool CommandLineParser::Contains(const std::wstring& arg) {
  return m_args.contains(arg);
}

std::wstring CommandLineParser::Value(const std::wstring& arg) {
  return Contains(arg) ? m_args[arg] : L"";
}

std::map<std::wstring, std::wstring> CommandLineParser::Args() {
  return m_args;
}

void CommandLineParser::TrimBinary(std::wstring& cmdline) {
  size_t pos = 0;

  // remove the binary path from command line
  if ((pos = cmdline.find(L".exe")) != std::wstring::npos) {
    cmdline.erase(0, pos + 4);

    // remove trailing space, if present
    if (!cmdline.empty() && cmdline[0] == ' ') {
      cmdline.erase(0, 1);
    }
  }
}

std::wstring& CommandLineParser::TrimQuote(std::wstring& val) {
  if (val.starts_with(L'\"')) {
    val.erase(0, 1);
  }
  if (val.ends_with(L'\"')) {
    val.erase(val.size() - 1, val.size());
  }
  return val;
}

std::vector<std::wstring> CommandLineParser::ParseArguments(std::wstring& cmdline) {
  size_t pos = 0;
  std::vector<std::wstring> args;

  // tokenize the remainder of command line into arguments
  while ((pos = cmdline.find(' ')) != std::wstring::npos || !cmdline.empty()) {
    args.push_back(cmdline.substr(0, pos));

    // contains trailing space
    if (pos != std::wstring::npos) {
      cmdline.erase(0, pos + 1);
    } else {
      // last argument, clear the string to stop loop
      // ugly but best I can think of right now :)
      cmdline.clear();
    }
  }

  return args;
}
