package marco.lang

data class Token(val lex: String, val type: TokenType, val extra: Any? = null)

data class StringExtra(val value: ByteArray)

val keywords = listOf("var", "fn", "ret")

enum class TokenType {
    KEYWORD, IDENTIFIER,
    EQ_BIND,
    STRING, NUMBER,
    NEQ, EQ, LT, GT, LTEQ, GTEQ,
    LP, RP, LCB, RCB,
    COMMA,
    PLUS, MINUS, MUL, DIV, MOD,
    EOF
}
