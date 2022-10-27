#include "global.hpp"

#include <cstdio>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <list>
#include <time.h>

#include "debug.hpp"
#include "checker.hpp"
#include "x86_64_nasm_linux.hpp"
#include "argparser.hpp"
#include "log.hpp"
#include "project_parser.hpp"

Log logger("Project");

String read_file_source(const char *file_path)
{

    std::ifstream inFile;
    inFile.open(file_path); // open the input file

    if (inFile.fail())
    {
        printf("Files doesn't exist! %s\n", file_path);
        exit(1);
    }

    std::stringstream strStream;
    strStream << inFile.rdbuf();       // read the file
    std::string str = strStream.str(); // str holds the content of the file

    return str;
}

Vec<String> glob_with_ext(const std::string &path, const char *ext)
{
    Vec<String> paths;
    for (const auto &p : std::filesystem::recursive_directory_iterator(path))
        if (!std::filesystem::is_directory(p))
            if (p.path().filename().extension() == ext || strcmp(ext, "*") == 0)
                paths.emplace_back(p.path());
    return paths;
}

int main(int argc, char **argv)
{
    argparse::ArgumentParser program("nava");
    program.add_argument("-s")
        .nargs(1)
        .help("Compile single file.");
    program.add_argument("-p")
        .help("Compile project in path")
        .nargs(1);
    program.add_argument("-i")
        .help("List of files to include when compiling a single file.")
        .nargs(argparse::nargs_pattern::any);
    program.add_argument("-dp")
        .default_value(false)
        .implicit_value(true)
        .help("Analyse the project and print the current project contents. Exits after.");
    program.add_argument("-r")
        .help("Set root path")
        .nargs(1);

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    if (argc == 1)
    {
        std::cerr << program;
        std::exit(1);
    }

    Project project;
    if (program.present<String>("-r"))
    {
        project.root_path = program.get<String>("-r");
    }

    auto parse_start = std::chrono::high_resolution_clock::now();
 
    ProjectParser proj_parser(&project);

    if (program.present("-s"))
    {
        if (!program.present<String>("-r"))
            project.root_path = program.get<String>("-s");
        project.single_file = true;
        proj_parser.parse(program.get<String>("-s"), read_file_source(program.get<String>("-s").c_str()));

        if (program.present("-i"))
        {
            auto files = program.get<Vec<String>>("-i");
            for (auto s : files)
            {
                proj_parser.parse(s, read_file_source(s.c_str()));
            }
        }
    }
    else if (program.present<String>("-p"))
    {
        if (!program.present<String>("-r"))
            project.root_path = program.get<String>("-p");
        for (auto globpath : glob_with_ext(program.get<String>("-p"), ".nava"))
        {
            proj_parser.parse(globpath, read_file_source(globpath.c_str()));
        }
    }

    auto parse_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - parse_start);

    if (program.get<bool>("-dp"))
    {
        print_project(project);
        exit(0);
    }

    auto checker_start = std::chrono::high_resolution_clock::now();

    Checker checker(&project);
    checker.start_checking();

    auto checker_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - checker_start);

    auto gen_start = std::chrono::high_resolution_clock::now();

    CodeGenerator gen(&project);
    gen.generate();

    auto gen_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - gen_start);

    printf("\n");
    logger.log_tdbg("Execution time:\n");
    logger.log_info("Parser:                     %ldms\n", parse_duration);
    logger.log_info("Checker:                    %ldms\n", checker_duration);
    logger.log_info("Generator [GENASM+NASM+LD]: %ldms\n", gen_duration);
    printf("\n");

}