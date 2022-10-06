#include <cstdio>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "tokenizer.hpp"
#include "nava.hpp"
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

    // size_t i = 0;
    // for (auto token : tokenizer.get_tokens())
    // {
    //     printf("[%ld] ", i);
    //     tokenizer.print_token(token);
    //     i++;
    // }

    AST ast(path, tokenizer.get_tokens());
    ast.parse();

    return 0;
}