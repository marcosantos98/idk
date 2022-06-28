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

        for (i in tokens.indices) {
            val token = tokens[i]
            when (token.type) {
                TokenType.IDENTIFIER -> {
                    if (token.lex == "println") {
                        textSection.append("\tmov rax, 1\n")
                        textSection.append("\tmov rdi, 1\n")
                        textSection.append("\tmov rsi, str_").append(strCount).append("\n")
                        textSection.append("\tmov rdx, str_len_").append(strCount).append("\n")
                        textSection.append("\tsyscall\n")
                    } else if(token.lex == "file") {
                        // FIXME: 28/06/22 Currently the flags and umode are hardcoded!
                        // FIXME: 28/06/22 abstract write syscall and make a system to store return values like the file descriptor
                        if(tokens[i + 1].type == TokenType.STRING && tokens[i + 2].type == TokenType.STRING) {
                            textSection.append("\tmov rax, 2\n")
                            textSection.append("\tmov rdi, str_").append(strCount).append("\n")
                            textSection.append("\tmov rsi, 0102o\n")
                            textSection.append("\tmov rdx, 0666o\n")
                            textSection.append("\tsyscall\n")
                            textSection.append("\tmov rdi, rax\n")
                            textSection.append("\tmov rax, 1\n")
                            textSection.append("\tmov rsi, str_").append(strCount + 1).append("\n")
                            textSection.append("\tmov rdx, str_len_").append(strCount + 1).append("\n")
                            textSection.append("\tsyscall\n")
                            textSection.append("\tmov rax, 3\n")
                            textSection.append("\tsyscall\n")
                        }
                    }
                }
                TokenType.STRING -> {
                    dataSection.append("\tstr_").append(strCount).append(" db ")
                    (token.extra as StringExtra).value.forEach { dataSection.append(it).append(", ") }
                    dataSection.append("0\n")
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