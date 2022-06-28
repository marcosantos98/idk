package marco.lang

data class Token(val lex: String, val type: TokenType, val extra: Any? = null)

data class StringExtra(val newLine: Boolean)

enum class TokenType {
    STRING, IDENTIFIER
}