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

Additionally I add a `KIR.h`, which is only responsible for transferring koopaIR into RISC-V.
