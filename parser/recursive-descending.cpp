#include <bits/stdc++.h>

#include "lex.hpp"

using Lex::TokenSequence;

struct Recursive {
  TokenSequence tokens;

  auto Current() const { return tokens.front(); }

  void ProcE() {
    std::cerr << "In ProcE\n";
    ProcT();
    if (Current().second == "+" || Current().second == "-") {
      tokens.pop();
      ProcT();
    }
  }

  void ProcT() {
    std::cerr << "In ProcT\n";
    ProcF();
    if (Current().second == "*" || Current().second == "/") {
      tokens.pop();
      ProcF();
    }
  }

  void ProcF() {
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

  void Error() { std::cerr << "Error\n"; }

  void Run(TokenSequence tok) {
    tokens = tok;
    ProcE();
  }
};

int main(int argc, const char **argv) {
  // 0l 1r 2type 3str
  std::ifstream src("in");
  Recursive r;
  r.Run(Lex::clangLex.GenSequence(src));
  return 0;
}