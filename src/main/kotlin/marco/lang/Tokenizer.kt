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
        var value = input.substring(start, cursor)

        // FIXME: 28/06/22 Hack to escape newLines at the end of a string x)
        var endsWithNewLine = false
        if(value.substring(1, value.length-1).endsWith("\\n")) {
            value = value.substring(0, value.length - 3) + "\""
            endsWithNewLine = true
        }

        val extra = StringExtra(endsWithNewLine)

        tokens.add(Token(value, TokenType.STRING, extra))
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