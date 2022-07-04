package marco.lang

class AstBuilder(private val tokens: List<Token>) {

    val ast = mutableListOf<ExpressionAst>()

    private var currentToken = 0

    fun build() {
        while (currentToken < tokens.size - 1) {
            ast.add(parseExpression()!!)
        }
    }

    private fun advance() {
        if (currentToken + 1 < tokens.size) currentToken++
    }

    private fun getToken(): Token = tokens[currentToken]

    private fun parseIf(): IfExpressionAst {
        assertCurrentToken(TokenType.KEYWORD)
        val condition = parseExpression()!!
        assertCurrentToken(TokenType.LCB, false)
        val body = parseBodyExpression()
        val elseBody = if (getToken().lex == Keywords.ELSE.lex) {
            assertCurrentToken(TokenType.KEYWORD)
            assertCurrentToken(TokenType.LCB, false)
            parseBodyExpression()
        } else BodyExpressionAst(emptyArray())
        return IfExpressionAst(condition, body, elseBody)
    }

    private fun parseVariableDeclaration(): VariableDeclarationAst {
        assertCurrentToken(TokenType.KEYWORD)
        val identifier = assertAndGetIdentifier()
        assertCurrentToken(TokenType.EQ_BIND)
        val expr = parseExpression()
        assertCurrentToken(TokenType.SEMI_COLON)
        return VariableDeclarationAst(identifier, expr!!)
    }

    private fun parsePrimary(): ExpressionAst? {
        return when (getToken().type) {
            TokenType.KEYWORD -> {
                when (getToken().lex) {
                    Keywords.RETURN.lex -> return parseReturn()
                    Keywords.WHILE.lex -> return parseWhileLoop()
                    Keywords.IF.lex -> return parseIf()
                    Keywords.FUNCTION.lex -> return parseFunction()
                    Keywords.VARIABLE.lex -> return parseVariableDeclaration()
                    else -> return null
                }
            }
            TokenType.IDENTIFIER -> parseIdentifier()
            TokenType.LP -> parseParentisExpression()
            TokenType.LCB -> parseBodyExpression()
            TokenType.NUMBER -> {
                val value = getToken().lex.toDouble()
                advance()
                return NumberExpressionAst(value)
            }
            TokenType.STRING -> {
                val value = getToken().lex
                advance()
                return StringExpressionAst(value)
            }
            else -> null
        }
    }

    private fun parseReturn(): ExpressionAst {
        assertCurrentToken(TokenType.KEYWORD)
        val retExpression = parseExpression()!!
        assertCurrentToken(TokenType.SEMI_COLON)
        return ReturnExpressionAst(retExpression)
    }

    private fun parseWhileLoop(): WhileExpressionAst {
        assertCurrentToken(TokenType.KEYWORD)
        val condition = parseExpression()!!
        assertCurrentToken(TokenType.LCB, false)
        val body = parseBodyExpression()
        return WhileExpressionAst(condition, body)
    }

    private fun parseBodyExpression(): BodyExpressionAst {
        assertCurrentToken(TokenType.LCB)
        val statements = mutableListOf<ExpressionAst>()
        while (getToken().type != TokenType.RCB) {
            statements.add(parseExpression()!!)
        }
        assertCurrentToken(TokenType.RCB)
        return BodyExpressionAst(statements.toTypedArray())
    }

    private fun parseBinaryRightExpression(precedence: Int, leftExpression: ExpressionAst): ExpressionAst? {
        var left = leftExpression
        while (true) {
            val binaryOp = BinaryOp.fromTokenType(getToken().type)
            if (binaryOp.precedence < precedence)
                return left
            advance()
            var rightExpression: ExpressionAst? = parsePrimary() ?: return null
            val nextBinaryOp = BinaryOp.fromTokenType(getToken().type)
            if (binaryOp.precedence < nextBinaryOp.precedence)
                rightExpression = parseBinaryRightExpression(binaryOp.precedence + 1, rightExpression!!)
            left = BinaryExpressionAst(left, binaryOp, rightExpression!!)
        }
    }

    private fun parseExpression(): ExpressionAst? {
        val left = parsePrimary() ?: return null
        return parseBinaryRightExpression(0, left)
    }

    private fun parsePrototype(): PrototypeExpressionAst {
        val identifier = assertAndGetIdentifier()
        assertCurrentToken(TokenType.LP)
        val args = mutableListOf<String>()
        while (getToken().type == TokenType.IDENTIFIER || getToken().type == TokenType.COMMA) {
            if (getToken().type == TokenType.COMMA) {
                advance()
                continue
            }
            val argName = assertAndGetIdentifier()
            args.add(argName)
        }
        assertCurrentToken(TokenType.RP)
        return PrototypeExpressionAst(identifier, args.toTypedArray())
    }

    private fun parseFunction(): ExpressionAst {
        assertCurrentToken(TokenType.KEYWORD)
        val prototype = parsePrototype()
        assertCurrentToken(TokenType.LCB, false)
        val expression = parseExpression()
        return FunctionExpressionAst(prototype, expression ?: BodyExpressionAst(emptyArray()))
    }

    private fun parseParentisExpression(): ExpressionAst {
        assertCurrentToken(TokenType.LP)
        val expr = parseExpression() ?: throw IllegalStateException("Something is wrong.")
        assertCurrentToken(TokenType.RP)
        return expr
    }

    private fun parseIdentifier(): ExpressionAst {
        val identifier = assertAndGetIdentifier()
        if (getToken().type != TokenType.LP)
            return VariableExpressionAst(identifier)
        advance()
        val args = mutableListOf<ExpressionAst>()
        while (true) {
            val expr = parseExpression()
            if (expr != null) args.add(expr)
            else throw IllegalStateException("Something is wrong.")
            if (getToken().type == TokenType.RP) break
            assertCurrentToken(TokenType.COMMA)
        }
        assertCurrentToken(TokenType.RP)
        // FIXME: 01/07/22 This should change when calling members functions.
        assertCurrentToken(TokenType.SEMI_COLON)
        return CallExpressionAst(identifier, args.toTypedArray())
    }

    private fun assertAndGetIdentifier(): String {
        if (getToken().type == TokenType.IDENTIFIER) {
            val identifier = getToken().lex
            advance()
            return identifier
        } else throw IllegalStateException("Expected IDENTIFIER got ${getToken()}")
    }

    private fun assertCurrentToken(token: TokenType, advance: Boolean = true) {
        if (getToken().type != token) throw IllegalStateException("Expected $token got ${getToken()}")
        else if (getToken().type == token && advance) advance()
    }
}