#include "analyser.h"

#include <climits>
#include <sstream>

namespace miniplc0 {
    std::vector<miniplc0::Instruction> _instructions;
    std::pair<Program, std::optional<CompilationError>> Analyser::Analyse() {
        _funcRetType=NULL_TOKEN;
        auto err = analyseProgram();
        if (err.has_value())
            return std::make_pair(Program(), err);
        else {
            auto Pro = Program(_CONSTS, _funcs, _program);
            return std::make_pair(Pro, std::optional<CompilationError>());
            printf("hello");
        }
    }
//	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
//		auto err = analyseProgram();
//		if (err.has_value())
//			return std::make_pair(std::vector<Instruction>(), err);
//		else
//			return std::make_pair(_instructions, std::optional<CompilationError>());
//	}

	// Done
	// Done
	// mini
	// <程序> ::= 'begin'<主过程>'end'

	// c0
	// <C0-program>
	std::optional<CompilationError> Analyser::analyseProgram() {

		// <主过程>
		auto err = analyseMain();
		if (err.has_value())
			return err;

		return {};
	}

	// Done
	// Done
	// mini
	// <主过程> ::= <常量声明><变量声明><语句序列>
	// 需要补全 已补全

	// c0
    // <C0-program> ::= {<variable-declaration>}{<function-definition>}
    // 改写
    // <C0-program> ::= <variable-declaration><function-definition>
    // <variable-declaration> ::= {<variable-declaration-statement>}
    // <function-definition> ::= {<function-definition-statement>}
	std::optional<CompilationError> Analyser::analyseMain() {

        isGlabol = true;
		// <变量声明>
        auto variableDeclarationError = analyseVariableDeclaration();
        if(variableDeclarationError.has_value())
            return variableDeclarationError;

        createStack();
		// <语句序列>
        auto functionDefinitionError = analyseFunctionDefinition();
        if(functionDefinitionError.has_value())
            return functionDefinitionError;

        if(!isFunctionDeclared("main"))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrMainFunctionMissing);
		// 没有错误 返回空值
		return {};
	}

	// Done
	// Done
	// mini
	// <变量声明> ::= {<变量声明语句>}
	// <变量声明语句> ::= 'var'<标识符>['='<表达式>]';'
	// 需要补全 已补全

	// c0
    // <variable-declaration> ::= {<variable-declaration-statement>}
    // <variable-declaration-statement> ::= ['const']'int'<init-declarator-list>';'

    // 定义一个全局变量 hasConst方便init-declarator-list里面进行直接引用
    int hasConst = 0;
    std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		// 变量声明语句可能有一个或者多个
        while(true){

            // 正常终止的两种情况
            // 1、后面没有token了，正常终止
            // 2、后面有token，但不是const，也不是int

            // 情况一：后面没有token了，正常终止
            auto next = nextToken();
            if (!next.has_value())
                return {};


            // 情况二：token是const
            // 如果是 const 那么说明是常量
            // 由于const 可有可无 使用 == 判断 在if内部读取nextToken
            hasConst = 0;
            if (next.value().GetType() == TokenType::CONST) {
                hasConst = 1;
                next = nextToken();
            }

            // 下面有两种情况
            // 1、有const+不是正常的type-specifier
            //      报错返回
            // 2、无const+不是正常的type-specifier
            //      正常返回
            // 3、是正常的type-specifier
            //      继续
            // 接下来必定是int
            // 不是int 并且有const 直接报错返回

            // 这里进行有无value的判断
            // 如果没有value
            //      1、如果前面有const，那么肯定语法错误，直接报错
            //      2、如果前面没有const，那么在一开始的value检查的时候就会返回，不会运行到这里
            // 如果有value，继续||后面的分析
            if (!next.has_value() || (next.value().GetType() != TokenType::INT && hasConst == 1)) {
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
            }else if (next.value().GetType() != TokenType::INT && hasConst == 0){
                unreadToken();
                return {};
            }

            nextToken();
            next = nextToken();
            if(next.value().GetType() == TokenType::LEFT_BRACKET){
                unreadToken();
                unreadToken();
                unreadToken();
                return {};
            }else{
                unreadToken();
                unreadToken();
            }

            // int 被匹配了 每个函数都以nextToken()开头，这里不要用nextToken
            //接下来匹配<init-declarator-list>
            auto err = analyseInitDeclaratorList();
            if (err.has_value())
                return err;

            // ';'
            next = nextToken();
            auto a = next.value().GetType();
            if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON){
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
            }

            // 语法正确，添加变量到map中去
