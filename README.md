# Compiler Principles Lab Report

## Preface

This is a report about Compiler Principles Lab for 2022 Spring. In this report, you will see the tasks and problems I've solved during the whole process.

### Environment

CPU: Intel(R) Xeon(R) Platinum 8163 CPU @ 2.50GHz

Disk: 40 GiB

Operating System: Ubuntu 20.04.3 LTS

Memory (Virtualized): 2GB

*Thanks for my friend Rui Ding. He provides me with this machine from Ali Cloud, on condition that I cannot use hyper-V on my laptop.*

## Before Everything

I met a lot of trouble when setting my environment. First, I tried to install `docker` on my server, and failed on starting docker service. After a short confusion, I realized that this server (from my lab) is organized using docker service. It seems ridiculous to run a docker in a docker. Therefore, I turned to try installing docker on my laptop, and got stuck in enabling hyper-V.  I didn't feel too unexpected or annoyed, just because my laptop had gone through too much ravage and abuse from myself. Then, I started a docker using the image from TA,  allocated it with a port number using portainer (which is used to organize our lab server). Then, I used `ssh` to log into that docker, and found some environment variables lost. After adding them manually, I finally succeeded in running `makefile`. However, after writing a few lines, I failed to compile my new codes. When staring at the codes but unable to find any error, I began to suspect my environment again. Recalling the loss of environment variables before, I decided to give up using ssh to access this docker.

I searched all of my servers. Some of them have been aborted for a long time, and some of them denied my permission. Finally I turned to my friend and begged him to give me an account on his server. Thanks to his kindness, at last I started a docker on his server, and my codes finally worked.

## Lv 1

In this stage, we first write a simple program to realize **lexical analysis** and **parser**.

We should first edit the `sysy.l` file to deal with comments using regular expressions. Just add:

```flex
LA [/]
LB [*]
LC [^*/]
LineComment   "//".*|"/*"{LA}*({LC}{LA}*|{LB}|{LC})*"*/"
```

Then, just copy and **understand** the sample codes, and then organize them well. We can get a "Repeater" program after finishing lv1.2, and then we begin to parse the `main` function. We've handled the comments, so now we only need to consider the case with only one `return` instruction. Our task is designing `AST`, and our main job is write codes in a header `AST.h`, and include it in other files.

We need to **define** AST of `CompUnit`, `FuncDef `, `FuncType`, `Block`, `Stmt` and `Number`. We use class in C++, define a basic `BaseAST`, and inherit it when dealing with different types. Until now, our AST classes only contain some member variables.  

Then, to **generate** AST, we should edit the parser rules in  `sysy.y`. We should generate an AST (in my opinion, similar to `new ` in C) and pass it to the caller of the parser. 

Then, to **output** AST, we should add some printer functions. Here we add `dump`, a member function of AST class. Then by calling it in the `main` function, we can see   whether our program runs well.

The last thing is to **output** IR. It is quite similar to what we do to output AST. Actually, I rewrite the `dump` function, change it to `dumpIR`, and then redirect STDOUT to the output file I want.

An example is shown here:

```bison
Block
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->Stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;
```

The above codes are added to `sysy.y`.

```C++
class BlockAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> Stmt; 
  void Dump() const override {
    std::cout << "BlockAST { ";
    Stmt->Dump();
    std::cout << " }";
  }
  void DumpIR() const override {
    std::cout << "\%entry:"<<endl;
    Stmt->DumpIR();
  }
};
```

The above codes are added to `AST.h`.

When writing C/C++ codes, using `#ifdef` is always a good habit. Because I need the program to output in my terminal when debugging, so I use these instructions:

```C++
// #define _SUB_MODE
#ifdef  _SUB_MODE
freopen(output, "w", stdout); 
#endif
```


## Lv 2

In this stage, we try on generating RISC-V codes using koopaIR.

The first thing to do is editting codes in lv1, because when generating IR i directly used `std::cout` function. I used a string to store all IR.

Then to generate RISC-V codes, we can use `koopa.h`. It is really a great helper and organize all the partitions well. Then we can just traverse the program by starting with `raw` by using `Visit` functions, and adding some outputting instructions.

A simple example is shown here:

```c++
void Visit(const koopa_raw_function_t &func) {
  cout<<"  .globl "<<func->name+1 <<"\n";
  cout<<func->name+1<<":\n";
  Visit(func->bbs);
}
```

