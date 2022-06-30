package marco.lang.ast

import marco.lang.*
import org.junit.jupiter.api.Test
import kotlin.test.assertEquals

class BasicExpressionTest {
    @Test
    fun number() {
        val src = "69"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(NumberExpressionAst(69.0), astBuilder.ast[0])
    }

    @Test
    fun string() {
        val src = "\"Hello, World\""
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(StringExpressionAst("Hello, World"), astBuilder.ast[0])
    }

    @Test
    fun identifier() {
        val src = "lang"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(VariableExpressionAst("lang"), astBuilder.ast[0])
    }

    @Test
    fun call() {
        val src = "add(34, 35)"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            CallExpressionAst("add", arrayOf(NumberExpressionAst(34.0), NumberExpressionAst(35.0))),
            astBuilder.ast[0]
        )
    }

    @Test
    fun function() {
        val src = "fn add(a, b)"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            FunctionExpressionAst(PrototypeExpressionAst("add", arrayOf("a", "b")), BodyExpressionAst(emptyArray())),
            astBuilder.ast[0]
        )
    }

    @Test
    fun binary() {
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
    fun variableDeclaration() {
        val src = "var x = 69"
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(1, astBuilder.ast.size)
        assertEquals(
            VariableDeclarationAst("x", NumberExpressionAst(69.0)),
            astBuilder.ast[0]
        )
    }
}