//            addVariable(tmp.value());

        }
		return {};
	}

	// Done
	// Done
    //    <init-declarator-list> ::= <init-declarator>{','<init-declarator>}
    std::optional<CompilationError> Analyser::analyseInitDeclaratorList() {

        // <init-declarator>
        auto err = analyseInitDeclarator();
        if (err.has_value())
            return err;

        // while 解决 {} 无限匹配的可能性
        while (true) {
            // 预读 判断是否正常结束匹配
            auto next = nextToken();
            if (!next.has_value())
                return {};

            // 有token，必定有值，只需要判断是不是 ','
            // 如果不是','，可能匹配下一条语句，回退返回
            auto type = next.value().GetType();
            if (type != TokenType::COMMA) {
                unreadToken();
                return {};
            }

            // 下面匹配<init-declarator>
            err = analyseInitDeclarator();
            if (err.has_value())
                return err;
        }

        return {};
    }

    // Done
    // Done
    // <init-declarator> ::= <identifier>['='<expression>]
    // 语义分析流程：
    // 检查在所有上级作用域和当前
    std::optional<CompilationError> Analyser::analyseInitDeclarator() {

        auto next = nextToken();

        // 判断是否开头为<identifier>
        if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
        }

        if (!checkDeclare(next.value().GetValueString()))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
        // 变量可能没有初始化，仍然需要一次预读
        // 预读？
        // 对是否出现错误进行判断 不初始化 要有';' 初始化要有 '='

        auto tmp = next;
        next = nextToken();
        // 没有初始化
        // 直接返回
        // 两种情况
        // 1、后面没有东西了
        // 2、后面的东西不是'='
        if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN){
            unreadToken();
            if (hasConst)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);
            addUninitializedVariable(tmp.value(), TokenType::UNSIGNED_INTEGER);
            _instructions.emplace_back(Operation::IPUSH, 0);
            return {};
        }

        // 情况二：初始化
        // 经过上面两个判断，执行到这里next的type必定是EQUAL_SIGN

        // '<expression>'
        auto err = analyseExpression();
        if (err.second.has_value())
            return err.second;
        auto ty=err.first.value()->gen();
        if(ty==VOID)
            return std::make_optional<CompilationError>(_current_pos,
                                                        ErrorCode::ErrAssignmentExpression);
        // 加入常量变量表
        if (hasConst == 1)
            addConstant(tmp.value(), TokenType::UNSIGNED_INTEGER);
        else
            addVariable(tmp.value(), TokenType::UNSIGNED_INTEGER);

        return {};
    }

	// Done
	// Done
	// mini
	// <表达式> ::= <项>{<加法型运算符><项>}

	// c0
	// <expression> ::= <multiplicative-expression>{<additive-operator><multiplicative-expression>}
	// <additive-operator> ::= '+' | '-'
    std::pair<std::optional<Analyser::Expression*>,std::optional<CompilationError>> Analyser::analyseExpression() {
        // 最后的表达式= items[0] ops[0] items[1] ops[1] items[2] ...
        std::vector<TokenType> ops;
        // 存储所有的项
        std::vector<Item> items;
		// <multiplicative-expression>
		auto err = analyseMultiplicativeExpression();
        if (err.second.has_value())
            return std::make_pair(std::optional<Expression*>(), err.second);
        items.emplace_back(err.first.value());


		// {<additive-operator><multiplicative-expression>}
		// while 解决 {} 无限匹配的可能性
		while (true) {
			// 预读 判断是否正常结束匹配
			auto next = nextToken();
			if (!next.has_value())
                return std::make_pair(new Expression(TokenType::NULL_TOKEN, ops, items), std::optional<CompilationError>());

			// 正式进入匹配
			// 匹配 <additive-operator>
			auto type = next.value().GetType();
			if (type != TokenType::PLUS_SIGN && type != TokenType::MINUS_SIGN) {
				unreadToken();
                return std::make_pair(new Expression(TokenType::NULL_TOKEN, ops, items), std::optional<CompilationError>());
			}

			// <multiplicative-expression>
			err = analyseMultiplicativeExpression();
            if (err.second.has_value())
                return std::make_pair(std::optional<Expression*>(), err.second);
            items.emplace_back(err.first.value());
            ops.emplace_back(type);

			// 根据结果生成指令
//			if (type == TokenType::PLUS_SIGN)
//				_instructions.emplace_back(Operation::ADD, 0);
//			else if (type == TokenType::MINUS_SIGN)
//				_instructions.emplace_back(Operation::SUB, 0);
		}
		return {};
	}

	// Done
	// Done
    // mini
    // <表达式> ::= <项>{<加法型运算符><项>}

    // c0
    // <multiplicative-expression> ::= <unary-expression>{<multiplicative-operator><unary-expression>}
    // <multiplicative-operator> ::= '*' | '/'
    std::pair<std::optional<Analyser::Item>,std::optional<CompilationError>> Analyser::analyseMultiplicativeExpression() {

        std::vector<TokenType> ops;
        std::vector<MulItem *> facs;

        // <unary-expression>
        auto err = analyseUnaryExpression();
        if (err.second.has_value())
            return std::make_pair(std::optional<Item>(), err.second);

        facs.emplace_back(err.first.value());

        // {<multiplicative-operator><unary-expression>}
        while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                return std::make_pair(Item(facs,ops),std::optional<CompilationError>());

            // 匹配 <multiplicative-operator>
            auto type = next.value().GetType();
            if (type != TokenType::MULTIPLICATION_SIGN && type != TokenType::DIVISION_SIGN) {
                unreadToken();
                return std::make_pair(Item(facs,ops),std::optional<CompilationError>());
            }

            // <unary-expression>
            err = analyseUnaryExpression();
            if (err.second.has_value())
                return std::make_pair(std::optional<Item>(), err.second);
            facs.emplace_back(err.first.value());
            ops.emplace_back(type);

            // 根据结果生成指令
//            if (type == TokenType::PLUS_SIGN)
//                _instructions.emplace_back(Operation::ADD, 0);
//            else if (type == TokenType::MINUS_SIGN)
//                _instructions.emplace_back(Operation::SUB, 0);
        }
        return {};
    }

	// Done
	// Done
	// <unary-expression> ::= [<unary-operator>]<primary-expression>
	// <primary-expression> ::= '('<expression>')' | <identifier>
	// | <integer-literal> | <function-call>
    std::pair<std::optional<Analyser::MulItem*>, std::optional<CompilationError>> Analyser::analyseUnaryExpression(){
        auto next = nextToken();
        if (!next.has_value())
            return std::make_pair(std::optional<Analyser::MulItem*>(),
                        std::make_optional<CompilationError>(_current_pos
                                , ErrorCode::ErrIncompleteExpression));

//        std::stringstream ss;
        // 判断是+1 还是 -1 还是 1
        TokenType sign=TokenType::PLUS_SIGN;
        if(next.value().GetType() == TokenType::PLUS_SIGN || next.value().GetType() == TokenType::MINUS_SIGN){
            sign = next.value().GetType();
        }else{
            unreadToken();
        }

        next = nextToken();
        if(!next.has_value()){
            return std::make_pair(std::optional<Analyser::MulItem*>(),
                                  std::make_optional<CompilationError>(_current_pos
                                          , ErrorCode::ErrIncompleteExpression));
        }
        switch (next.value().GetType()) {
            case TokenType::IDENTIFIER:{
                std::string str = next.value().GetValueString();
                next = nextToken();
                // 如果接下来没有token了 或者接下来不是 '(' 说明这是个变量
                if(!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET){
                    if (isDeclared(str)) {
                        Var var = getVar(str);
                        unreadToken();
                        return std::make_pair(new Variable(sign, var), std::optional<CompilationError>());
                        //利用标识符找到常量、变量在栈中的索引，利用load指令载入identifi的值
                    }else{
                        return std::make_pair(std::optional<MulItem*>(),
                                std::make_optional<CompilationError>(_current_pos,ErrorCode::ErrNotDeclared));
                    }
                }
                else {
                    if (isFunctionDeclared(str)) {
                        unreadToken();
                        unreadToken();
                        auto err = analyseFunctionCall();
                        if (err.second.has_value())
                            return std::make_pair(std::optional<Analyser::MulItem *>(), err.second);
                        return std::make_pair(err.first, std::optional<CompilationError>());
                    } else {
                        return std::make_pair(std::optional<MulItem *>(),
                                              std::make_optional<CompilationError>(_current_pos,
                                                                                   ErrorCode::ErrNotDeclared));
                    }
                }
            }
            case TokenType::UNSIGNED_INTEGER:
            case TokenType::UNSIGNED_HEX_INTEGER:
                uint32_t k;
                if (!checkState(next.value())) {
                    addCONST(next.value());
                    k = _CONSTS.size() - 1;
                } else
                    k = getConstIndex(next.value());
                return std::make_pair(new Integer(sign, k), std::optional<CompilationError>());
            case TokenType::LEFT_BRACKET:{
                auto err = analyseExpression();
                if (err.second.has_value())
                    return std::make_pair(std::optional<Analyser::MulItem*>(), err.second);
                err.first.value()->sign = sign;
                next = nextToken();
                if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
                    return std::make_pair(std::optional<Analyser::MulItem*>(),
                                          std::make_optional<CompilationError>(_current_pos,
                                                                               ErrorCode::ErrIncompleteExpression));
                return std::make_pair(err.first,std::optional<CompilationError>());
                break;
            }
            default:
                return std::make_pair(std::optional<MulItem*>(),
                        std::make_optional<CompilationError>(_current_pos,
                                ErrorCode::ErrIncompleteExpression));
        }
        return {};
    }

    // Done
    // Done
    // <function-definition> ::= {<function-definition-statement>}
    // <function-definition-statement> ::= <type-specifier><identifier><parameter-clause><compound-statement>
    // <type-specifier> ::= <simple-type-specifier>
    // <simple-type-specifier> ::= 'void'|'int'
    // 语义分析和符号处理基本流程
    // 由于要用到_instructions和_var的变量我们首先将它们分别进行保存
    // 保存之后对_instructions和_var重新进行初始化
    // 检查func有没有被预先定义
    // 没有就将func将入常量表，
    //
    std::optional<CompilationError> Analyser::analyseFunctionDefinition(){
	    while(true){
	        // 进入新的函数作用域 保存当前指令
            _program.emplace_back(_instructions);
            _instructions.erase(_instructions.begin(), _instructions.end());

            auto next = nextToken();
            if(!next.has_value()){
                return {};
            }
            // 判断是否开头为<type-specifier>
            if(next.value().GetType() != TokenType::VOID && next.value().GetType() != TokenType::INT){
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionDefinition);
            }

            // 对栈顶进行初始化
            _nextTokenIndex = 0;

            auto ret = next.value().GetType();
            _funcRetType=ret;

            // 匹配为<identifier>
            next = nextToken();
            if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER){
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionDefinition);
            }

            std::string str = next.value().GetValueString();
            if (isFunctionDeclared(str))
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
            Token fun = next.value();
            //函数名加入常量表，记录常量表中的index
            addCONST(fun);
            // 进入新的函数 设置为非全局变量
            // 保存当前区域变量表进入变量表的表
            isGlabol = false;
            pushStack();

            // 下面匹配<parameter-clause>
            auto err1 = analyseParameterClause();
            addFunction(str, 1, err1.first, ret);
            if (err1.second.has_value())
                return err1.second;

            auto err = analyseCompoundStatement();
            if (err.has_value())
                return err;
            _instructions.emplace_back(RET,0);
        }
        return {};
    }

    // Done
    // Done
    // <compound-statement> ::= '{' {<variable-declaration>} <statement-seq> '}'
    std::optional<CompilationError> Analyser::analyseCompoundStatement(){
        // 判断是否开头为'{'
        auto next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACE){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrCompoundStatement);
        }

        // {<variable-declaration>}
        while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                break;

            if(next.value().GetType() != TokenType::CONST && next.value().GetType() != TokenType::INT){
                unreadToken();
                break;
            }

            unreadToken();
            // <variable-declaration>
            auto err = analyseVariableDeclaration();
            if (err.has_value())
                return err;

            // 根据结果生成指令
