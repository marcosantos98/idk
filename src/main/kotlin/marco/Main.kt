package marco

import marco.lang.getTokens
import marco.lang.tokenize
import java.nio.file.Files
import java.nio.file.Paths

fun main() {
    val fileName = "tests/print"
    val sourceCode = Files.readString(Paths.get("$fileName.idk"))

    tokenize(sourceCode)

    val asm = StringBuilder()

    var strCount = 0

    val dataSection = StringBuilder()
    dataSection.append("section .data\n")

    val textSection = StringBuilder()
    textSection.append("section .text\n")
    textSection.append("\tglobal _start\n")
    textSection.append("_start:\n")

    for (token in getTokens()) {
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

    textSection.append("\tmov rax, 60\n")
    textSection.append("\tmov rdi, 0\n")
    textSection.append("\tsyscall\n")

    asm.append(textSection).append(dataSection)

    println()
    println(asm)

    Files.write(Paths.get("$fileName.asm"), asm.toString().toByteArray())
}