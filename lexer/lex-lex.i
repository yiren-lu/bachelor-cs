%{

enum {
    END,
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
const char TOKEN_NAME[][20]={
    "","ID", "INT",  "FLOAT", "RELOP", "ASSIGN", "OPER",
    "COMMA",   "SINGLE_MARK", "PREP", "STR",   "BLANK", "CHAR",   "ERR"
};

%}

%%

[ \t\n]                                                                                     {  }
(_|[A-Za-z])(_|[A-Za-z0-9])*                                                                { return ID; }
[+|-]?((([1-9])([0-9])*)|(0x[0-9A-Z]+)|(0[0-7]*))(U|L|UL|LL|ULL)?                           { return INT; }
[+|-]?((([1-9][0-9]*)\.[0-9]+(e[+|-]?[1-9][0-9]*)?)|[1-9][0-9]*e[+|-]?[1-9][0-9]*)(L)?      { return FLOAT; }
"<"                                                                                         { return RELOP; }
"<="                                                                                        { return RELOP; }
">"                                                                                         { return RELOP; }
">="                                                                                        { return RELOP; }
"=="                                                                                        { return RELOP; }
"="                                                                                         { return ASSIGN; }
"+"                                                                                         { return OPER; }
"-"                                                                                         { return OPER; }
"*"                                                                                         { return OPER; }
"/"                                                                                         { return OPER; }
">>"                                                                                        { return OPER; }
"<<"                                                                                        { return OPER; }
"&"                                                                                         { return OPER; }
"|"                                                                                         { return OPER; }
"^"                                                                                         { return OPER; }
(\/\/[^\n]*\n)|(\/\*[^*]*\*+([^/*][^*]*\*+)*\/)                                            { return COMMA; }
"("                                                                                         { return SINGLE_MARK; }
")"                                                                                         { return SINGLE_MARK; }
"["                                                                                         { return SINGLE_MARK; }
"]"                                                                                         { return SINGLE_MARK; }
"{"                                                                                         { return SINGLE_MARK; }
"}"                                                                                         { return SINGLE_MARK; }
"."                                                                                         { return SINGLE_MARK; }
"!"                                                                                         { return SINGLE_MARK; }
"~"                                                                                         { return SINGLE_MARK; }
";"                                                                                         { return SINGLE_MARK; }
":"                                                                                         { return SINGLE_MARK; }
"?"                                                                                         { return SINGLE_MARK; }
","                                                                                         { return SINGLE_MARK; }
#[^\n]*\n                                                                                   { return PREP; }
\"([^\\^\n]|(\\[^\n]))*\"                                                                   { return STR; }
'([^\\^\n]|(\\[^\n]))+'                                                                     { return CHAR; }
.                                                                                           { return ERR; }
%%

int main (int argc, char ** argv){
    yyin=fopen(argv[1],"r");
#define RESET_COLOR "\033[0m"
#define FADE_COLOR "\033[2m"
    for (char c; (c = yylex()) != 0; )
        fprintf(yyout,FADE_COLOR"<"RESET_COLOR"%s"FADE_COLOR","RESET_COLOR"%s"FADE_COLOR">\n",TOKEN_NAME[c],yytext);
    fclose(yyin);
    return 0;
}