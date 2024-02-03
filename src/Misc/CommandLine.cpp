#include "CommandLine.h"

CommandLineParser::CommandLineParser(std::string cmdline) {
  // example input C:\Program Files (x86)\Diablo II\Game.exe -profile "test" -handle "4325261" -cachefix -multi
  // -title "some title" -w -ns -sleepy -ftj
  TrimBinary(cmdline);
  auto args = ParseArguments(cmdline);

  auto it = args.begin();
  while (it != args.end()) {
    std::string arg = (*it);
    std::string val;
    if (++it != args.end() && (*it)[0] != L'-') {
      val = (*it);
      ++it;
    }
    m_args[arg] = TrimQuote(val);
  }
}

bool CommandLineParser::Contains(const std::string& arg) {
  return m_args.contains(arg);
}

std::string CommandLineParser::Value(const std::string& arg) {
  return Contains(arg) ? m_args[arg] : "";
}

std::map<std::string, std::string> CommandLineParser::Args() {
  return m_args;
}

void CommandLineParser::TrimBinary(std::string& cmdline) {
  size_t pos = 0;

  // remove the binary path from command line
  if ((pos = cmdline.find(".exe")) != std::string::npos) {
    cmdline.erase(0, pos + 4);

    // remove trailing space, if present
    if (!cmdline.empty() && cmdline[0] == ' ') {
      cmdline.erase(0, 1);
    }
  }
}

std::string& CommandLineParser::TrimQuote(std::string& val) {
  if (val.starts_with(L'\"')) {
    val.erase(0, 1);
  }
  if (val.ends_with(L'\"')) {
    val.erase(val.size() - 1, val.size());
  }
  return val;
}

std::vector<std::string> CommandLineParser::ParseArguments(std::string& cmdline) {
  size_t pos = 0;
  std::vector<std::string> args;

  // tokenize the remainder of command line into arguments
  while ((pos = cmdline.find(' ')) != std::string::npos || !cmdline.empty()) {
    args.push_back(cmdline.substr(0, pos));

    // contains trailing space
    if (pos != std::string::npos) {
      cmdline.erase(0, pos + 1);
    } else {
      // last argument, clear the string to stop loop
      // ugly but best I can think of right now :)
      cmdline.clear();
    }
  }

  return args;
}
