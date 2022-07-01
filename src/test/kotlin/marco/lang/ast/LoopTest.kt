package marco.lang.ast

import marco.lang.AstBuilder
import marco.lang.Tokenizer
import org.junit.jupiter.api.Test
import kotlin.test.assertEquals

class LoopTest {

    @Test
    fun whileTest() {
        val src = """
            var i = 0;
            while i < 10 {
                print(i);
            }
        """.trimIndent()
        val tokenizer = Tokenizer(src)
        tokenizer.run()
        val astBuilder = AstBuilder(tokenizer.tokens)
        astBuilder.build()
        assertEquals(2, astBuilder.ast.size)
    }
}