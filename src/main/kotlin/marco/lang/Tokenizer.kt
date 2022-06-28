package marco.lang

class Tokenizer(input: String) {

    private var cursor = 0

    val tokens = mutableListOf<Token>()

    private var inputBytes: ByteArray

    init {
        val str = input.toByteArray().toMutableList()
        var i = 0
        while(i < str.size) {
            val byte = str[i]
            if(byte ==  '\\'.code.toByte() && str[i + 1] == 'n'.code.toByte()) {
                str.removeAt(i+1)
                str[i] = 10
            }
            i++
        }
        inputBytes = str.toByteArray()
    }

    fun run() {

        while (!eof()) {
            when (inputBytes[cursor]) {
                ' '.code.toByte(), '\n'.code.toByte(), '\t'.code.toByte(), '\r'.code.toByte() -> cursor++
                '"'.code.toByte() -> parseStringToken()
                else -> {
                    parseKeywordOrIdentifierToken()
                }
            }
        }
    }

    private fun getChar(): Char = inputBytes[cursor].toInt().toChar()

    private fun parseStringToken() {
        val start = cursor
        cursor++
        while (!eof() && getChar() != '"') cursor++
        cursor++
        val valFromArray = inputBytes.copyOfRange(start + 1, cursor - 1)
        tokens.add(Token(String(valFromArray), TokenType.STRING, StringExtra(valFromArray)))
    }

    private fun parseKeywordOrIdentifierToken() {
        val start = cursor
        while (!eof() && getChar().isLetter()) {
            cursor++
        }
        val value = String(inputBytes.copyOfRange(start, cursor))
        tokens.add(Token(value, TokenType.IDENTIFIER))
    }

    private fun eof(): Boolean = cursor >= inputBytes.size - 1
}