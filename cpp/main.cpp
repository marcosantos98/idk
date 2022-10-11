#include <cstdio>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "tokenizer.hpp"
#include "nava.hpp"
#include "ast.hpp"
#include "codegen.hpp"

std::string read_file_source(const char *file_path)
{

    std::ifstream inFile;
    inFile.open(file_path); // open the input file

    if(inFile.fail())
    {
        printf("Files doesn't exist! %s\n", file_path);
        exit(1);
    }

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

    //printf("Loading %s\n", path);

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
    
    CodeGenerator code_gen(ast.get_root_class());
    String s = code_gen.generate();

    std::string path_str = path;
    //fixme 22/10/04: a bit of hackery
    std::string final_path = path_str.substr(0, path_str.length() - 4).append("asm");
    std::ofstream out(final_path);
    out << s;
    out.close();

    return 0;
}