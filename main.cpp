#include "argparse.hpp"
#include "fmt/core.h"
#include "type.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"

#include <iostream>
#include <fstream>
#include "analyser/analyser.h"


inline void printOperation(miniplc0::Instruction &instruction,std::ofstream &);

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

void Binary(std::istream &input, std::ofstream &out) {
    char bytes[8];
    auto tks = _tokenize(input);
    miniplc0::Analyser analyser(tks);
    auto p = analyser.Analyse();
    if (p.second.has_value()) {
        fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
        exit(2);
    }
    auto v = p.first;
    std::vector<std::pair<std::string, int>> Consts = v.getConstList();
    std::vector<miniplc0::Function> funlist = v.getFunctionList();
    std::vector<miniplc0::Instruction> beginCode = v.getBeginCode();
    std::vector<std::vector<miniplc0::Instruction>> program = v.getProgramList();

    const auto writeNBytes = [&](void* addr, int count) {
        assert(0 < count && count <= 8);
        char* p = reinterpret_cast<char*>(addr) + (count-1);
        for (int i = 0; i < count; ++i) {
            bytes[i] = *p--;
        }
        out.write(bytes, count);
    };

    // magic
    out.write("\x43\x30\x3A\x29", 4);
    // version
    out.write("\x00\x00\x00\x01", 4);
    // constants_count
    vm::u2 constants_count = Consts.size();
    writeNBytes(&constants_count, sizeof constants_count);
    // constants
    for(int i=0;i<Consts.size();i++)
    {
        int type,length;
        if(Consts.at(i).second==0)
            type=1;//INT
        else
            type=0;//String
        //type 8
        if(type == 0) {
            out.write("\x00", 1);
            std::string v = Consts.at(i).first;
            vm::u2 len = v.length();
            writeNBytes(&len, sizeof len);
            out.write(v.c_str(), len);
        }else if(type==1){
            out.write("\x01", 1);
            std::stringstream ss;
            ss<<Consts.at(i).first;
            vm::int_t v;
            ss>>v;
            writeNBytes(&v, sizeof v);
        }
    }

    vm::u2 instructions_count = beginCode.size();
    writeNBytes(&instructions_count, sizeof instructions_count);

    for(auto it:beginCode)
        printOperation(it,out);

    vm::u2 functions_count = funlist.size();
    writeNBytes(&functions_count, sizeof functions_count);
    for(int i=0;i<funlist.size();i++)
    {
        vm::u2 v;
        v = funlist[i].nameindex; writeNBytes(&v, sizeof v);
        v = funlist[i].getParaSize(); writeNBytes(&v, sizeof v);
        v = funlist[i].level;     writeNBytes(&v, sizeof v);
        v = program[i+1].size();     writeNBytes(&v, sizeof v);
        //ins_count 16
        for(auto it:program[i+1])
        {
            printOperation(it,out);
        }
    }
}

