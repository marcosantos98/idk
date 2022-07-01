package marco.lang.ast

import marco.lang.*
import org.junit.jupiter.api.Test
import kotlin.test.assertEquals

class ControlFlowTest {
    @Test
    fun ifTest() {
        val src = """
            if x < 10 { 
                print("Hello");
            }
            """.trimIndent()
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            IfExpressionAst(
                BinaryExpressionAst(
                    VariableExpressionAst("x"),
                    BinaryOp.LT,
                    NumberExpressionAst(10.0)
                ),
                BodyExpressionAst(
                    arrayOf(
                        CallExpressionAst(
                            "print",
                            arrayOf(StringExpressionAst("Hello"))
                        )
                    )
                ),
                BodyExpressionAst(emptyArray())
            ),
            astBuilder.ast[0]
        )
    }

    @Test
    fun ifElseTest() {
        val src = """
            if x < 10 { 
                print("Hello");
            } else {
                print("World");
            }
            """.trimIndent()
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            IfExpressionAst(
                BinaryExpressionAst(
                    VariableExpressionAst("x"),
                    BinaryOp.LT,
                    NumberExpressionAst(10.0)
                ),
                BodyExpressionAst(
                    arrayOf(
                        CallExpressionAst(
                            "print",
                            arrayOf(StringExpressionAst("Hello"))
                        )
                    )
                ),
                BodyExpressionAst(
                    arrayOf(
                        CallExpressionAst(
                            "print",
                            arrayOf(StringExpressionAst("World"))
                        )
                    )
                )
            ),
            astBuilder.ast[0]
        )
    }

    @Test
    fun insideBlockIf() {
        val src = """
            fn main(args) {
                if x < 10 { 
                    print("Hello");
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
                                VariableExpressionAst("x"),
                                BinaryOp.LT,
                                NumberExpressionAst(10.0)
                            ),
                            BodyExpressionAst(
                                arrayOf(
                                    CallExpressionAst(
                                        "print",
                                        arrayOf(StringExpressionAst("Hello"))
                                    )
                                )
                            ),
                            BodyExpressionAst(emptyArray())
                        )
                    )
                )
            ),
            astBuilder.ast[0]
        )
    }
}