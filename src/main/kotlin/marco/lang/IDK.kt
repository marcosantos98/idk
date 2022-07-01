package marco.lang

data class Token(val lex: String, val type: TokenType, val extra: Any? = null)

data class StringExtra(val value: ByteArray)

enum class Keywords(val lex: String) {
    VARIABLE("var"), FUNCTION("fn"), RETURN("ret"), WHILE("while"), IF("if"), ELSE("else");

    companion object {
        fun isValidKeyword(lex: String): Boolean {
            return values().any { it.lex == lex }
        }
    }
}

enum class TokenType {
    KEYWORD, IDENTIFIER,
    EQ_BIND,
    STRING, NUMBER,
    NEQ, EQ, LT, GT, LTEQ, GTEQ,
    LP, RP, LCB, RCB,
    COMMA, SEMI_COLON,
    PLUS, MINUS, MUL, DIV, MOD,
    EOF
}
