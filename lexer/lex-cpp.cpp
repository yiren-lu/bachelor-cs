#include <bits/stdc++.h>

enum STATES {
    FAIL = -1,  // 当前状态没有合适的转移，则转移到失配状态
    INIT,       // 初始状态
    ALPHA,  // 读入了一个字母或者_到达的状态，表示着一个标识符的开始
    SINGLE_MARK,  // 读入一个在词法分析中不起作用的符号到达的状态，如([{等
    DIV,          // 读入一个/
    LESS,         // 读入一个<
    GREATER,      // >
    PREP_SHARP,   // #
    ZERO,         // 0
    BASE10,       //表示十进制数的状态
    SINGLE_DOT,   //读入一个.到达的状态
    AND,          // &
    OR,           // |
    CHAR_QUOTATION,  // '
    CHAR_PREF,       // 读入'和一个字符之后到达的状态
    CHAR_END,        // 表示字符结尾的状态
    PLUS,            // 读入+到达状态
    MINUS,           // -
    EQUAL,           // =
    STR_QUOTATION,   // "
    ID,              // 表示标识符的状态
    DD_COMMA,        // 输入//之后的状态
    DS_COMMA,        // 输入/*之后的状态
    LESS_EQUAL,      // <=
    BIN_OPER,        // 表示二元运算符的状态
    GREATER_EQUAL,   // >=
    BASE8,           //表示八进制数的状态
    BASE16,          //表示十六进制数字的状态
    FLOAT_DOT,  // 表示输入了浮点数到小数点一位时到达的状态
    AAND,       // &&
    OOR,        // ||
    PPLUS,      // ++
    MMINUS,     // --
    EEQUAL,     // ==
    STR_END,    // 表示字符串结尾的状态
    DD_COMMA_END,  // 表示单行注释结束的状态
    DS_COMMA_S,    // 表示多行注释结尾输入了一个*的状态
    SELF_OPER,     // 表示自操作运算符的状态，如+=，>>=
    INT_U,         //整数结尾输入后缀U的状态
    INT_L,         // 整数结尾输入后缀L的状态
    FLOAT,         // 表示浮点数的状态
    DS_COMMA_END,  //表示多行注释结束的状态
    INT_UL,        //整数结尾输入后缀UL的状态
    INT_LL,        // 整数结尾输入后缀LL的状态
    FLOAT_E,       // 浮点数后输入e的状态
    INT_ULL,       // 整数结尾输入ULL的状态
    FLOAT_E_FLAG,  // 浮点数e后输入了一个+/-号的状态
    FL_INT,        // 浮点数e和+/-号后面跟了一个整数的状态
    FLOAT_L,       // 浮点数后加入L后缀的状态
    STR_DASH,      //正在输入的字符串中输入了一个\到达的状态
    CHAR_DASH      //正在输入的字符中输入了一个\到达的状态
};

enum class TOKEN {
    NOT_END,      // 未结束的符号
    ID,           // 表示标识符的符号
    INT,          // 整数
    FLOAT,        //浮点数
    RELOP,        // 关系比较符
    ASSIGN,       // 赋值符号
    OPER,         // 运算符
    COMMA,        // 注释
    SINGLE_MARK,  // 单个不在词法分析中产生作用的符号
    PREP,         // 预处理语句
    STR,          // 字符串
    BLANK,        // 空白符
    CHAR,         // 字符
    ERR           // 表示错误
};

const std::string TOKEN_NAME[] = {
    "NOT_END", "ID",          "INT",  "FLOAT", "RELOP", "ASSIGN", "OPER",
    "COMMA",   "SINGLE_MARK", "PREP", "STR",   "BLANK", "CHAR",   "ERR"};

using Func = std::function<bool(char)>;

Func Include(std::set<char> S) {  // 返回一个判断字符是否在集合内的function
    return [=](char c) -> bool { return S.count(c); };
}

Func Exclude(std::set<char> S) {  // 返回判断字符是否不在集合内的function
    return [=](char c) -> bool { return !S.count(c); };
}

