g++ recursive-descent.cpp -o recursive-descent -Wall -Wextra -std=c++17 -fsanitize=undefined -g
g++ ll1.cpp -o ll1 -Wall -Wextra -std=c++17 -fsanitize=undefined -g
g++ lr1.cpp -o lr1 -Wall -Wextra -std=c++17 -fsanitize=undefined -g
flex lex.l && bison -vtdy yacc.y && gcc -o flex-bison y.tab.c lex.yy.c && gcc -o cc y.tab.c lex.yy.c