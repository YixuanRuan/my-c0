#include "binary/type.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"
#include "analyser/analyser.h"
#include <sstream>
#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <fstream>

inline void catOp(miniplc0::Instruction &instruction,std::ofstream &out);

void Binary(miniplc0::Program&, std::ofstream &out);