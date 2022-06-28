package marco.lang

data class Token(val lex: String, val type: TokenType, val extra: Any? = null)

data class StringExtra(val value: ByteArray)

enum class TokenType {
    STRING, IDENTIFIER
}