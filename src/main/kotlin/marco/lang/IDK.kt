package marco.lang

data class Token(val lex: String, val type: TokenType)

enum class TokenType {
    STRING, IDENTIFIER
}