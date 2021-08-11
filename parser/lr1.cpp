#include <bits/stdc++.h>

#include "grammar_util.hpp"
#include "lex.hpp"
#include "table.hpp"

namespace GrammarLR1 {

struct Item {  // 语法分析项目
  std::string a;
  const Expr *expr;
  std::string n;
  int cur;
  // [a->expr(with . before cursor), n]

  bool operator<(Item const &x) const {
    if (a != x.a) return a < x.a;
    if (n != x.n) return n < x.n;
    if (cur != x.cur) return cur < x.cur;
    return expr < x.expr;
  }
};

using ISet = std::set<Item>;  // 项目集

struct LR1Automata : GrammarAutomataBase {
  SMap<SSet> follow;
  std::set<ISet> I;   //项目集规范族
  const ISet *start;  // 包含了 [S'->.S, $] 的项目集
  std::string
      realInit;  // 由于输入是拓广文法，真实的初始状态是初始状态唯一的生成式的右部
  std::map<const ISet *, SMap<const ISet *>> goTo;
  std::map<const ISet *,
           SMap<std::tuple<std::string, const Expr *, const ISet *>>>
      action;  // action中的每个表项式一个<a,e,s>的tuple，若e和s均为NULL，表示ACC；若e为NULL，则表示shift
               // s，否则表示reduce a -> e
  std::map<const ISet *, int> id;  // 为了方便输出，给每个项目集定一个编号

  LR1Automata(std::string init, Grammar g) : GrammarAutomataBase(init, g) {
    assert(g.at(init).size() == 1 && !g.at(init).front().front().T);
    realInit = g.at(init).front().front().str;
    Epsilon();
    allTokens.insert(END);
    First();
    CalcI();
    Table();
    Print();
  }

  void Table() {  // 生成分析表
    for (auto &S : I) {
      for (auto &i : S) {
        if (i.cur == i.expr->size()) {  // 形如 A -> B. 的项目
          if (i.a != init)
            action[&S][i.n] = {i.a, i.expr, NULL};  // 正常的reduce
          else {
            assert(i.expr->size() == 1 &&
                   (!i.expr->front().T && i.expr->front().str == realInit) &&
                   i.n == "$");
            action[&S][i.n] = {
                "", NULL, NULL};  // [S -> S. , $]的状态，得到$之后的action为ACC
          }
        } else {  // 形如 A -> BC.D 的项目
          auto t = i.expr->at(i.cur);
          assert(I.count(Go(S, t)));
          auto gop = &(*I.find(Go(S, t)));  //正常向后转移一步
          if (t.T)
            action[&S][t.str] = {"", NULL,
                                 gop};  // 若下一个符号为终结符，则为shift
          else
            goTo[&S][t.str] = gop;  //否则在goto中填上到的位置，用于规约后的转移
        }
      }
    }
  }

  void CalcI() {
    I = {{Closure({{init, &g.at(init).front(), END.str, 0}})}};
    start = &(*I.begin());
    std::set<ISet> I0;
    do {
      I0 = I;
      for (auto &s : I)
        for (auto &t : allTokens) {
          auto gos = Go(s, t);
          if (!gos.empty()) I.insert(gos);
        }
    } while (I.size() > I0.size());  // 重复直到项目集规范族不再变化
    int no = 0;
    for (auto &k : I) id[&k] = no++;
  }

  ISet Closure(ISet S) {
    ISet S0;
    do {
      S0 = S;
      for (auto i : S0) {
        if (i.cur != i.expr->size() && !i.expr->at(i.cur).T) {
          Expr backward(i.expr->begin() + i.cur + 1, i.expr->end());
          backward.push_back({1, i.n});
          for (auto &n : SequenceFirst(
                   backward.begin(),
                   backward.end())) {  // 处理后缀符号串的First，把相应表项加入
            for (auto const &expr : g.at(i.expr->at(i.cur).str))
              S.insert({i.expr->at(i.cur).str, &expr, n, 0});
          }
        }
      }
    } while (S.size() > S0.size());
    return S;
  }

  ISet Go(ISet S, Token X) {
    ISet T;
    for (auto i : S) {  // 选出能向后走X的项目
      if (i.cur != i.expr->size() && i.expr->at(i.cur) == X) {
        i.cur++;
        T.insert(i);
      }
    }
    return Closure(T);
  }
  std::string StringfyAction(
      std::tuple<std::string, const Expr *, const ISet *> p) {
    auto [a, expr, s] = p;
    if (!expr && !s)
      return "ACC";
    else if (s) {
      return "shift " + std::to_string(id[s]);
    } else {
      return "reduce " + StringfyExpr(a, *expr);
    }
  }

