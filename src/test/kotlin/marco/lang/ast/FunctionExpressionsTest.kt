package marco.lang.ast

import marco.lang.*
import org.junit.jupiter.api.Test
import org.junit.jupiter.api.assertThrows
import kotlin.test.assertEquals

class FunctionExpressionsTest {

    @Test
    fun emptyBody() {
        val src = """
            fn add(a, b) {}
        """.trimIndent()
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            FunctionExpressionAst(
                PrototypeExpressionAst("add", arrayOf("a", "b")),
                BodyExpressionAst(emptyArray())
            ),
            astBuilder.ast[0]
        )
    }

    @Test
    fun withBody() {
        val src = """
            fn add(a, b) {
                a + b
            }
        """.trimIndent()
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            FunctionExpressionAst(
                PrototypeExpressionAst("add", arrayOf("a", "b")),
                BodyExpressionAst(
                    arrayOf(
                        BinaryExpressionAst(
                            VariableExpressionAst("a"),
                            BinaryOp.PLUS,
                            VariableExpressionAst("b")
                        )
                    )
                )
            ),
            astBuilder.ast[0]
        )
    }

    @Test
    fun withReturn() {
        val src = """
            fn favoriteNumber() {
                ret 69;
            }
        """.trimIndent()
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            FunctionExpressionAst(
                PrototypeExpressionAst("favoriteNumber", emptyArray()),
                BodyExpressionAst(
                    arrayOf(
                        ReturnExpressionAst(NumberExpressionAst(69.0))
                    )
                )
            ),
            astBuilder.ast[0]
        )
    }

    @Test
    fun throwWhenNoBody() {
        val src = "fn bla()"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        assertThrows<IllegalStateException> { astBuilder.build() }
    }
}