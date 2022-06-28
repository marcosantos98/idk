package marco.lang

class Tokenizer(private val input: String) {

    private var cursor = 0

    val tokens = mutableListOf<Token>()

    fun run() {
        while (!eof()) {
            when (input[cursor]) {
                ' ', '\n', '\t', '\r' -> cursor++
                '"' -> parseStringToken()
                else -> {
                    parseKeywordOrIdentifierToken()
                }
            }
        }
    }

    private fun getChar(): Char = input[cursor]

    private fun parseStringToken() {
        val start = cursor
        cursor++
        while (!eof() && getChar() != '"') cursor++
        cursor++
        tokens.add(Token(input.substring(start, cursor), TokenType.STRING))
    }

    private fun parseKeywordOrIdentifierToken() {
        val start = cursor
        while (!eof() && getChar().isLetter()) {
            cursor++
        }
        val value = input.substring(start, cursor)
        tokens.add(Token(value, TokenType.IDENTIFIER))
    }

    private fun eof(): Boolean = cursor >= input.length
}