Additionally I add a `RISCV.h`, which is only responsible for transferring koopaIR into RISC-V.

## Lv 3

In level 3, we should make our compiler able to deal with an expression as the return value.

Because the grammar has changed, so we should first edit `sysy.l` file, adding the operation symbols:

```flex
RelOP         [<=|>=]?
EqOP          [==|!=]

{RelOP}         { yylval.str_val = new string(yytext); return RELOP; }
{EqOP}          { yylval.str_val = new string(yytext); return EQOP; }
"&&"            { yylval.str_val = new string(yytext); return ANDOP; }
"||"            { yylval.str_val = new string(yytext); return OROP; }
```

And then we should edit `sysy.y` file. Here I add a lot of things, including tokens and non-terminal symbols:

```bison
%token <str_val> IDENT
%token <int_val> INT_CONST
%token <str_val> RELOP EQOP ANDOP OROP

%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp AddExp MulExp
%type <ast_val> RelExp EqExp LAndExp LOrExp Decl ConstDecl ConstDef ConstInitVal BlockItem ConstExp
%type <ast_val> VarDecl VarDef InitVal OpenStmt ClosedStmt SimpleStmt
%type <vec_val> BlockItem_List ConstDef_List VarDef_List
%type <int_val> Number
%type <str_val> UnaryOp BType LVal
```

And the generation rules are also changed. `Stmt` are changed to of the form: `RETURN Exp ';'`.

And the most complicated part is adding the generation rules. I mainly following the doc:

```enbf
Exp         ::= LOrExp;
PrimaryExp  ::= '(' Exp ')'| Number | LVal;
Number      ::= INT_CONST;
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp     ::= "+" | "-" | "!";
MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;;
AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;;
RelExp      ::= AddExp | RelExp RELOP AddExp;
EqExp       ::= RelExp | EqExp EQOP RelExp;
LAndExp     ::= EqExp | LAndExp ANDOP EqExp;
LOrExp      ::= LAndExp | LOrExp OROP LAndExp;
```

Here `RELOP`, `EQOP`, `ANDOP` and `OROP` are all defined in the `sysy.l` file.

Additionally, for every rule, I choose to design an AST for each rule on the right side of `::=`, and construct the corresponding AST when the corresponding rule is parsed.

For example, my `LAndExp` looks like:

```bison
LAndExp
  : EqExp {
    auto land_exp = new LAndExpAST();
    land_exp->op = "";
    land_exp->eq_exp = unique_ptr<BaseAST>($1);
    $$ = land_exp;
  }
  | LAndExp ANDOP EqExp {
    auto land_exp = new LAndExpAST();
    land_exp->land_exp = unique_ptr<BaseAST>($1);
    land_exp->op = *unique_ptr<string>($2);
    land_exp->eq_exp = unique_ptr<BaseAST>($3);
    $$ = land_exp;
  }
  ;
```

Here the symbols ends with `Exp` means that the returned value should be an AST, but the operation can just be a string.

And now is the `AST.h` part.

Here because calculating the value of expressions may be divided to some parts: calculating the left part, and then the right part, and then combine them, the returning value might be `void` or `int` or `string`, so I rewrite the `DumpIR` function outside the definition of class, and take one AST as the argument. Additionally, I gave up using a string to store all IR, but still use the io buffer, adding some instructions:

```c++
//改变输出流到缓冲区，再存到字符串里
stringstream ss;
streambuf* cout_buf = cout.rdbuf();
cout.rdbuf(ss.rdbuf());
DumpIR((CompUnitAST*)(ast.get()));
string ir_str = ss.str();
const char *ir = ir_str.data();
cout.rdbuf(cout_buf);
```

These are copied from CSDN, to make RISCV generation function do further work.

For the `unary_exp` case, we just deal with +, - or ! case:

```c++
switch (unary_exp->op[0])
{
    case '+':
        return result_var;
        break;
    //unchanged
    case '-':
        cout << "  " << next_var << " = sub 0, " << result_var << "\n";
        break;
    //use 0 to sub
    case '!':
        cout << "  " << next_var << " = eq " << result_var << ", 0" << "\n";
        break;
    //test whether equal
    default:
        assert(false);
}
symbol_num++;
return next_var;
```