//            if (type == TokenType::PLUS_SIGN)
//                _instructions.emplace_back(Operation::ADD, 0);
//            else if (type == TokenType::MINUS_SIGN)
//                _instructions.emplace_back(Operation::SUB, 0);
        }

        auto err = analyseStatementSeq();
        if (err.has_value())
            return err;


        // 判断是否结尾为'}'
        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACE){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrCompoundStatement);
        }
        _instructions.emplace_back(POPN,getVarsNum());
        popStack();
        return {};
    }

    // Done
    // Done
    // <statement-seq> ::= {<statement>}
    std::optional<CompilationError> Analyser::analyseStatementSeq(){

        // {<statement>}
        while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                return {};

            // <statement>
            auto type = next.value().GetType();
            if(type==TokenType::RIGHT_BRACE){
                unreadToken();
                return {};
            }

            unreadToken();
            auto err = analyseStatement();
            if (err.has_value())
                return err;

        }

        return {};
    }

    // Done
    // Done
    // <statement> ::=
    //     '{' <statement-seq> '}'
    //    |<condition-statement>
    //    |<loop-statement>
    //    |<jump-statement>
    //    |<print-statement>
    //    |<scan-statement>
    //    |<assignment-expression>';'
    //    |<function-call>';'
    //    |';'
    std::optional<CompilationError> Analyser::analyseStatement(){
        auto next = nextToken();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrStatement);
        std::optional<CompilationError> err;
        switch (next.value().GetType()) {
            // '{' <statement-seq> '}'
            case TokenType::LEFT_BRACE:{
                unreadToken();
                pushStack();
                err = analyseCompoundStatement();
                if (err.has_value())
                    return err;
//                next = nextToken();
//                if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACE){
//                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrStatement);
//                }
                return {};
            }
            // <condition-statement>
            case TokenType::IF:{
                unreadToken();
                err = analyseConditionStatement();
                if (err.has_value())
                    return err;
                return {};
            }
            // <loop-statement>
            case TokenType::WHILE:{
                unreadToken();
                err = analyseLoopStatement();
                if (err.has_value())
                    return err;
                return {};
            }
            // <jump-statement>
            case TokenType::RETURN:{
                unreadToken();
                err = analyseJumpStatement();
                if (err.has_value())
                    return err;
                return {};
            }
            // <print-statement>
            case TokenType::PRINT:{
                unreadToken();
                err = analysePrintStatement();
                if (err.has_value())
                    return err;
                return {};
            }
            // <scan-statement>
            case TokenType::SCAN:{
                unreadToken();
                err = analyseScanStatement();
                if (err.has_value())
                    return err;
                return {};
            }
            case TokenType::IDENTIFIER:{
                std::string str = next.value().GetValueString();
                next = nextToken();
                if(!next.has_value()){
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrStatement);
                }
                else if(next.value().GetType() == TokenType::EQUAL_SIGN){
                    // <assignment-expression>';'
                    if (isDeclared(str)) {
                        unreadToken();
                        unreadToken();
                        err = analyseAssignmentExpression();
                        if (err.has_value())
                            return err;
                    }else{
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrStatement);
                    }
                }else if(next.value().GetType() == TokenType::LEFT_BRACKET){
                    // <function-call>';'
                    if (isFunctionDeclared(str)) {
                        unreadToken();
                        unreadToken();
                        auto err = analyseFunctionCall();
                        if (err.second.has_value())
                            return err.second;
                        err.first.value()->gen();
                    }else{
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrStatement);
                    }
                }else{
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrStatement);
                }
                next = nextToken();
                if(!next.has_value() || next.value().GetType() != TokenType::SEMICOLON){
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrStatement);
                }
                return {};
            }
            case TokenType::SEMICOLON:
                return {};
            default:
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrStatement);
        }
    }

    // Done
    // Done
    // <jump-statement> ::= <return-statement>
    std::optional<CompilationError> Analyser::analyseJumpStatement(){
        auto err = analyseReturnStatement();
        if (err.has_value())
            return err;

        return {};
    }

    // Done
    // Done
    // <return-statement> ::= 'return' [<expression>] ';'
    std::optional<CompilationError> Analyser::analyseReturnStatement(){
        auto next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::RETURN){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrReturnStatement);
        }

        int hasExp = 0;
        next = nextToken();
        if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrReturnStatement);

        if(next.value().GetType() != TokenType::SEMICOLON){
            unreadToken();
            auto err = analyseExpression();
            if (err.second.has_value())
                return err.second;
            err.first.value()->gen();
            hasExp = 1;
            next = nextToken();
        }

        if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        if (!hasExp && _funcRetType == TokenType::VOID) {
            _instructions.emplace_back(Operation::POPN, _nextTokenIndex);
            _instructions.emplace_back(Operation::RET, 0);
        } else if ( hasExp && _funcRetType != TokenType::VOID) {
            _instructions.emplace_back(Operation::IRET, 0);
        } else
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrReturnStatement);
        return {};
    }


    // Done
    // Done
    // <condition-statement> ::=
    //    'if' '(' <condition> ')' <statement> ['else' <statement>]
    // JCOND .code1/.code_next
    // .code0: ifstatement
    // (
    // jmp .code_next
    // .code1: elsestatement
    // )
    // .code_next
    std::optional<CompilationError> Analyser::analyseConditionStatement(){
        auto next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::IF){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConditionStatement);
        }

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConditionStatement);
        }

        auto err = analyseCondition();
        if (err.second.has_value())
            return err.second;
        int jp=err.first.value();

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConditionStatement);
        }

        auto err2 = analyseStatement();
        if (err2.has_value())
            return err2;

        // If成立需要跳过else，如果没有else也要跳到后面
        _instructions.emplace_back(JMP,0);

        int jp2=_instructions.size()-1;
        //if不成立跳到这里
        _instructions[jp].SetX(_instructions.size());

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::ELSE){
            _instructions[jp2].SetX(_instructions.size());
            unreadToken();
            return {};
        }

        err2 = analyseStatement();
        if (err2.has_value())
            return err2;
        _instructions[jp2].SetX(_instructions.size());
        return {};

    }

    // Done
    // Done
    // <condition> ::= <expression>[<relational-operator><expression>]
    std::pair<std::optional<int32_t >,std::optional<CompilationError>> Analyser::analyseCondition(){
        auto err = analyseExpression();
        if (err.second.has_value())
            return std::make_pair(std::optional<int32_t>(),err.second);
        err.first.value()->gen();

        auto next = nextToken();

        if(!next.has_value() ||
                (next.value().GetType() != TokenType::SMALL &&
                        next.value().GetType() != TokenType::BIG &&
                        next.value().GetType() != TokenType::SMALL_EQUAL &&
                        next.value().GetType() != TokenType::BIG_EQUAL &&
                        next.value().GetType() != TokenType::NOT_EQUAL &&
                        next.value().GetType() != TokenType::EQUAL)){
            unreadToken();
            _instructions.emplace_back(Operation::JE, 0);
            return std::make_pair(_instructions.size()-1,std::optional<CompilationError>());
        }

        err = analyseExpression();
        if (err.second.has_value()) return std::make_pair(std::make_optional(int32_t()),err.second);
        err.first.value()->gen();
        _instructions.emplace_back(Operation::ISUB, 0);
        switch (next.value().GetType()) {
            case TokenType::BIG:{
                _instructions.emplace_back(Operation::JLE, 0);
                break;
            }
            case TokenType::SMALL:{
                _instructions.emplace_back(Operation::JGE, 0);
                break;
            }
            case TokenType::BIG_EQUAL:{
                _instructions.emplace_back(Operation::JL, 0);
                break;
            }
            case TokenType::SMALL_EQUAL:{
                _instructions.emplace_back(Operation::JG, 0);
                break;
            }
            case TokenType::NOT_EQUAL:{
                _instructions.emplace_back(Operation::JE, 0);
                break;
            }
            case TokenType::EQUAL:{
                _instructions.emplace_back(Operation::JNE, 0);
                break;
            }
            default: break;
        }
        return std::make_pair(_instructions.size()-1,std::optional<CompilationError>());
    }


    // Done
    // Done
    // <loop-statement> ::=
    //    'while' '(' <condition> ')' <statement>
    std::optional<CompilationError> Analyser::analyseLoopStatement(){
        auto next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::WHILE){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLoopStatement);
        }

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLoopStatement);
        }


        int s0=_instructions.size();
        auto err = analyseCondition();
        if (err.second.has_value())
            return err.second;
        int jcond=err.first.value();

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLoopStatement);
        }

        auto Serr = analyseStatement();
        if (Serr.has_value())
            return Serr;
        _instructions.emplace_back(JMP,s0);
        _instructions[jcond].SetX(_instructions.size());
        return {};
    }

    // Done
    // Done
    // <scan-statement>  ::= 'scan' '(' <identifier> ')' ';'
    std::optional<CompilationError> Analyser::analyseScanStatement(){
        auto next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::SCAN){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrScanStatement);
        }

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrScanStatement);
        }

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrScanStatement);
        }

        std::string str = next.value().GetValueString();
        if (!isDeclared(str))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);

        int index, level;
        Var var=getVar(str);
        index=var.getIndex()-1;
        level=var.isGlobal1();
        _instructions.emplace_back(Operation::LOADA, level, index);
        _instructions.emplace_back(Operation::ISCAN, 0);
        _instructions.emplace_back(Operation::ISTORE, 0);

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrScanStatement);
        }

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::SEMICOLON){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrScanStatement);
        }

        return {};
    }

    // Done
    // Done
    //  <print-statement> ::= 'print' '(' [<printable-list>] ')' ';'
    std::optional<CompilationError> Analyser::analysePrintStatement(){
        auto next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::PRINT){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrPrintStatement);
        }

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrPrintStatement);
        }

        next = nextToken();
        if(!next.has_value()){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrPrintStatement);
        }

        if(next.value().GetType() != TokenType::RIGHT_BRACKET){
            unreadToken();
            auto err = analysePrintableList();
            if (err.has_value())
                return err;
            next = nextToken();
        }
        _instructions.emplace_back(PRINTL,0);
        if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrPrintStatement);
        }

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::SEMICOLON){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrPrintStatement);
        }

        return {};
    }

    // Done
    // Done
    //  <printable-list>  ::= <printable> {',' <printable>}
    std::optional<CompilationError> Analyser::analysePrintableList(){
        auto err = analysePrintable();
        if (err.has_value())
            return err;

        while(true){
            auto next = nextToken();
            if(!next.has_value() || next.value().GetType() != TokenType::COMMA){
                unreadToken();
                return {};
            }
            _instructions.emplace_back(IPUSH,' ');
            _instructions.emplace_back(CPRINT,0);
            err = analysePrintable();
            if (err.has_value())
                return err;
        }

        return {};
    }

    // Done
    // Done
    //  <printable> ::= <expression>
    std::optional<CompilationError> Analyser::analysePrintable(){
        auto err = analyseExpression();
        if (err.second.has_value())
            return err.second;
        err.first.value()->gen();
        _instructions.emplace_back(IPRINT, 0);
        return {};
    }

    // Done
    // Done
    // <assignment-expression> ::=
    //    <identifier><assignment-operator><expression>
    std::optional<CompilationError> Analyser::analyseAssignmentExpression(){
        auto next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignmentExpression);
        }

        std::string str = next.value().GetValueString();
        if (!isDeclared(str))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
        if (isConstant(str))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
        Var var = getVar(str);

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignmentExpression);
        }

        int index, level;
        index = var.getIndex()-1;
        level = var.isGlobal1();
        _instructions.emplace_back(Operation::LOADA, level, index);//7

        auto err = analyseExpression();
        if (err.second.has_value()) return err.second;
        auto rettype=err.first.value()->gen();
        if(rettype==TokenType::VOID)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignmentExpression);
        _instructions.emplace_back(Operation::ISTORE, 0);
        return {};
    }

    // Done
    // Done
    // <parameter-clause> ::= '(' [<parameter-declaration-list>] ')'
    std::pair<std::vector<TokenType>,std::optional<CompilationError>> Analyser::analyseParameterClause(){
        // 进入新函数 清空函数参数列表
        _paras = std::vector<TokenType>();
        // 判断是否开头为<type-specifier>
        auto next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET){
            return std::make_pair(_paras,
                                  std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionDefinition));
        }

        // 接下来一个token如果不是 ')'
        // 那么必定是有参数的
        std::optional<CompilationError> err;
        next = nextToken();
        if(!next.has_value()){
            return std::make_pair(_paras,
                                  std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionDefinition));
        }
        if(next.value().GetType() != TokenType::RIGHT_BRACKET){
            unreadToken();
            err = analyseParameterDeclarationList();
            if (err.has_value())
                return std::make_pair(std::vector<TokenType>(), err);
            next = nextToken();
        }
        if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET){
            return std::make_pair(std::vector<TokenType>(), std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionDefinition));
        }

        return std::make_pair(_paras, std::optional<CompilationError>());
    }

    // Done
    // Done
    // <parameter-declaration-list> ::= <parameter-declaration>{','<parameter-declaration>}
    std::optional<CompilationError> Analyser::analyseParameterDeclarationList(){

        std::optional<CompilationError> err;
        // <parameter-declaration>
        err = analyseParameterDeclaration();
        if (err.has_value())
            return err;

        // while 解决 {} 无限匹配的可能性
        while (true) {
            // 预读 判断是否正常结束匹配
            auto next = nextToken();
            if (!next.has_value())
                return {};

            // 有token，必定有值，只需要判断是不是 ','
            // 如果不是','，可能匹配下一条语句，回退返回
            auto type = next.value().GetType();
            if (type != TokenType::COMMA) {
                unreadToken();
                return {};
            }

            // 下面匹配<parameter-declaration>
            err = analyseParameterDeclaration();
            if (err.has_value())
                return err;
        }

        return {};
    }

    // Done
    // Done
    // <parameter-declaration> ::= [<const-qualifier>]<type-specifier><identifier>
    std::optional<CompilationError> Analyser::analyseParameterDeclaration(){
	    auto next=nextToken();

        int hasConst = 0;
        if (next.value().GetType() == TokenType::CONST) {
            hasConst = 1;
            next = nextToken();
        }

        // 下面有两种情况
        // 没有东西了或者不是INT，报错返回
        // 接下来必定是int
        // 不是int 并且有const 直接报错返回

        // 这里进行有无value的判断
        // 如果没有value
        //      1、如果前面有const，那么肯定语法错误，直接报错
        //      2、如果前面没有const，那么在一开始的value检查的时候就会返回，不会运行到这里
        // 如果有value，继续||后面的分析
        if (!next.has_value() || next.value().GetType() != TokenType::INT) {
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionDefinition);
        }

        // int 被匹配了 接下来匹配IDENTIFIER
        next=nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) {
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionDefinition);
        }

        std::string str = next.value().GetValueString();

        if (!checkDeclare(str))
            return std::make_optional<CompilationError>(_current_pos,ErrorCode::ErrDuplicateDeclaration);

        // 加入参数表，符号表
        _paras.emplace_back(TokenType::UNSIGNED_INTEGER);
        if (hasConst) addConstant(next.value(), TokenType::UNSIGNED_INTEGER);
        else addVariable(next.value(), TokenType::UNSIGNED_INTEGER);

        return {};
    }


    // Done
    // Done
    // <function-call> ::= <identifier> '(' [<expression-list>] ')'
    std::pair<std::optional<Analyser::FunCall*>,std::optional<CompilationError>> Analyser::analyseFunctionCall(){
        // 判断是否开头为<identifier>
        auto next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER){
            return std::make_pair(std::optional<Analyser::FunCall*>(),
                                  std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier));
        }
        std::string str = next.value().GetValueString();

        //函数是否被声明成变量
        if (isDeclared(str))
            return std::make_pair(std::optional<Analyser::FunCall*>(),
                                  std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionCall));
        //函数是否被声明过
        if (!isFunctionDeclared(str))
            return std::make_pair(std::optional<Analyser::FunCall*>(),
                                  std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionCall));

        int index = getFuncIndex(str);
        Function function = getFunc(index);
        index--;
        std::vector<Expression *> exps;
        // 判断下一个是否为'('
        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET){
            return std::make_pair(std::optional<Analyser::FunCall*>(),
                                  std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionCall));
        }

        // 接下来一个token如果不是 ')'
        // 那么必定是有参数的
        next = nextToken();
        if(!next.has_value()){
            return std::make_pair(std::optional<Analyser::FunCall*>(),
                                  std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionCall));
        }

        if(next.value().GetType() != TokenType::RIGHT_BRACKET){
            unreadToken();
            auto err = analyseExpressionList();
            if (err.second.has_value())
                return std::make_pair(std::optional<Analyser::FunCall*>(), err.second);
            exps = err.first.value();
            next = nextToken();
            if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET){
                return std::make_pair(std::optional<Analyser::FunCall*>(),
                                      std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionCall));
            }
            return std::make_pair(new FunCall(NULL_TOKEN, function, exps, index), std::optional<CompilationError>());
        }

        if (function.getParaSize() != 0)
            return std::make_pair(std::optional<Analyser::FunCall*>(),
                                  std::make_optional<CompilationError>(_current_pos,
                                                                       ErrorCode::ErrFunctionCall));
        return std::make_pair(new FunCall(NULL_TOKEN, function, exps, index), std::optional<CompilationError>());

    }

    // Done
    // Done
    // <expression-list> ::= <expression>{','<expression>}
    std::pair<std::optional<std::vector<Analyser::Expression*>> ,std::optional<CompilationError>> Analyser::analyseExpressionList(){

        std::vector<Analyser::Expression *> exprs;

        // <expression>
        auto err = analyseExpression();
        if (err.second.has_value())
            return std::make_pair(std::optional<std::vector<Analyser::Expression *>>(), err.second);

        exprs.emplace_back(err.first.value());

        // while 解决 {} 无限匹配的可能性
        while (true) {
            // 预读 判断是否正常结束匹配
            auto next = nextToken();
            if (!next.has_value())
                return {};

            // 有token，必定有值，只需要判断是不是 ','
            // 如果不是','，可能匹配下一条语句，回退返回
            auto type = next.value().GetType();
            if (type != TokenType::COMMA) {
                unreadToken();
                return std::make_pair(exprs, std::optional<CompilationError>());
            }

            // 下面匹配<expression>
            err = analyseExpression();
            if (err.second.has_value())
                return std::make_pair(std::optional<std::vector<Analyser::Expression *>>(), err.second);

            exprs.emplace_back(err.first.value());
        }

        return {};
    }

	std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}


    //常量表、函数表操作
    void Analyser::addCONST(const Token &tk) {
        if (tk.GetType() == TokenType::IDENTIFIER) {
            std::string str = tk.GetValueString();
            _CONSTS.push_back(make_pair(str, 1));
            _constant[str] = _CONSTS.size();
        }else if (tk.GetType() == TokenType::UNSIGNED_INTEGER || tk.GetType() == TokenType::UNSIGNED_HEX_INTEGER) {
            std::string str = tk.GetValueString();
            _CONSTS.push_back(make_pair(str, 0));
            _constant[str] = _CONSTS.size();
        }

    }

    bool Analyser::isFunctionDeclared(const std::string &s) {
        if(_function.size()==0){
            return false;
        }
        return _function.find(s) != _function.end();
    }

    void Analyser::addFunction(std::string name, int level, std::vector<TokenType> &paras, TokenType &ret) {
        int nameindex = getConstIndex(Token(TokenType::IDENTIFIER, name));
        Function f(nameindex, level, paras, ret);
        _funcs.emplace_back(f);
        _function[name] = _funcs.size();
    }

    int32_t Analyser::getConstIndex(const Token &tk) {
        if (tk.GetType() == TokenType::IDENTIFIER)//string
        {
            return _constant[tk.GetValueString()] - 1;
        } else if (tk.GetType() == TokenType::UNSIGNED_HEX_INTEGER || tk.GetType() == TokenType::UNSIGNED_INTEGER) {
            std::string s=tk.GetValueString();
            return _constant[s] - 1;
        }
    }

    int32_t Analyser::getFuncIndex(const std::string &s) {
        return _function[s];
    }

    Function Analyser::getFunc(int32_t index) {
        return _funcs[index - 1];
    }

    bool Analyser::checkState(Token &token) {
        std::string s;
        s = token.GetValueString();
        return isCONST(s);

    }

    bool Analyser::isCONST(std::string basicString) {
        return _constant[basicString] != 0;
    }

    //变量表

    bool Analyser::isDeclared(const std::string &s) {
        return _findLocal(s).getIndex() != 0 || _findGlobal(s).getIndex() != 0;
    }

    bool Analyser::checkDeclare(const std::string &s) {
        if (isGlabol == true) {
            return _findGlobal(s).getIndex() == 0;
        } else {
            return _find(s, *_var).getIndex() == 0;
        }
    }

    bool Analyser::isConstant(const std::string &s) {//0
        auto p = _findLocal(s);
        auto g = _findGlobal(s);
        return (p.getIndex() != 0 && p.isConst1()) && (g.getIndex() != 0 && g.isConst1());
    }

    //底层操作，添加，查找
    void Analyser::_add(const Token &tk, std::map<std::string, Var> &sk, const TokenType &type, const bool &isConst,
                        const bool &isUnit) {
        if (tk.GetType() != TokenType::IDENTIFIER)
            DieAndPrint("only identifier can be added to the table.");
        std::string s = tk.GetValueString();
        if (isGlabol == false)
            sk[s] = Var(++_nextTokenIndex, type, isConst, isUnit, false);
        else
            sk[s] = Var(++_nextGTokenIndex, type, isConst, isUnit, true);
    }

    Var Analyser::_find(const std::string &s, std::map<std::string, Var> &sk) {
        return sk[s];
    }

    Var Analyser::_findLocal(const std::string &s) {
        if (_find(s, *_var).getIndex() != 0)
            return _find(s, *_var);
        for (int i = _var_table.size() - 1; i >= 0; i--) {
            if (_find(s, (*(_var_table[i]))).getIndex() != 0)
                return _find(s, (*(_var_table[i])));
        }
        return Var();
    }

    Var Analyser::_findGlobal(const std::string &s) {
        if (_find(s, (g_var)).getIndex() != 0)
            return _find(s, (g_var));
        return Var();
    }

    Var Analyser::getVar(const std::string &s) {
        auto var = _findLocal(s), gvar = _findGlobal(s);
        if (var.getIndex() != 0)
            return var;
        if (gvar.getIndex() != 0)
            return gvar;
        return Var();
    }

    //添加变量、常量、未初始化变量
    void Analyser::addVariable(const Token &tk, const TokenType &type) {
        if (isGlabol == true)
            _add(tk, g_var, type, false, false);
        else
            _add(tk, (*_var), type, false, false);
    }

    void Analyser::addConstant(const Token &tk, const TokenType &type) {
        if (isGlabol == true)
            _add(tk, g_var, type, true, false);
        else
            _add(tk, (*_var), type, true, false);
    }

    void Analyser::addUninitializedVariable(const Token &tk, const TokenType &type) {
        if (isGlabol == true)
            _add(tk, g_var, type, false, true);
        else
            _add(tk, (*_var), type, false, true);
    }

    void Analyser::pushStack() {
        _var_table.emplace_back(_var);
        _var = new std::map<std::string, Var>();
    }

    void Analyser::popStack() {
        (*_var).clear();
        _var = _var_table.back();
        _var_table.pop_back();
    }

    void Analyser::createStack() {
	    // 程序开始创建第一个栈帧
        _var = new std::map<std::string, Var>();
    }

    int32_t Analyser::getVarsNum()
    {
        int num=0;
        for(auto it :*_var)
        {
            if(it.second.getIndex()!=0)
                num++;
        }
        return num;
    }
}