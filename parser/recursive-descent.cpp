#include <bits/stdc++.h>

#include "lex.hpp"

using Lex::TokenSequence;

struct Recursive {
  TokenSequence tokens; // 当前在分析的符号流

  auto Current() const { // 当前分析的第一个符号
    return tokens.front();
  }

  void ProcE() { // E的递归分析程序
    std::cerr << "In ProcE\n";
    ProcT();
    if (Current().second == "+" || Current().second == "-") {
      tokens.pop();
      ProcT();
    }
  }

  void ProcT() { // T的递归分析程序
    std::cerr << "In ProcT\n";
    ProcF();
    if (Current().second == "*" || Current().second == "/") {
      tokens.pop();
      ProcF();
    }
  }

  void ProcF() { // F的递归分析程序
    std::cerr << "In ProcF\n";
    if (Current().first == TOKEN::FLOAT || Current().first == TOKEN::INT) {
      tokens.pop();
    } else if (Current().second == "(") {
      tokens.pop();
      ProcE();
      if (Current().second == ")")
        tokens.pop();
      else
        Error();
    } else
      Error();
  }

  void Error() {
    std::cerr << "Error\n";
    exit(0);
  }

  void Run(TokenSequence tok) {
    tokens = tok;
    tokens.push({TOKEN::BLANK, ""});
    ProcE();
    if (tokens.size() != 1)
      Error();
    else
      std::cerr << "Success\n";
  }
};

int main(int argc, const char **argv) {
  if (argc != 2)
    std::cerr << "Usage: ./recursive-descent source_file\n";
  else {
    std::ifstream src(argv[1]);
    Recursive r;
    r.Run(Lex::clangLex.GenSequence(src));
  }
  return 0;
}