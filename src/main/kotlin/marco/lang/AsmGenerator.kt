package marco.lang

class AsmGenerator(private val tokens: List<Token>) {

    private var strCount = 0

    fun generate(): String {
        val asm = StringBuilder()

        val textSection = StringBuilder("section .text\n")
        val dataSection = StringBuilder("section .data\n")

        textSection.append("\tglobal _start\n")
        textSection.append("halt:\n")
            .append("\tmov rax, 60\n")
            .append("\tmov rdi, 0\n")
            .append("\tsyscall\n")
        textSection.append("_start:\n")

        for (token in tokens) {
            when (token.type) {
                TokenType.IDENTIFIER -> {
                    if (token.lex == "println") {
                        textSection.append("\tmov rax, 1\n")
                        textSection.append("\tmov rdi, 1\n")
                        textSection.append("\tmov rsi, str_").append(strCount).append("\n")
                        textSection.append("\tmov rdx, str_len_").append(strCount).append("\n")
                        textSection.append("\tsyscall\n")
                    }
                }
                TokenType.STRING -> {
                    dataSection.append("\tstr_").append(strCount).append(": db ").append(token.lex).append(", 10\n")
                    dataSection.append("\tstr_len_").append(strCount).append(" equ $-str_").append(strCount).append("\n")
                    strCount++
                }
            }
        }

        textSection.append("\tcall halt\n")

        asm.append(textSection)
        asm.append(dataSection)

        return asm.toString()
    }
}