  void Print() {
    std::cout << I.size() << " sets.\n";
    TableContent t1, t2;
    t1.push_back({{"No."}, {"Elements"}});
    std::vector<SMap<std::string>> v;
    for (auto &s : I) {
      v.push_back({});
      for (auto &i : s)
        v.back()[StringfyExpr(i.a, *i.expr, i.cur)] += i.n + ' ';
    }
    for (size_t i = 0; i < v.size(); ++i) {
      t1.push_back({{std::to_string(i)}, {}});
      for (auto &[x, y] : v[i]) t1.back().back().push_back(x + ',' + y);
    }
    PrintTable(t1, {5, 30});
    SSet VN, VT;
    for (auto &i : allTokens) (i.T ? VT : VN).insert(i.str);

    t2.push_back({{""}});
    for (auto &s : VT) t2.back().push_back({s});
    for (auto &s : VN) t2.back().push_back({s});
    for (auto &k : I) {
      t2.push_back({{std::to_string(id[&k])}});
      for (auto &s : VT)
        t2.back().push_back(
            {action[&k].count(s) ? StringfyAction(action[&k][s]) : ""});
      for (auto &s : VN)
        t2.back().push_back(
            {goTo[&k].count(s) ? std::to_string(id[goTo[&k][s]]) : ""});
    }
    std::vector<int> lw = {3};
    for (auto &s : VT) lw.push_back(20);
    for (auto &s : VN) lw.push_back(10);
    std::cout << "Analysis table:\n";
    std::cout << std::setfill(' ') << std::setw(5) << ""
              << "action" << std::setw(21 * VT.size() - 6) << ""
              << "goto" << '\n';
    PrintTable(t2, lw);
  }

  void Run(Lex::TokenSequence &tokens) {
    tokens.push({TOKEN::BLANK, END.str});
    std::stack<Token> prefix;         // 语法前缀栈
    std::stack<const ISet *> states;  // 状态栈
    states.push(start);               // 从[S'->.S,$]所在项目集开始

    TableContent t = {{{"States"}, {{"Prefix"}}, {{"Input"}}, {{"Strategy"}}}};
    while (!tokens.empty()) {
      auto Error = [&]() {
        std::cout << "LR(1) Parsing Failed.\nPartial Parsing Sequence:\n";
        PrintTable(t, {25});
        exit(0);
      };
      if (!action[states.top()].count(tokens.front().second))
        Error();
      else {
        auto [a, expr, s] = action[states.top()][tokens.front().second];
        t.push_back(
            {{StringfyContainer(
                 states,
                 [&](auto x) -> std::string { return std::to_string(id[x]); })},
             {StringfyContainer(prefix,
                                [](auto x) -> std::string { return x.str; })},
             {StringfyContainer(
                 tokens, [](auto x) -> std::string { return x.second; }, 1)},
             {StringfyAction({a, expr, s})}});
        if (!expr && !s) {  // ACC 如果不出问题的话，此时tokens应该空了
          tokens.pop();
        } else if (s) {  // shift
          prefix.push({1, tokens.front().second});
          tokens.pop();
          states.push(s);
        } else {  // reduce
          for (int r = expr->size(); r--; prefix.pop(), states.pop())
            assert(prefix.top() == expr->at(r));
          prefix.push({0, a});
          assert(goTo[states.top()].count(prefix.top().str));
          states.push(goTo[states.top()][prefix.top().str]);
        }
      }
    }
    std::cout << "LR(1) Parsing Complete.\nFull Parsing Sequence:\n";
    PrintTable(t, {40});
  }
};

using ItemSet = std::set<Item>;
}  // namespace GrammarLR1
int main(int argc, const char **argv) {
  using namespace GrammarLR1;
  if (argc != 3)
    std::cerr << "Usage: ./lr1 grammar_file source_file\n";
  else {
    std::ifstream gin(argv[1]), fin(argv[2]);
    auto [init, gen] = Parse(gin);
    LR1Automata lr1(init, gen);
    auto tok = Lex::clangLex.GenSequence(fin);
    lr1.Run(tok);
  }
  return 0;
}