#include <bits/stdc++.h>

using TableContent = std::vector<std::vector<std::vector<std::string>>>;
void PrintTable(TableContent const &t,
                std::vector<int> lw) {  //用于以表格样式输出vector内容的函数
  int n = t.size(), m = t.front().size();
  while (lw.size() < m) lw.push_back(lw.back());
  auto Divide = [&]() {
    std::cout << '+';
    for (auto &x : lw)
      std::cout << std::setw(x) << std::setfill('-') << "" << '+';
    std::cout << '\n';
  };
  Divide();
  for (auto &l : t) {
    int lht = 0;
    for (auto &v : l) lht = std::max(lht, (int)v.size());
    for (int r = 0; r < lht; ++r) {
      std::cout << "|";
      for (int i = 0; i < lw.size(); ++i)
        std::cout << std::setw(lw[i]) << std::setfill(' ') << std::left
                  << (r < l[i].size() ? l[i][r] : "") << "|";
      std::cout << '\n';
    }
    Divide();
  }
}