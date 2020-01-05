#include "argparse.hpp"
#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"
#include "binary/binary.h"
#include <iostream>
#include <fstream>

std::vector<miniplc0::Token> _tokenize(std::istream &input) {
    miniplc0::Tokenizer tkz(input);
    auto p = tkz.AllTokens();
    if (p.second.has_value()) {
        fmt::print(stderr, "Tokenization error: {}\n", p.second.value());
        exit(2);
    }
    return p.first;
}

void Tokenize(std::istream &input, std::ostream &output) {
    auto v = _tokenize(input);
    for (auto &it : v)
        output << fmt::format("{}\n", it);
    return;
}

void Analyse(std::istream &input, std::ostream &output) {
    auto tks = _tokenize(input);

    miniplc0::Analyser analyser(tks);
    auto p = analyser.Analyse();
    if (p.second.has_value()) {
        fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
        exit(2);
    }
    auto v = p.first;
    std::vector<std::pair<std::string, int>> cons = v.cons();
    output << fmt::format(".constants:\n");
    for (int i = 0; i < cons.size(); i++) {
        std::string type, value = cons[i].first;
        if (cons[i].second == 0) type = "I";
        else{ type = "S"; value = '"'+value+'"';}
        output << fmt::format("\t{} {} {}\n", i, type, value);
    }
    // 全局量加载
    std::vector<miniplc0::Instruction> start = v.start();
    output << fmt::format("\n.start:\n");
    for (int i = 0; i < start.size(); i++) {
        output << fmt::format("\t{} {}\n", i, start[i]);
    }
    // 函数表
    std::vector<miniplc0::Function> funlist = v.funcs();
    output << fmt::format("\n.functions:\n");
    for (int i = 0; i < funlist.size(); i++) {
        output << fmt::format("\t{} {} {} {}\n", i, funlist[i].nameindex, funlist[i].getParaSize(), funlist[i].level);
    }
    // 函数代码
    std::vector<std::vector<miniplc0::Instruction>> program = v.codes();
    output << fmt::format("\n");
    for (int i = 1; i < program.size(); i++) {
        auto  p = program[i];
        output << fmt::format(".F{}:\n", i - 1);
        for (int j = 0; j < p.size(); j++) {
            output << fmt::format("\t{} {}\n", j, p[j]);
        }
    }

    return;
}

int main(int argc, char **argv) {
    argparse::ArgumentParser program("c0");
    program.add_argument("input")
            .help("speicify the file to be compiled.");
    program.add_argument("-t")
            .default_value(false)
            .implicit_value(true)
            .help("perform tokenization for the input file.");
    program.add_argument("-s")
            .default_value(false)
            .implicit_value(true)
            .help("generate assembly code.");
    program.add_argument("-c")
            .default_value(false)
            .implicit_value(true)
            .help("generate binary file.");
    program.add_argument("-o", "--output")
            .required()
            .default_value(std::string("-"))
            .help("specify the output file.");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err) {
        fmt::print(stderr, "{}\n\n", err.what());
        program.print_help();
        exit(2);
    }

    auto input_file = program.get<std::string>("input");
    auto output_file = program.get<std::string>("--output");
    std::istream *input;
    std::ostream *output;
    std::ifstream inf;
    std::ofstream outf;
    if (input_file != "-") {
        inf.open(input_file, std::ios::in);
        if (!inf) {
            fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
            exit(2);
        }
        input = &inf;
    } else
        input = &std::cin;
    if (program["-t"] == true && program["-s"] == true) {
        fmt::print(stderr, "You can only perform tokenization or syntactic analysis at one time.");
        exit(2);
    }
    if (program["-t"] == true) {
        if (output_file != "-") {
            outf.open(output_file, std::ios::out | std::ios::trunc);
            if (!outf) {
                fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
                exit(2);
            }
            output = &outf;
        } else
            output = &std::cout;
        Tokenize(*input, *output);
    } else if (program["-s"] == true) {
        if (output_file != "-") {
            outf.open(output_file, std::ios::out | std::ios::trunc);
            if (!outf) {
                fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
                exit(2);
            }
            output = &outf;
        } else{
            output_file = input_file+".s";
            outf.open(output_file, std::ios::binary | std::ios::out | std::ios::trunc);
            if (!outf) {
                fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
                exit(2);
            }
        }
        output = &outf;
        Analyse(*input, *output);
    } else if (program["-c"] == true) {
        if (output_file != "-") {
            outf.open(output_file, std::ios::binary | std::ios::out | std::ios::trunc);
            if (!outf) {
                fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
                exit(2);
            }
            output = &outf;
        } else{
            output_file = input_file+".out";
            outf.open(output_file, std::ios::binary | std::ios::out | std::ios::trunc);
            if (!outf) {
                fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
                exit(2);
            }
            output = &outf;
        }
        std::ofstream* real_out = dynamic_cast<std::ofstream*>(output);
        auto tks = _tokenize(*input);
        miniplc0::Analyser analyser(tks);
        auto p = analyser.Analyse();
        if (p.second.has_value()) {
            fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
            exit(2);
        }
        miniplc0::Program v = p.first;
        Binary(v, *real_out);
    } else {
        fmt::print(stderr, "You must choose tokenization or syntactic analysis.");
        exit(2);
    }
    return 0;
}
