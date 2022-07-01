package marco

import kotlinx.cli.ArgParser
import kotlinx.cli.ArgType
import kotlinx.cli.default
import marco.lang.AstBuilder
import marco.lang.Tokenizer
import java.nio.file.Files
import java.nio.file.Paths

fun main(args: Array<String>) {

    val argParser = ArgParser("idk")
    val input by argParser.argument(ArgType.String, description = "Input source file")
    //val output by argParser.option(ArgType.String, shortName = "o", description = "Output asm file")
    //val printAsm by argParser.option(ArgType.Boolean, shortName = "p", description = "Print generated asm").default(false)
    val debugTokens by argParser.option(ArgType.Boolean, shortName = "pt", description = "Only prints the tokens")
        .default(false)
    val debugAst by argParser.option(ArgType.Boolean, shortName = "pa", description = "Only prints the tokens")
        .default(false)
    argParser.parse(args)

    val sourceCode = Files.readString(Paths.get(input))

    val tokenizer = Tokenizer(sourceCode)
    tokenizer.run()


    if (debugTokens) {
        tokenizer.tokens.forEach { println(it) }
        return
    }

    val astBuilder = AstBuilder(tokenizer.tokens)
    astBuilder.build()

    if(debugAst) {
        astBuilder.ast.forEach { println(it) }
        return
    }

    //val asmGenerator = AsmGenerator(astBuilder.ast)
    //val generatedAsm = asmGenerator.generate()
    //if(printAsm) println(generatedAsm)

    //Files.write(Paths.get(output ?: "${input.split(".")[0]}.asm"), generatedAsm.toByteArray())
}