#include <bits/stdc++.h>

#include "grammar_util.hpp"
#include "lex.hpp"
#include "table.hpp"

namespace GrammarLL1 {

struct LL1Automata : GrammarAutomataBase {
  SMap<SSet> follow;                        // follow集合
  SMap<SMap<std::vector<const Expr *>>> M;  // 分析表

  LL1Automata(std::string init, Grammar g) : GrammarAutomataBase(init, g) {
    Epsilon();
    First();
    Follow();
    MTable();
    Print();
  }

  void Follow() {           // 构造Follow集合
    allTokens.insert(END);  // 符号集合中加入表示结尾的符号$
    std::map<Token, std::map<Token, bool>> include;  // 表示Follow集合的包含关系
    include[{0, init}][END] = 1;
    for (auto &[a, exprs] : g) {
      for (auto &expr : exprs) {
        std::set<std::string> last =
            {};  // 对于每个右部Y1Y2...Yn，从右到左枚举每一项，维护可能接在Yi后面的终止符
        bool epsSuffix = 1;  // 表示当前枚举到的后缀是否可以转移到空
        for (int i = expr.size() - 1; i >= 0; --i) {
          if (!expr[i]
                   .T) {  // 对于非终止符，将所有last中的符号加入它的follow集合
            for (auto &s : last) include[expr[i]][{1, s}] = 1;
            if (epsSuffix)
              include[expr[i]][{0, a}] =
                  1;  // 若后缀整体可以为空，那么它的Follow集合还包含了生成式左部的Follow集合
          }
          if (expr[i].T || !eps.count(expr[i].str))
            last =
                {};  // 若出现了转移不到空的非终止符或者终止符，那么当前的last中的符号就不能接到更靠左的符号后面了
          if (expr[i].T)  // 前面的符号后方可以接上终止符自身
            last.insert(expr[i].str);
          else  // 前面的符号可以接上当前符号First集合中所有非空项
            for (auto &s : first[expr[i].str])
              if (s != EPSILON.str) last.insert(s);
          epsSuffix &=
              !expr[i].T &&
              eps.count(
                  expr[i]
                      .str);  //如果当前符号不能转移到空了，那后缀就无法转移到空了
        }
      }
    }
    for (auto &k : allTokens)
      for (auto &i : allTokens)
        for (auto &j : allTokens)
          include[i][j] |= include[i][k] & include[k][j];  // Warshall
    for (auto &i : allTokens)
      if (!i.T) {
        for (auto &j : allTokens)
          if (j.T && include[i][j]) follow[i.str].insert(j.str);
      }
  }

  void MTable() {  // 构造分析表
    for (auto &[a, exprs] : g)
      for (auto &expr : exprs)
        for (auto &s : SequenceFirst(expr.begin(), expr.end()))
          if (s ==
              EPSILON
                  .str)  // 右部能转移到空，则对左部follow集合中的元素b，把该生成式加入M[左部,b]中
            for (auto &b : follow[a]) M[a][b].push_back(&expr);
          else  // 对First(右部)中的非空元素s，把该生成式加入M[左部,s]中
            M[a][s].push_back(&expr);
  }

  void Print() {
    TableContent tFirstFollow = {{{""}, {"FIRST"}, {"FOLLOW"}}};
    SSet VN, VT;
    for (auto &i : allTokens) (i.T ? VT : VN).insert(i.str);
    for (auto &n : VN) {
      std::string fs = "{", fl = "{";
      for (auto s : first[n]) fs += (fs == "{" ? "" : ",") + s;
      for (auto s : follow[n]) fl += (fl == "{" ? "" : " ,") + s;
      fs += " }";
      fl += " }";
      tFirstFollow.push_back({{n}, {fs}, {fl}});
    }
    std::cout << "FIRST and FOLLOW set:\n";
    PrintTable(tFirstFollow, {4, 20, 20});
    std::cout << "M table:\n";

    TableContent tM = {{{""}}};
    for (auto &t : VT) tM.back().push_back({t});
    for (auto &n : VN) {
      tM.push_back({});
      tM.back().push_back({n});
      for (auto &t : VT) {
        tM.back().push_back({});
        for (auto &e : M[n][t]) tM.back().back().push_back(StringfyExpr(n, *e));
      }
    }
    PrintTable(tM, {4, 10});
  }

  void Run(Lex::TokenSequence tokens) {
    std::stack<Token> S;
    S.push(END);  // 栈底放一个$
    tokens.push({TOKEN::BLANK, END.str});
    S.push({0, init});  // 从起始状态开始推导
    TableContent app = {{{"Stack"}, {"Input"}, {"Strategy"}, {"Prefix"}}};
    std::string grammar;  // 当前分析出的语法串的前缀
    auto Error = [&]() {
      std::cout << "LL(1) Parsing Failed.\nPartial Parsing Sequence:\n";
      PrintTable(app, {30, 30, 30, 30});
      exit(0);
    };
    while (!tokens.empty()) {
      std::string expr;
      app.push_back(
          {{StringfyContainer(S, [](auto x) -> std::string { return x.str; })},
           {StringfyContainer(
               tokens, [](auto x) -> std::string { return x.second; }, 1)}});
      if (S.top().T) {  // 栈顶为终结符
        if (S.top().str ==
            tokens.front().second) {  // 弹出状态栈栈顶和符号队列队首
          if (!S.top().T || S.top() != END) grammar += S.top().str + ' ';
          tokens.pop();
          S.pop();
        } else
          Error();
      } else {
        auto T =
            M[S.top().str]
             [tokens.front().second];  // 栈顶为非终结符，则按照相应的生成式规约
        if (T.size() != 1)
          Error();
        else {
          auto &e = T.front();
          expr = StringfyExpr(S.top().str, *e);
          S.pop();
          for (int i = e->size() - 1; i >= 0; --i)
            S.push(e->at(i));  // 将生成式右部的符号逆序插入栈中
        }
      }
      app.back().push_back({expr});
      app.back().push_back({grammar});
    }
    if (!S.empty())
      Error();
    else {
      std::cout << "LL(1) Parsing Complete.\nFull Parsing Sequence:\n";
      PrintTable(app, {40, 40, 40, 40});
    }
  }
};
}  // namespace GrammarLL1

int main(int argc, const char **argv) {
  using namespace GrammarLL1;
  if (argc != 3)
    std::cerr << "Usage: ./ll1 grammar_file source_file\n";
  else {
    std::ifstream gin(argv[1]), fin(argv[2]);
    auto [init, gen] = Parse(gin);
    auto tok = Lex::clangLex.GenSequence(fin);
    LL1Automata ll1(init, gen);
    ll1.Run(tok);
  }
  return 0;
}