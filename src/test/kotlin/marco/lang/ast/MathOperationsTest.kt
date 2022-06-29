package marco.lang.ast

import marco.lang.*
import org.junit.jupiter.api.Test
import kotlin.test.assertEquals

class MathOperationsTest {

    @Test
    fun plus() {
        val src = "34 + 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.PLUS, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }

    @Test
    fun minus() {
        val src = "34 - 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.MINUS, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }

    @Test
    fun multiply() {
        val src = "34 * 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.MUL, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }

    @Test
    fun divide() {
        val src = "34 / 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.DIV, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }

    @Test
    fun mod() {
        val src = "34 % 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.MOD, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }

    @Test
    fun multiPrecedenceOperation() {
        val src = "4 + 35 * 2 - 35 / 2"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(
                BinaryExpressionAst(
                    NumberExpressionAst(4.0),
                    BinaryOp.PLUS,
                    BinaryExpressionAst(
                        NumberExpressionAst(35.0),
                        BinaryOp.MUL,
                        NumberExpressionAst(2.0)
                    )
                ),
                BinaryOp.MINUS,
                BinaryExpressionAst(
                    NumberExpressionAst(35.0),
                    BinaryOp.DIV,
                    NumberExpressionAst(2.0)
                )
            ),
            astBuilder.ast[0]
        )
    }

    @Test
    fun equal() {
        val src = "34 == 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.EQ, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }

    @Test
    fun notEqual() {
        val src = "34 != 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.NEQ, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }

    @Test
    fun lessThan() {
        val src = "34 < 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.LT, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }

    @Test
    fun greaterThan() {
        val src = "34 > 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.GT, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }

    @Test
    fun lessOrEqualTo() {
        val src = "34 <= 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.LTEQ, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }

    @Test
    fun greaterOrEqualTo() {
        val src = "34 >= 35"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            BinaryExpressionAst(NumberExpressionAst(34.0), BinaryOp.GTEQ, NumberExpressionAst(35.0)),
            astBuilder.ast[0]
        )
    }
}