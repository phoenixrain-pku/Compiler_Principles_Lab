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
RelOP         [<|>][=]?
EqOP          [=|!][=]

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

