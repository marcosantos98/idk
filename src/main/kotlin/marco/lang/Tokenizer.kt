package marco.lang

class Tokenizer(input: String) {

    private var cursor = 0

    val tokens = mutableListOf<Token>()

    private var inputBytes: ByteArray

    init {
        val str = input.toByteArray().toMutableList()
        var i = 0
        while (i < str.size) {
            val byte = str[i]
            if (byte == '\\'.code.toByte() && str[i + 1] == 'n'.code.toByte()) {
                str.removeAt(i + 1)
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
                '!'.code.toByte() -> {
                    if(inputBytes[cursor+1] == '='.code.toByte()) {
                        addToken(Token("!=", TokenType.NEQ))
                        cursor++
                    }
                }
                '='.code.toByte() -> {
                    if(inputBytes[cursor+1] == '='.code.toByte()) {
                        addToken(Token("==", TokenType.EQ))
                        cursor++
                    } else addToken(Token("=", TokenType.EQ_BIND))
                }
                '<'.code.toByte() -> {
                    if(inputBytes[cursor+1] == '='.code.toByte()) {
                        addToken(Token("<=", TokenType.LTEQ))
                        cursor++
                    } else addToken(Token("<", TokenType.LT))
                }
                '>'.code.toByte() -> {
                    if(inputBytes[cursor+1] == '='.code.toByte()) {
                        addToken(Token(">=", TokenType.GTEQ))
                        cursor++
                    } else addToken(Token(">", TokenType.GT))
                }
                '+'.code.toByte() -> addToken(Token("+", TokenType.PLUS))
                '-'.code.toByte() -> addToken(Token("-", TokenType.MINUS))
                '*'.code.toByte() -> addToken(Token("*", TokenType.MUL))
                '/'.code.toByte() -> addToken(Token("/", TokenType.DIV))
                '%'.code.toByte() -> addToken(Token("%", TokenType.MOD))
                '('.code.toByte() -> addToken(Token("(", TokenType.LP))
                ')'.code.toByte() -> addToken(Token(")", TokenType.RP))
                '{'.code.toByte() -> addToken(Token("{", TokenType.LCB))
                '}'.code.toByte() -> addToken(Token("}", TokenType.RCB))
                ','.code.toByte() -> addToken(Token(",", TokenType.COMMA))
                else -> {
                    if (getChar().isDigit()) parseNumberToken()
                    else parseKeywordOrIdentifierToken()
                }
            }
        }
        addToken(Token("EOF", TokenType.EOF))
    }

    private fun addToken(token: Token, advance: Boolean = true) {
        tokens.add(token)
        if (advance) cursor++
    }

    private fun getChar(): Char = inputBytes[cursor].toInt().toChar()

    private fun parseNumberToken() {
        val start = cursor
        while (!eof() && getChar().isDigit()) cursor++
        val valFromArray = inputBytes.copyOfRange(start, cursor)
        tokens.add(Token(String(valFromArray), TokenType.NUMBER))
    }

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
        if (keywords.contains(value)) tokens.add(Token(value, TokenType.KEYWORD))
        else tokens.add(Token(value, TokenType.IDENTIFIER))
    }

    private fun eof(): Boolean = cursor >= inputBytes.size
}