We can just maintain a `symbol_num` to record current number of symbols.

And for multiple expression, we should deal with the left part and right part respectively. The instructions are similar to calculating suffix expression:

```c++
static string DumpIR(const MulExpAST *mul_exp) {
    string result_var = "";
    if (mul_exp->op == "")
        result_var = DumpIR((UnaryExpAST*)(mul_exp->unary_exp.get()));
    else {
        string left_result = DumpIR((MulExpAST*)(mul_exp->mul_exp.get()));
        string right_result = DumpIR((UnaryExpAST*)(mul_exp->unary_exp.get()));
        result_var = "%" + to_string(symbol_num++);
        switch (mul_exp->op[0])
        {
        case '*':
            cout << "  " << result_var << " = mul " << left_result << ", " << right_result << "\n";
            break;
        case '/':
            cout << "  " << result_var << " = div " << left_result << ", " << right_result << "\n";
            break;
        case '%':
            cout << "  " << result_var << " = mod " << left_result << ", " << right_result << "\n";
            break;
        default:
            assert(false);
            break;
        }
    }

    return result_var;
}
```

As for other expression, we also divide the process into calculating `left_result` and `right_result`.

The codes shown above are to generate AST. As for calculating the number, we use a `DumpEXP` function and do like this:

```c++
static int DumpEXP(const MulExpAST *mul_exp) {
    int result = 0;
    if (mul_exp->op == "")
        result = DumpEXP((UnaryExpAST*)(mul_exp->unary_exp.get()));
    else {
        int left_result = DumpEXP((MulExpAST*)(mul_exp->mul_exp.get()));
        int right_result = DumpEXP((UnaryExpAST*)(mul_exp->unary_exp.get()));
        switch (mul_exp->op[0])
        {
        case '%':
            result = left_result % right_result;
            break;
        case '*':
            result = left_result * right_result;
            break;
        case '/':
            result = left_result / right_result;
            break;
        default:
            assert(false);
            break;
        }
    }

    return result;
}
```

As for the RISC-V part, we first define the register list:

```c++
string reg_names[16] = {"a0", "a1", "a2", "a3", "a4", "a5", "a6","a7", "t0", "t1", "t2", "t3", "t4", "t5", "t6","x0"};
```

And their states:

```c++
int reg_stats[16] = {0};          
```

And add rules for all binary operations:

```c++
Reg RISC_Visit(const koopa_raw_binary_t &binary)
{
    // returns memory reg_add
    struct Reg left_val = RISC_Visit(binary.lhs);
    int left_register = left_val.reg_name;
    int old_stat = reg_stats[left_register];
    reg_stats[left_register] = 2;
    struct Reg right_val = RISC_Visit(binary.rhs);
    int right_register = right_val.reg_name;
    reg_stats[left_register] = old_stat;
    struct Reg result_var = {find_reg(1), -1};

    string left_reg_name = reg_names[left_register];
    string right_reg_name = reg_names[right_register];
    string result_reg_name = reg_names[result_var.reg_name];

    switch (binary.op)
    {
    case 0: // ne
        cout << "  "<< "xor " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        cout << "  "<< "snez " << result_reg_name << ", " << result_reg_name << "\n";
        break;
    case 1: // eq
        cout << "  "<< "xor " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        cout << "  "<< "seqz " << result_reg_name << ", " << result_reg_name << "\n";
        break;
    case 2: // gt
        cout << "  " << "sgt " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        break;
    case 3: // lt
        cout << "  "<< "slt " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        break;
    case 4: // ge 
        cout << "  "<< "slt " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        cout << "  "<< "xori " << result_reg_name << ", " << result_reg_name << ", 1" << "\n";
        break;
    case 5: // le
        cout << "  "<< "sgt " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        cout << "  "<< "xori " << result_reg_name << ", " << result_reg_name << ", 1" << "\n";
        break;
    case 6: // add
        cout << "  "<< "add " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        break;
    case 7: // sub
        cout << "  "<< "sub " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        break;
    case 8: // mul
        cout << "  "<< "mul " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        break;
    case 9: // div
        cout << "  "<< "div " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        break;
    case 10: // mod
        cout << "  "<< "rem " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        break;
    case 11: // and
        cout << "  "<< "and " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        break;
    case 12: // or
        cout << "  "<< "or " << result_reg_name << ", " << left_reg_name << ", " << right_reg_name << "\n";
        break;
    default:
        assert(false);
    }

    return result_var;
}

```

