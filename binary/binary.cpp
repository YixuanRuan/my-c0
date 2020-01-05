#include "binary.h"
inline void catOp(miniplc0::Instruction &instruction,std::ofstream &out) {
    char bytes[32];
    const auto writeNBytes = [&](void* addr, int count) {
        char* p = reinterpret_cast<char*>(addr) + (count-1);
        for (int i = 0; i < count; ++i) {
            bytes[i] = *p--;
        }
        out.write(bytes, count);
    };
    int ope;
    switch(instruction.GetOperation())
    {
        case miniplc0::IDIV:{
            ope=0x3c;break;
        }
        case miniplc0::RET:{
            ope=0x88;break;
        }
        case miniplc0::IRET:{
            ope=0x89;break;
        }
        case miniplc0::IPRINT:{
            ope=0xa0;break;
        }
        case miniplc0::ILOAD:{
            ope=0x10;break;
        }
        case miniplc0::ISTORE:{
            ope=0x20;break;
        }
        case miniplc0::IADD:{
            ope=0x30;break;
        }
        case miniplc0::ISUB:{
            ope=0x34;break;
        }
        case miniplc0::IMUL:{
            ope=0x38;break;
        }
        case miniplc0::INEG:{
            ope=0x40;break;
        }
        case miniplc0::ISCAN:{
            ope=0xb0;break;
        }
        case miniplc0::POP:{
            ope=0x04;break;
        }
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
            writeNBytes(&x, sizeof x);
            vm::u4 y = static_cast<vm::u4>(instruction.GetY());
            y = y & 0x00ff;
            writeNBytes(&y, sizeof y);
            return ;
        }
    }
    vm::u1 op = ope;
    writeNBytes(&op, sizeof op);
}

void Binary(miniplc0::Program &v, std::ofstream &out) {
    char bytes[32];
    const auto writeNBytes = [&](void* addr, int count) {
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
    std::vector<std::pair<std::string, int>> Consts = v.cons();
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

    std::vector<miniplc0::Instruction> beginCode = v.start();
    vm::u2 instructions_count = beginCode.size();
    writeNBytes(&instructions_count, sizeof instructions_count);

    for(auto it:beginCode)
        catOp(it,out);

    std::vector<miniplc0::Function> funlist = v.funcs();
    vm::u2 functions_count = funlist.size();
    writeNBytes(&functions_count, sizeof functions_count);

    std::vector<std::vector<miniplc0::Instruction>> program = v.codes();
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
            catOp(it,out);
        }
    }
}