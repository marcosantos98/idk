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

    @Test
    fun varDeclarationInsideFunc() {
        val src = """
            fn main(args) {
                var x = 69;
            }
        """.trimIndent()
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            FunctionExpressionAst(
                PrototypeExpressionAst("main", arrayOf("args")),
                BodyExpressionAst(
                    arrayOf(
                        VariableDeclarationAst("x", NumberExpressionAst(69.0))
                    )
                )
            ),
            astBuilder.ast[0]
        )
    }

    @Test
    fun controlFlowInsideFunc() {
        val src = """
            fn main(args) {
                if a < 10 {
                    print(a);
                } else {
                    print(b);
                }
            }
        """.trimIndent()
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            FunctionExpressionAst(
                PrototypeExpressionAst("main", arrayOf("args")),
                BodyExpressionAst(
                    arrayOf(
                        IfExpressionAst(
                            BinaryExpressionAst(
                                VariableExpressionAst("a"),
                                BinaryOp.LT,
                                NumberExpressionAst(10.0)
                            ),
                            BodyExpressionAst(arrayOf(CallExpressionAst("print", arrayOf(VariableExpressionAst("a"))))),
                            BodyExpressionAst(arrayOf(CallExpressionAst("print", arrayOf(VariableExpressionAst("b")))))
                        )
                    )
                )
            ),
            astBuilder.ast[0]
        )
    }

    @Test
    fun whileLoopInsideFunc() {
        val src = """
            fn main(args) {
                while x < 10 {
                    print(x);
                }
            }
        """.trimIndent()
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            FunctionExpressionAst(
                PrototypeExpressionAst("main", arrayOf("args")),
                BodyExpressionAst(arrayOf(
                    WhileExpressionAst(
                        BinaryExpressionAst(
                            VariableExpressionAst("x"),
                            BinaryOp.LT,
                            NumberExpressionAst(10.0)
                        ),
                        BodyExpressionAst(arrayOf(CallExpressionAst("print", arrayOf(VariableExpressionAst("x"))))),
                    )
                ))
            ),
            astBuilder.ast[0]
        )
    }
}