## lv 4

In this level, we should deal with new symbols. To do so, we should maintain a `symbol_table`, with this declaration:

```c++
map<string, variant<int, string> > symbol_table;
```

The `sysy.y` should be editted to support these rules:

```
Decl          ::= ConstDecl | VarDecl;
ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
BType         ::= "int";
ConstDef      ::= IDENT "=" ConstInitVal;
ConstInitVal  ::= ConstExp;
VarDecl       ::= BType VarDef {"," VarDef} ";";
VarDef        ::= IDENT | IDENT "=" InitVal;
InitVal       ::= Exp;
```

And when to add a new symbol (variant), the instructions are:

```c++
static void DumpIR(const VarDefAST *var_def) {
    string var_name = "@" + var_def->ident;
    string name = var_name + "_" + to_string(var_names[var_name]++);
    cout << "  " << name << " = alloc i32" << "\n";
    int i = symbol_tables.size() - 1;
    symbol_tables[i][var_def->ident] = name;
    if (var_def->has_init_val) {
        string value = DumpIR((ExpAST*)((InitValAST*)(var_def->init_val.get()))->exp.get());
        cout << "  store " << value << ", " << name << "\n";
    }
}
```

And this is for new variant declaration:

```c++
static void DumpIR(const VarDeclAST *var_decl) {
    assert(var_decl->b_type == "int"); // Only support int type
    int size = var_decl->var_def_list.size();
    for (int i = 0; i < size; ++i)
        DumpIR((VarDefAST*)(var_decl->var_def_list[i].get()));
}
```

And as for the RISCV part, an important part is managing frame.

I add the two declaration:

```c++
int stack_top = 0;
static map<uintptr_t, int> stack_frame;
```

And for store operation:

```c++
void RISC_Visit(const koopa_raw_store_t &store)
{
    struct Reg value = RISC_Visit(store.value);
    koopa_raw_value_t dest = store.dest;
    assert(value_map.count(dest));
    if (value_map[dest].reg_add == -1)
    {
        value_map[dest].reg_add = stack_top;
        stack_top += 4;
    }
    int reg_name = value.reg_name, reg_add = value_map[dest].reg_add;
    cout << "  " << "sw " << reg_names[reg_name] << ", " << to_string(reg_add) << "(sp)" << "\n";
}
```

And for alloc:

```c++
case KOOPA_RVT_ALLOC:
        result_var.reg_add = stack_top;
        stack_top += 4;
        value_map[value] = result_var;
        break;
```

We also add a stack_frame size calculating function:

```c++
int cal_size(const koopa_raw_slice_t &slice)
{
    int stack_frame_size = 0;
    for (size_t i = 0; i < slice.len; ++i)
    {
        auto ptr = slice.buffer[i];
        assert(slice.kind == KOOPA_RSIK_VALUE);
        auto value = reinterpret_cast<koopa_raw_value_t>(ptr);
        if (value->kind.tag == KOOPA_RVT_ALLOC || value->ty->tag != KOOPA_RTT_UNIT)
        {
            stack_frame[reinterpret_cast<uintptr_t>(value)] = stack_frame_size;
            stack_frame_size++;
        }
    }
    if ((stack_frame_size & 3) > 0)
    {
        stack_frame_size = ((stack_frame_size >> 2) + 1) << 2;
    }
    return stack_frame_size;
}
```



## lv 5

In this level, we should deal with blocks, and each block may introduce some new symbols. To do so, we should maintain a `symbol_tables`, which is composed of many `symbol_table`, with this declaration:

```c++
static vector<map<string, variant<int, string> > > symbol_tables;
```

And in `void DumpIR(const BlockAST *block)` function, we should add some instructions:

```c++
map<string, variant<int, string> > symbol_table;//在本block内建立符号表
symbol_tables.push_back(symbol_table);
int size = block->block_item_list.size();
for (int i = 0; i < size; ++i)
    DumpIR((BlockItemAST*)(block->block_item_list[i].get()));
symbol_tables.pop_back();
```

Because the symbol table should be deleted when escaping the block, so a `pop_back` instruction is needed.

