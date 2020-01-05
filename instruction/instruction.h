#pragma once

#include <cstdint>
#include <utility>

namespace miniplc0 {

	enum Operation {
        ILL = 0,
        LOADA,
        LOADC,
        IPUSH,
        ILOAD,
        ISTORE,
        IADD,
        ISUB,
        IMUL,
        INEG,
        IDIV,
        ICMP,
        CALL,
        RET,
        IRET,
        IPRINT,
        CPRINT,
        ISCAN,
        POPN,
        POP,
        JE,
        JNE,
        JMP,
        JG,
        JL,
        JGE,
        JLE
	};
	
	class Instruction final {
	private:
		using int32_t = std::int32_t;
	public:
		friend void swap(Instruction& lhs, Instruction& rhs);
    public:
        Instruction(Operation opr, int32_t x) : _opr(opr), _x(x) {}
        Instruction(Operation opr, int32_t x,int32_t y) : _opr(opr), _x(x),_y(y) {}
        Instruction() : Instruction(Operation::ILL, 0){}
        Instruction(const Instruction& i) { _opr = i._opr; _x = i._x;_y=i._y ;}
        Instruction(Instruction&& i) :Instruction() { swap(*this, i); }
        Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
        bool operator==(const Instruction& i) const { return _opr == i._opr && _x == i._x&& _y == i._y; }

        Operation GetOperation() const { return _opr; }
        int32_t GetX() const { return _x; }
        int32_t GetY() const { return _y; }
        void SetX(int32_t x){  _x=x;}
    private:
        Operation _opr;
        int32_t _x;
        int32_t _y;
    };

	inline void swap(Instruction& lhs, Instruction& rhs) {
		using std::swap;
		swap(lhs._opr, rhs._opr);
		swap(lhs._x, rhs._x);
        swap(lhs._y, rhs._y);
	}
}