inline void printOperation(miniplc0::Instruction &instruction,std::ofstream &out) {
    char bytes[8];
    const auto writeNBytes = [&](void* addr, int count) {
        assert(0 < count && count <= 8);
        char* p = reinterpret_cast<char*>(addr) + (count-1);
        for (int i = 0; i < count; ++i) {
            bytes[i] = *p--;
        }
        out.write(bytes, count);
    };
    int ope,num=1;
    switch(instruction.GetOperation())
    {
        case miniplc0::ILOAD:ope=0x10;num=1;break;
        case miniplc0::ISTORE:ope=0x20;num=1;break;
        case miniplc0::IADD:ope=0x30;num=1;break;
        case miniplc0::ISUB:ope=0x34;num=1;break;
        case miniplc0::IMUL:ope=0x38;num=1;break;
        case miniplc0::INEG:ope=0x40;num=1;break;
        case miniplc0::IDIV:ope=0x3c;num=1;break;
        case miniplc0::RET:ope=0x88;num=1;break;
        case miniplc0::IRET:ope=0x89;num=1;break;
        case miniplc0::IPRINT:ope=0xa0;num=1;break;
        case miniplc0::ISCAN:ope=0xb0;num=1;break;
        case miniplc0::POP:ope=0x04;num=1;break;
        case miniplc0::CALL:{
            ope=0x80;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u2 x = static_cast<vm::u2>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::POPN:{
            ope=0x06;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u4 x = static_cast<vm::u4>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::LOADC:{
            ope=0x09;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u2 x = static_cast<vm::u2>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::IPUSH:{
            ope=0x02;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u4 x = static_cast<vm::u4>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::JE:{
            ope=0x71;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u2 x = static_cast<vm::u2>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::JNE:{
            ope=0x72;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u2 x = static_cast<vm::u2>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::JMP:{
            ope=0x70;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u2 x = static_cast<vm::u2>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::JL:{
            ope=0x73;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u2 x = static_cast<vm::u2>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::JLE:{
            ope=0x76;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u2 x = static_cast<vm::u2>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::JG:{
            ope=0x75;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u2 x = static_cast<vm::u2>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::JGE:{
            ope=0x74;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u2 x = static_cast<vm::u2>(instruction.GetX());
            writeNBytes(&x, sizeof x);
            return ;
        }
        case miniplc0::LOADA:{
            ope=0x0a;
            vm::u1 op = static_cast<vm::u1>(ope);
            writeNBytes(&op, sizeof op);
            vm::u2 x = static_cast<vm::u2>(instruction.GetX());
//            std::cout<<x;
            writeNBytes(&x, sizeof x);
            vm::u4 y = static_cast<vm::u4>(instruction.GetY());
            writeNBytes(&y, sizeof y);
            return ;
        }
    }
    if(num==1){
        vm::u1 op = ope;
        writeNBytes(&op, sizeof op);
    }

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
    std::vector<std::pair<std::string, int>> Consts = v.getConstList();
    std::vector<miniplc0::Function> funlist = v.getFunctionList();
    std::vector<miniplc0::Instruction> beginCode = v.getBeginCode();
    std::vector<std::vector<miniplc0::Instruction>> program = v.getProgramList();
    output << fmt::format(".constants:\n");
    for (int i = 0; i < Consts.size(); i++) {
        std::string INT = "I", STR = "S", type, value = Consts[i].first;
        if (Consts[i].second == 0)
            type = INT;
        else{
            type = STR;
            value = '"'+value+'"';
        }
        output << fmt::format("\t{} {} {}\n", i, type, value);
    }
    output << fmt::format("\n");
    output << fmt::format(".start:\n");
    for (int i = 0; i < beginCode.size(); i++) {
        output << fmt::format("\t{} {}\n", i, beginCode[i]);
    }
    output << fmt::format("\n");
    output << fmt::format(".functions:\n");

    for (int i = 0; i < funlist.size(); i++) {
        output << fmt::format("\t{} {} {} {}\n", i, funlist[i].nameindex, funlist[i].getParaSize(), funlist[i].level);
    }

    output << fmt::format("\n");
    for (int i = 1; i < program.size(); i++) {
        auto it = program[i];
        output << fmt::format(".F{}:\n", i - 1);
        for (int j = 0; j < it.size(); j++) {
            output << fmt::format("\t{} {}\n", j, it[j]);
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
        } else
            output = &std::cout;
        Analyse(*input, *output);
    } else if (program["-c"] == true) {
        if (output_file != "-") {
            outf.open(output_file, std::ios::binary | std::ios::out | std::ios::trunc);
            if (!outf) {
                fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
                exit(2);
            }
            output = &outf;
        } else
            output = &std::cout;
        std::ofstream* real_out = dynamic_cast<std::ofstream*>(output);
        Binary(*input, *real_out);
    } else {
        fmt::print(stderr, "You must choose tokenization or syntactic analysis.");
        exit(2);
    }
    return 0;
}