This means that we should affiliate a symbol table to each block.

And the RISCV part doesn't need any change.

## lv 6

To deal with if and else, I rewrite the rules for `Stmt`:

```bison
Stmt
  : OpenStmt {
    auto stmt = ($1);
    $$ = stmt;
  }
  | ClosedStmt {
    auto stmt = ($1);
    $$ = stmt;
  }
  ;

ClosedStmt
  : SimpleStmt {
    auto stmt = new StmtAST();
    stmt->type = "simple";
    stmt->exp_simple = unique_ptr<BaseAST>($1);
    $$ = stmt;
  }
  | IF '(' Exp ')' ClosedStmt ELSE ClosedStmt {
    auto stmt = new StmtAST();
    stmt->type = "ifelse";
    stmt->exp_simple = unique_ptr<BaseAST>($3);
    stmt->if_stmt = unique_ptr<BaseAST>($5);
    stmt->else_stmt = unique_ptr<BaseAST>($7);
    $$ = stmt;
  }
  ;

OpenStmt
  : IF '(' Exp ')' Stmt {
    auto stmt = new StmtAST();
    stmt->type = "if";
    stmt->exp_simple = unique_ptr<BaseAST>($3);
    stmt->if_stmt = unique_ptr<BaseAST>($5);
    $$ = stmt;
  }
  | IF '(' Exp ')' ClosedStmt ELSE OpenStmt {
    auto stmt = new StmtAST();
    stmt->type = "ifelse";
    stmt->exp_simple = unique_ptr<BaseAST>($3);
    stmt->if_stmt = unique_ptr<BaseAST>($5);
    stmt->else_stmt = unique_ptr<BaseAST>($7);
    $$ = stmt;
  }
  ;

SimpleStmt
  : RETURN Exp ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "ret";
    stmt->block_exp = unique_ptr<BaseAST>($2);
    $$ = stmt;
  }
  | RETURN ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "ret";
    stmt->block_exp = nullptr;
    $$ = stmt;
  }
  | LVal '=' Exp ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "lval";
    stmt->l_val = *unique_ptr<string>($1);
    stmt->block_exp = unique_ptr<BaseAST>($3);
    $$ = stmt;
  }
  | Block {
    auto stmt = new SimpleStmtAST();
    stmt->type = "block";
    stmt->block_exp = unique_ptr<BaseAST>($1);
    $$ = stmt;
  }
  | Exp ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "exp";
    stmt->block_exp = unique_ptr<BaseAST>($1);
    $$ = stmt;
  }
  | ';' {
    auto stmt = new SimpleStmtAST();
    stmt->type = "exp";
    stmt->block_exp = nullptr;
    $$ = stmt;
  }
  ;
```

Here `OpenStmt` means those Stmt that might not end with a `else`, for example:

```c++
if (a == 1)
{
	if (b == 1)
		{do...}
	else if (b == 2)
		{do...}
}
```

We also set a global variant `if_else_num` to identify every if/else block.

And when we stmt is if/else type, we would write  `if_else_num` after if or else.

To realize short circuit evaluation, we change the codes from

```c
result = Cal_AST((LAndExpAST*)(land_exp->land_exp.get()))&&Cal_AST((EqExpAST*)(land_exp->eq_exp.get()));
```

```c++
int left_result = Cal_AST((LAndExpAST*)(land_exp->land_exp.get()));
if (left_result == 0)
    return 0;
result = (Cal_AST((EqExpAST*)(land_exp->eq_exp.get())) != 0);
```

As for the RISCV part, we add two functons:

```c++
void RISC_Visit(const koopa_raw_branch_t &branch)
{
    string true_label = branch.true_bb->name + 1;
    string false_label = branch.false_bb->name + 1;
    int cond_reg = RISC_Visit(branch.cond).reg_name;
    cout << "  " << "bnez " << reg_names[cond_reg] << ", " << true_label << "\n";
    cout << "  " << "j " << false_label << "\n";
}

void RISC_Visit(const koopa_raw_jump_t &jump)
{
    string target = jump.target->name + 1;
    cout << "  " << "j " << target << "\n";
}
```

Here `int cond_reg = RISC_Visit(branch.cond).reg_name` is an important instruction, because it calculate the condition value, and return a register name, telling us which to compare.
