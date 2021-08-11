#include <bits/stdc++.h>

struct Token {      // 文法符号的定义
  bool T;           // T=1 为终止符
  std::string str;  // 符号的名称
  bool operator<(Token const &x) const {
    return T != x.T ? T < x.T : str < x.str;
  }
  bool operator==(Token const &x) const { return T == x.T && str == x.str; }
  bool operator!=(Token const &x) const { return T != x.T || str != x.str; }
};

const Token EPSILON = {1, "eps"},  //空符号
    END = {1, "$"};                //终止符

using Expr = std::vector<Token>;  // 生成式右部，定义成Token序列
template <class T>
using SMap = std::map<std::string, T>;
using SSet =
    std::set<std::string>;  // 为了方便，将和string有关的set和map重命名一下
using Grammar = SMap<
    std::vector<Expr>>;  // 文法定义成从左部到包含着所有可能的右部的map，比如A
                         // -> B|C，就会被存储为[A]->{{B},{C}}

std::string StringfyExpr(std::string a, Expr b,
                         int dot = -1) {  //将生成式输出成字符串
  a += " -> ";
  if (b.empty()) b.push_back(EPSILON);
  for (auto &i : b) a += (!dot-- ? "." : "") + i.str;
  if (!dot) a += '.';
  return a;
}

template <class T, class F>
std::string StringfyContainer(std::stack<T> a, F f,
                              bool rev = 0) {  //将容器中的元素输出成字符串
  std::string s;
  for (; !a.empty(); a.pop())
    s = (rev ? s + (s == "" ? "" : " ") + f(a.top()) : f(a.top()) + ' ' + s);
  return s;
}

template <class T, class F>
std::string StringfyContainer(std::queue<T> a, F f, bool rev = 0) {
  std::string s;
  for (; !a.empty(); a.pop())
    s = (rev ? s + (s == "" ? "" : " ") + f(a.front())
             : f(a.front()) + ' ' + s);
  return s;
}

std::pair<std::string, Grammar> Parse(
    std::ifstream &gin) {  //将描述文法的字符串存储到代码中相应的数据结构中
  std::string init;
  gin >> init;  //首行为起始状态
  Grammar gen;
  gin.get();
  for (std::string s; std::getline(gin, s);) {
    std::istringstream iss(s);
    std::vector<std::string> v;
    while (std::getline(iss, s, ' ')) v.push_back(s);
    assert(v[1] == "->");  // 左部 -> 右部
    gen[v[0]].push_back({});
    for (size_t i = 2; i < v.size(); ++i) {
      if (v[i] == "|")  // 不同的右部用 | 分隔
        gen[v[0]].push_back({});
      else
        gen[v[0]].back().push_back({1, v[i]});
    }
  }
  for (auto &[a, exprs] : gen) {
    for (auto &expr : exprs) {
      for (auto &i : expr) i.T = !gen.count(i.str);
      if (expr.front() == EPSILON)
        expr = {};  // 将转移到空的生成式右部置为空vector方便处理
    }
  }
  return {init, gen};
}

struct GrammarAutomataBase {  // 语法分析自动机的基类
  const std::string init;     // 初始状态
  const Grammar g;            // 语法
  std::set<Token> allTokens;  // 所有符号的集合
  SSet eps;                   // 能转移到空的非终止符集合
  SMap<SSet> first;           // first集合

  GrammarAutomataBase(std::string init, Grammar g) : init(init), g(g) {}

  SSet SequenceFirst(
      Expr::const_iterator begin,
      Expr::const_iterator
          end) {  // 求出一个符号序列Y1Y2Y3..Yn的first，要考虑每个前缀能否转移到空
    if (begin == end)
      return {EPSILON.str};
    else {
      SSet fs;
      bool allEps = true;
      for (; begin != end; ++begin) {
        if (begin->T)
          fs.insert(begin->str);
        else
          for (auto &s : first[begin->str])
            if (s != EPSILON.str) fs.insert(s);
        if (begin->T || !eps.count(begin->str)) {
          allEps = 0;
          break;
        }
      }
      if (allEps) {
        std::cerr << "strange\n";
        throw;
        fs.insert(EPSILON.str);
      }
      return fs;
    }
  }

  void Epsilon() {              // 求出能转移到epsilon的非终结符
    std::queue<std::string> Q;  // 能转移到epsilon的非终结符队列
    SMap<std::vector<std::pair<std::string, const Expr *>>>
        possible;  // 对于每个非终结符，维护所有右部包含它的生成式
    for (auto &[a, exprs] : g) {
      allTokens.insert({0, a});
      for (auto &expr : exprs) {
        if (expr.empty()) {
          eps.insert(a);
          Q.push(a);  // 将所有能转移到空的符号插入队列
        } else if (std::count_if(expr.begin(), expr.end(),
                                 [](auto const &x) -> bool { return x.T; }) ==
                   0)
          for (auto &i : expr)
            possible[i.str].push_back(
                {a,
                 &expr});  // 若一个生成式的右部只包含非终结符，则每个非终结符能转移到空都可能给整个式子转移到空带来一线生机
        for (auto &i : expr) allTokens.insert(i);
      }
    }
    while (!Q.empty()) {
      auto s = Q.front();
      Q.pop();
      for (auto &[a, expr] : possible[s])
        if (!eps.count(a))
          if (!std::count_if(expr->begin(), expr->end(),
                             [&](auto const &sub) -> bool {
                               return sub.T || !eps.count(sub.str);
                             })) {  // 若生成式右部的所有符号都能转移到空了
            eps.insert(a);
            Q.push(a);
          }
      possible[s].erase(
          std::remove_if(
              possible[s].begin(), possible[s].end(),
              [&](auto const &x) -> bool { return eps.count(x.first); }),
          possible[s].end());  // 删掉所有整体能变成空的生成式，避免冗余判断
    }
  }

  void First() {                                     // 求First集合
    std::map<Token, std::map<Token, bool>> include;  // First集合的包含关系
    for (auto &i : allTokens) include[i][i] = 1;
    for (auto &[a, exprs] : g) {
      for (auto &expr : exprs) {
        for (auto &i : expr) {
          include[{0, a}][i] =
              1;  // 生成式右部的每个符号，如果它前面的部分能转移到空，那么生成式左部的First就包含了它的First
          if (i.T || !eps.count(i.str)) break;
        }
      }
    }

    for (auto &k : allTokens)
      for (auto &i : allTokens)
        for (auto &j : allTokens)
          include[i][j] |= include[i][k] & include[k][j];  // Warshall求传递闭包

    for (auto &i : allTokens)
      if (!i.T) {
        for (auto &j : allTokens)
          if (j.T && include[i][j]) first[i.str].insert(j.str);
        if (eps.count(i.str)) first[i.str].insert(EPSILON.str);
      }
  }
};