Func IsBase8 = [](char c) -> bool {  // 返回判断字符是否是8进制串中的合法字符
    return '0' <= c && c <= '7';
};
Func IsBase16 = [](char c) -> bool {  // 返回判断字符是否是16进制串中的合法字符
    return isdigit(c) or ('A' <= c && c <= 'F');
};

Func IsBlank = [](char c) -> bool {  // 判断字符是否是空白符
    return (c == '\t' || c == ' ' || c == '\n');
};

Func Between(char l, char r) {  // 返回判断字符是否在一个指定区间的function
    return [=](char c) -> bool { return l <= c && c <= r; };
}

Func InCharSet = [&](char c) -> bool {  // 判断字符是否在支持的字符集中
    return isalpha(c) || isdigit(c) || IsBlank(c) ||
           std::set<char>({'@', '`',  '$',  // 只可能出现在string中的字符
                           '!', '"',  '#', '%', '&', '(', ')', '*', '+', ',',
                           '-', '\'', '.', '/', ':', ';', '<', '=', '>', '?',
                           '[', '\\', ']', '^', '_', '{', '|', '}', '~'})
               .count(c);
};

using Token = std::tuple<int, int, TOKEN, std::string>;

struct LexAutomata {
    struct State {                                // 自动机状态的定义
        TOKEN type;                               // 状态的属性
        std::vector<std::pair<Func, STATES>> go;  // 表示转移的数组
        STATES
        Go(char c) {  // 转移函数
            for (auto &[pred, s] : go)
                if (pred(c)) return s;
            return FAIL;
        }
    };
    std::vector<State> state;
    LexAutomata()  // 自动机的初始化过程
        : state(
              {// INIT
               {TOKEN::NOT_END,
                {{[](char c) -> bool { return isalpha(c) || c == '_'; }, ALPHA},
                 {Include({'!', ';', '(', ')', '[', ']', '{', '}', ',', '?',
                           ':', '~'}),
                  SINGLE_MARK},
                 {Include({'/'}), DIV},
                 {Include({'<'}), LESS},
                 {Include({'>'}), GREATER},
                 {Include({'*', '^', '%'}), BIN_OPER},
                 {Include({'#'}), PREP_SHARP},
                 {Include({'0'}), ZERO},
                 {Between('1', '9'), BASE10},
                 {Include({'.'}), SINGLE_DOT},
                 {Include({'&'}), AND},
                 {Include({'|'}), OR},
                 {Include({'+'}), PLUS},
                 {Include({'-'}), MINUS},
                 {Include({'='}), EQUAL},
                 {Include({'"'}), STR_QUOTATION},
                 {IsBlank, INIT},
                 {Include({'\''}), CHAR_QUOTATION}}},
               // ALPHA
               {TOKEN::ID,
                {{[](char c) -> bool { return isalnum(c) || c == '_'; }, ID}}},
               // SINGLE_MARK
               {TOKEN::SINGLE_MARK, {}},
               // DIV
               {TOKEN::OPER,
                {{Include({'='}), SELF_OPER},
                 {Include({'/'}), DD_COMMA},
                 {Include({'*'}), DS_COMMA}}},
               // LESS
               {TOKEN::RELOP,
                {{Include({'='}), LESS_EQUAL}, {Include({'<'}), BIN_OPER}}},
               // GREATER
               {TOKEN::RELOP,
                {{Include({'='}), GREATER_EQUAL}, {Include({'>'}), BIN_OPER}}},
               // PREP_SHARP
               {TOKEN::PREP, {{Exclude({'\n'}), PREP_SHARP}}},
               // ZERO
               {TOKEN::INT,
                {{IsBase8, BASE8},
                 {Include({'x'}), BASE16},
                 {Include({'.'}), FLOAT_DOT}}},
               // BASE10
               {TOKEN::INT,
                {{isdigit, BASE10},
                 {Include({'U'}), INT_U},
                 {Include({'L'}), INT_L},
                 {Include({'.'}), FLOAT_DOT},
                 {Include({'e'}), FLOAT_E}}},
               // SINGLE_DOT
               {TOKEN::OPER, {{isdigit, FLOAT}}},
               // AND
               {TOKEN::OPER,
                {{Include({'&'}), AAND}, {Include({'='}), SELF_OPER}}},
               // OR
               {TOKEN::OPER,
                {{Include({'|'}), OOR}, {Include({'='}), SELF_OPER}}},
               // CHAR_QUOTATION
               {TOKEN::NOT_END,
                {{Include({'\\'}), CHAR_DASH},
                 {Exclude({'\\', '\'', '\n'}), CHAR_PREF}}},
               // CHAR_PREF
               {TOKEN::NOT_END,
                {{Include({'\\'}), CHAR_DASH},
                 {Exclude({'\\', '\'', '\n'}), CHAR_PREF},
                 {Include({'\''}), CHAR_END}}},
               // CHAR_END
               {TOKEN::CHAR, {}},
               // PLUS
               {TOKEN::OPER,
                {{Include({'+'}), PPLUS},
                 {Include({'0'}), ZERO},
                 {Between('1', '9'), BASE10},
                 {Include({'='}), SELF_OPER}}},
               // MINUS
               {TOKEN::OPER,
                {{Include({'-'}), MMINUS},
                 {Include({'0'}), ZERO},
                 {Between('1', '9'), BASE10},
                 {Include({'='}), SELF_OPER}}},
               // EQUAL
               {TOKEN::ASSIGN, {{Include({'='}), EEQUAL}}},
               // STR_QUOTATION
               {TOKEN::NOT_END,
                {{Include({'\\'}), STR_DASH},
                 {Exclude({'\\', '"', '\n'}), STR_QUOTATION},
                 {Include({'"'}), STR_END}}},
               // ID
               {TOKEN::ID, {{isalnum, ID}}},
               // DD_COMMA
               {TOKEN::COMMA,
                {{Exclude({'\n'}), DD_COMMA}, {Include({'\n'}), DD_COMMA_END}}},
               // DS_COMMA,
               {TOKEN::NOT_END,
                {{Exclude({'*'}), DS_COMMA}, {Include({'*'}), DS_COMMA_S}}},
               // LESS_EQUAL
               {TOKEN::RELOP, {}},
               // BIN_OPER
               {TOKEN::OPER, {{Include({'='}), SELF_OPER}}},
               // GREATER_EQUAL
               {TOKEN::RELOP, {}},
               // BASE8
               {TOKEN::INT,
                {{IsBase8, BASE8},
                 {Include({'U'}), INT_U},
                 {Include({'L'}), INT_L}}},
               // BASE16
               {TOKEN::INT,
                {{IsBase16, BASE16},
                 {Include({'U'}), INT_U},
                 {Include({'L'}), INT_L}}},
               // FLOAT_DOT
               {TOKEN::NOT_END, {{isdigit, FLOAT}}},
               // AAND
               {TOKEN::OPER, {}},
               // OOR
               {TOKEN::OPER, {}},
               // PPLUS
               {TOKEN::OPER, {}},
               // MMINUS
               {TOKEN::OPER, {}},
               // EEQUAL
               {TOKEN::RELOP, {}},
               // STR_END
               {TOKEN::STR, {}},
               // DD_COMMA_END
               {TOKEN::COMMA, {}},
               // DS_COMMA_S
               {TOKEN::NOT_END,
                {{Include({'/'}), DS_COMMA_END},
                 {Exclude({'/', '*'}), DS_COMMA},
                 {Include({'*'}), DS_COMMA_S}}},
               // SELF_OPER
               {TOKEN::OPER, {}},
               // INT_U
               {TOKEN::INT, {{Include({'L'}), INT_UL}}},
               // INT_L
               {TOKEN::INT, {{Include({'L'}), INT_LL}}},
               // FLOAT
               {TOKEN::FLOAT,
                {{isdigit, FLOAT},
                 {Include({'e'}), FLOAT_E},
                 {Include({'L'}), FLOAT_L}}},
               // DS_COMMA_END
               {TOKEN::COMMA, {}},
               // INT_UL
               {TOKEN::INT, {{Include({'L'}), INT_ULL}}},
               // INT_LL
               {TOKEN::INT, {}},
               // FLOAT_E
               {TOKEN::NOT_END,
                {{isdigit, FL_INT}, {Include({'+', '-'}), FLOAT_E_FLAG}}},
               // INT_ULL
               {TOKEN::INT, {}},
               // FLOAT_E_FLAG
               {TOKEN::NOT_END, {{isdigit, FL_INT}}},
               // FL_INT
               {TOKEN::FLOAT, {{isdigit, FL_INT}, {Include({'L'}), FLOAT_L}}},
               // FLOAT_L
               {TOKEN::FLOAT, {}},
               // STR_DASH
               {TOKEN::NOT_END, {{Exclude({'\n'}), STR_QUOTATION}}},
               // CHAR_DASH
               {TOKEN::NOT_END, {{Exclude({'\n'}), CHAR_PREF}}}}) {}

