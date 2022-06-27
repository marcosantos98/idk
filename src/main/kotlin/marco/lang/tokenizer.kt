package marco.lang

import marco.Token
import marco.TokenType

private var cursor: Int = 0
private var sourceCode: String = ""
private var tokens: MutableList<Token> = mutableListOf()

fun tokenize(sourceCodeIn: String) {
    sourceCode = sourceCodeIn
    while (cursor < sourceCodeIn.length) {
        when (sourceCodeIn[cursor]) {
            ' ', '\n', '\t', '\r' -> cursor++
            '"' -> parseString()
            else -> {
                parseKeywordOrIdentifier()
            }
        }
    }
}

fun getTokens(): MutableList<Token> = tokens


private fun parseString() {
    val start = cursor
    cursor++
    while (sourceCode[cursor] != '"') cursor++
    cursor++
    tokens.add(Token(sourceCode.substring(start, cursor), TokenType.STRING))
}

private fun parseKeywordOrIdentifier() {
    val start = cursor
    while ((cursor + 1) <= sourceCode.length && sourceCode[cursor].isLetter()) {
        cursor++
    }
    val value = sourceCode.substring(start, cursor)
    tokens.add(Token(value, TokenType.IDENTIFIER))
}
