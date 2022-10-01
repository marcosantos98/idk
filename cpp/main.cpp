#include <cstdio>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "tokenizer.hpp"
#include "ast.hpp"

std::string read_file_source(const char *file_path)
{

    std::ifstream inFile;
    inFile.open(file_path); // open the input file

    std::stringstream strStream;
    strStream << inFile.rdbuf();       // read the file
    std::string str = strStream.str(); // str holds the content of the file

    return str;
}

int main(int argc, char **args)
{
    (void)argc;
    (void)*args++; // advance program name
    const char *path = *args;

    printf("Loading %s\n", path);

    Tokenizer tokenizer(read_file_source(path));
    tokenizer.run();

    for (auto token : tokenizer.get_tokens())
        tokenizer.print_token(token);

    ASTBuilder ast_builder(tokenizer.get_tokens());
    ast_builder.run();
    ast_builder.print_ast();
    
    return 0;
}