    std::vector<Token> Run(
        std::ifstream &stream) {  //将流在自动机上匹配输出符号的过程
        std::string token;        //当前输入的符号前缀
        std::vector<Token> tokens;  // 输出的符号列表
        STATES s = INIT;            // 当前状态，初始设置为INIT
        std::vector<int> linec = {
            1};  // 表示每行分别已经输入了多少个字符，用于统计行号和列号
        int line = 1, col = 1;  // 当前符号的开始位置的行列
        bool eof = 0;           // 是否已经读到流的结尾
        auto Output = [&](TOKEN type) {
            auto IsNumberToken = [&](TOKEN c)
                -> bool {  // 判断是否是数字状态的函数，在下面的判断中可以少些几遍
                return c == TOKEN::INT || c == TOKEN::FLOAT;
            };

            auto Can = [&](std::pair<TOKEN, std::string> a,
                           std::pair<TOKEN, std::string> b)
                -> bool {  // 判断两个符号能否相邻的函数
                if (b.first == TOKEN::ID) return !IsNumberToken(a.first);
                if (b.first == TOKEN::PREP) return a.second == "\n";
                if (a.first == TOKEN::ERR)
                    return b.first == TOKEN::BLANK ||
                           b.first == TOKEN::SINGLE_MARK ||
                           b.first == TOKEN::OPER;
                return true;
            };
            if (tokens.empty())
                tokens.push_back({line, col, type, token});
            else {
                auto &[l0, c0, typ0, tok0] = tokens.back();
                bool primaryFail = false;
                if (IsNumberToken(type)) {  //将1和+1的形式拆成1 + 1
                    if (IsNumberToken(typ0) || typ0 == TOKEN::ID) {
                        if (token[0] == '+' || token[0] == '-') {
                            tokens.push_back(
                                {line, col, TOKEN::OPER, token.substr(0, 1)});
                            tokens.push_back(
                                {line, col + 1, type,
                                 token.substr(1, token.length() - 1)});
                        } else
                            primaryFail = true;
                    }
                }
                if (primaryFail ||
                    !Can({typ0, tok0},
                         {type, token})) {  //不能相邻的符号，合在一起组成ERR
                    std::get<2>(tokens.back()) = TOKEN::ERR;
                    std::get<3>(tokens.back()) += token;
                } else
                    tokens.push_back({line, col + 1, type, token});
            }
            line = linec.size();
            col = linec.back();
            token = "";
        };
        for (char c; (c = stream.get()) != EOF;) {
            if (!InCharSet(c)) {
                std::cerr << "Unsupported character " << c
                          << '\n';  // 不在字符集中的符号，直接忽略
            } else {
                if (c == '\n')  // 新行开始
                    linec.push_back(1);
                else
                    linec.back()++;       // 当前行多了一个字符
                auto n = state[s].Go(c);  // 计算转移到的状态
                if (s == INIT &&
                    n ==
                        INIT) {  // 只有空白分隔符能从INIT转移到INIT，此时直接输出不用处理
                    token += c;
                    Output(TOKEN::BLANK);
                } else if (n == FAIL) {  // 失配了
                    if (state[s].type ==
                        TOKEN::
                            NOT_END) {  // 上一个状态不是终止态，则认为发生了错误
                        token += c;
                        Output(TOKEN::ERR);  //报错
                    } else {
                        stream.unget();  // 回退一个字符
                        if (c == '\n')
                            linec.pop_back();
                        else
                            linec.back()--;
                        Output(state[s].type);  // 将已经结束的符号输出
                    }
                    s = INIT;
                } else {  //没有发生什么问题，平平无奇的转移
                    s = n;
                    token += c;
                }
            }
        }
        if (token != "")
            Output(state[s].type == TOKEN::NOT_END ? TOKEN::ERR
                                                   : state[s].type);
        return tokens;
    }
} lam;

