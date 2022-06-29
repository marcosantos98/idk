package marco.lang

interface ExpressionAst

data class NumberExpressionAst(val number: Double) : ExpressionAst
data class StringExpressionAst(val string: String) : ExpressionAst
data class VariableExpressionAst(val refName: String) : ExpressionAst
data class VariableDeclarionAst(val refName: String, val value: ExpressionAst) : ExpressionAst
data class BinaryExpressionAst(val left: ExpressionAst, val op: BinaryOp, val right: ExpressionAst) : ExpressionAst
data class FunctionExpressionAst(val protoAst: PrototypeExpressionAst, val body: ExpressionAst) : ExpressionAst
data class ReturnExpressionAst(val ast: ExpressionAst) : ExpressionAst

data class BodyExpressionAst(val stats: Array<ExpressionAst>) : ExpressionAst {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as BodyExpressionAst

        if (!stats.contentEquals(other.stats)) return false

        return true
    }

    override fun hashCode(): Int {
        return stats.contentHashCode()
    }
}

data class CallExpressionAst(val name: String, val args: Array<ExpressionAst>) : ExpressionAst {

    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as CallExpressionAst

        if (name != other.name) return false
        if (!args.contentEquals(other.args)) return false

        return true
    }

    override fun hashCode(): Int {
        var result = name.hashCode()
        result = 31 * result + args.contentHashCode()
        return result
    }
}

data class PrototypeExpressionAst(val name: String, val args: Array<String>) : ExpressionAst {

    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as PrototypeExpressionAst

        if (name != other.name) return false
        if (!args.contentEquals(other.args)) return false

        return true
    }

    override fun hashCode(): Int {
        var result = name.hashCode()
        result = 31 * result + args.contentHashCode()
        return result
    }
}

enum class BinaryOp(val precedence: Int) {
    NEQ(10), EQ(10),
    LT(10), GT(10),
    LTEQ(10), GTEQ(10),
    PLUS(20), MINUS(20),
    MOD(30), MUL(30), DIV(30),
    INVALID(-1);

    companion object {
        fun fromTokenType(tokenType: TokenType): BinaryOp {
            return when (tokenType) {
                TokenType.PLUS -> PLUS
                TokenType.MINUS -> MINUS
                TokenType.MUL -> MUL
                TokenType.DIV -> DIV
                TokenType.MOD -> MOD
                TokenType.EQ -> EQ
                TokenType.NEQ -> NEQ
                TokenType.LT -> LT
                TokenType.GT -> GT
                TokenType.LTEQ -> LTEQ
                TokenType.GTEQ -> GTEQ
                else -> INVALID
            }
        }
    }
}