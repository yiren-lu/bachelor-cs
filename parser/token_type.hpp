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