namespace Formatter {
static const std::string COLOR[] = {
    //在Linux Shell中控制颜色的字符串
    "\e[0;31m"
    "\e[0;32m",
    "\e[1;33m",
    "\e[1;34m",
    "\e[1;35m",
};
#define REVERT_COLOR "\e[7m"
#define RESET_COLOR "\033[0m"
#define FADE_COLOR "\033[2m"
void Code(std::vector<Token> output) {
    int lino = 0;
    for (auto [l, c, typ, tok] : output) {
        auto GColor = []() -> std::string {
            static int cc;
            return COLOR[cc++ % 4];
        };
        auto color = GColor();  //循环使用4种颜色输出符号，便于区分辨认
        if (typ == TOKEN::ERR)
            color += REVERT_COLOR;  //对于错误的符号，用反色来强调
        if (lino == 0) tok = '\n' + tok;
        for (char c : tok) {
            if (c == '\n')
                std::cerr << RESET_COLOR << c << FADE_COLOR << std::setw(3)
                          << std::setfill(' ') << ++lino << '\t';  // 输出行号
            else
                std::cerr << RESET_COLOR + color << c;
        }
        std::cerr << RESET_COLOR;
    }
    std::cerr << "\n\n" << lino << " lines in total.\n";
}

void TokenList(std::vector<Token> output) {
    for (auto &[l, c, typ, tok] : output)
        if (typ != TOKEN::BLANK && typ != TOKEN::COMMA && typ != TOKEN::PREP) {
            std::cerr << std::setw(30) << std::setfill(' ') << std::left
                      << std::string(RESET_COLOR FADE_COLOR "line " +
                                     std::to_string(l) + ", column " +
                                     std::to_string(c) + ": ");
            std::cerr << RESET_COLOR << COLOR[0] << "<" << COLOR[1];
            if (typ == TOKEN::ERR) std::cerr << REVERT_COLOR;
            std::cerr << TOKEN_NAME[(int)typ];
            std::cerr << RESET_COLOR << COLOR[0] << "," << COLOR[2];
            for (char c : tok)
                if (c == '\n')
                    std::cerr << "\\n";
                else if (c == '\t')
                    std::cerr << "\\t";
                else
                    std::cerr << c;
            std::cerr << COLOR[0] << ">\n";
        }
    std::cerr << RESET_COLOR;
}
}  // namespace Formatter

int main(int argc, const char **argv) {
    std::ifstream src(argv[1]);
    auto tokens = lam.Run(src);
    src.close();
    std::cerr << "Colored Source Code:\n";
    Formatter::Code(tokens);
    std::cerr << "\n\nToken List:\n";
    Formatter::TokenList(tokens);
    std::cerr << RESET_COLOR "\n\nToken Counts:\n";
    std::map<std::string, int> tcount;
    int ccount = 0;
    for (auto &[l, c, typ, tok] : tokens) {
        tcount[TOKEN_NAME[(int)typ]]++;
        ccount += tok.length();
    }
    for (auto &[t, c] : tcount)
        std::cerr << "Number of " << std::setw(15) << std::right << t << '\t'
                  << c << '\n';

    std::cerr << RESET_COLOR "\n\nChar Counts: " << ccount << '\n';
    return 0;
}