#pragma once

#include <string>
#include <iostream>
#include <cassert>

#include <map>
#include "koopa.h"

using namespace std;

struct Reg
{
    int reg_name;
    int reg_add;
};

int stack_top = 0;
static map<uintptr_t, int> stack_frame;

int stack_size = 0;
string reg_names[16] = {"a0", "a1", "a2", "a3", "a4", "a5", "a6","a7", "t0", "t1", "t2", "t3", "t4", "t5", "t6","x0"};
koopa_raw_value_t registers[16]; 
int reg_stats[16] = {0};

koopa_raw_value_t present_value = 0; // 当前正访问的指令

map<const koopa_raw_value_t, Reg> value_map;

// Declaration of the functions
void RISC_Visit(const koopa_raw_program_t &program);
void RISC_Visit(const koopa_raw_slice_t &slice);
void RISC_Visit(const koopa_raw_function_t &func);
void RISC_Visit(const koopa_raw_basic_block_t &bb);
void RISC_Visit(const koopa_raw_return_t &ret);
Reg RISC_Visit(const koopa_raw_value_t &value);
Reg RISC_Visit(const koopa_raw_integer_t &integer);
Reg RISC_Visit(const koopa_raw_binary_t &binary);
Reg RISC_Visit(const koopa_raw_load_t &load);
void RISC_Visit(const koopa_raw_store_t &store);
void RISC_Visit(const koopa_raw_branch_t &branch);
void RISC_Visit(const koopa_raw_jump_t &jump);
int find_reg(int stat);
int cal_size(const koopa_raw_slice_t &slice);

int find_reg(int stat)
{
    for (int i = 0; i < 15; ++i)
    {
        if (reg_stats[i] == 0)
        {
            registers[i] = present_value;
            reg_stats[i] = stat;
            return i;
        }
    }
    for (int i = 0; i < 15; ++i)
    {
        if (reg_stats[i] == 1)
        {
            value_map[registers[i]].reg_name = -1;
            int add = value_map[registers[i]].reg_add;
            if (add == -1)
            {
                add = stack_top;
                stack_top += 4;
                value_map[registers[i]].reg_add = add;
                cout << "  sw " << reg_names[i] << ", " << to_string(add) << "(sp)" << "\n";
            }
            registers[i] = present_value;
            reg_stats[i] = stat;
            return i;
        }
    }
    assert(false);
    return -1;
}

// 访问 raw program
void RISC_Visit(const koopa_raw_program_t &program)
{
    cout << ".text" << "\n";
    // 访问所有全局变量
    RISC_Visit(program.values);
    // 访问所有函数
    RISC_Visit(program.funcs);
}

// 访问 raw slice
void RISC_Visit(const koopa_raw_slice_t &slice)
{
    for (size_t i = 0; i < slice.len; ++i)
    {
        auto ptr = slice.buffer[i];
        // 根据 slice 的 kind 决定将 ptr 视作何种元素
        switch (slice.kind)
        {
        case KOOPA_RSIK_FUNCTION:
            // 访问函数
            RISC_Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            // 访问基本块
            RISC_Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            break;
        case KOOPA_RSIK_VALUE:
            // 访问指令
            RISC_Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
            break;
        default:
            // 我们暂时不会遇到其他内容, 于是不对其做任何处理
            assert(false);
        }
    }
}

// 访问函数
void RISC_Visit(const koopa_raw_function_t &func)
{
    cout << "  " << ".globl " << (func->name + 1) << "\n";
    cout << (func->name + 1) << ":" << "\n";
    RISC_Visit(func->bbs);
}

// 访问基本块
void RISC_Visit(const koopa_raw_basic_block_t &bb)
{
    int stack_frame_size = cal_size(bb->insts);
    cout << "addi sp, sp, -" + to_string(stack_frame_size * 4) << "\n";
    // 访问所有指令
    cout << bb->name + 1 << ":" << "\n";
    RISC_Visit(bb->insts);
}

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

Reg RISC_Visit(const koopa_raw_value_t &value)
{
    // 返回时值一定在寄存器里
    koopa_raw_value_t old_value = present_value;
    present_value = value;
    if (value_map.count(value))
    {
        if (value_map[value].reg_name == -1)
        {
            int reg_name = find_reg(1);
            value_map[value].reg_name = reg_name;
            cout << "  "    << "lw " << reg_names[reg_name] << ", " << to_string(value_map[value].reg_add) << "(sp)" << "\n";
        }
        present_value = old_value;
        return value_map[value];
    }

    const auto &kind = value->kind;
    struct Reg result_var = {-1, -1};
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN:
        // 访问 return 指令
        RISC_Visit(kind.data.ret);
        break;
    case KOOPA_RVT_INTEGER:
        // 访问 integer 指令
        result_var = RISC_Visit(kind.data.integer); 
        break;
    case KOOPA_RVT_BINARY:
        result_var = RISC_Visit(kind.data.binary);
        value_map[value] = result_var;
        break;
    case KOOPA_RVT_ALLOC:
        result_var.reg_add = stack_top;
        stack_top += 4;
        value_map[value] = result_var;
        break;
    case KOOPA_RVT_LOAD:
        result_var = RISC_Visit(kind.data.load);
        value_map[value] = result_var;
        break;
    case KOOPA_RVT_STORE:
        RISC_Visit(kind.data.store);
        break;
    case KOOPA_RVT_BRANCH:
        RISC_Visit(kind.data.branch);
        break;
    case KOOPA_RVT_JUMP:
        RISC_Visit(kind.data.jump);
        break;
    default:
        assert(false);
    }

    present_value = old_value; // 防止递归时改掉值

    return result_var;
}

void RISC_Visit(const koopa_raw_return_t &ret)
{
    koopa_raw_value_t ret_value = ret.value;
    struct Reg result_var = RISC_Visit(ret_value);
    cout << "  " << "mv a0, " << reg_names[result_var.reg_name] << "\n";
    if (stack_size <= 2048)
        cout << "  "<< "addi sp, sp, " << to_string(stack_size) << "\n";
    else
    {
        cout << "  "<< "li t0, " << to_string(stack_size) << "\n";
        cout << "  "<< "addi sp, sp, t0" << "\n";
    }
    cout << "  " << "ret" << "\n";
}

Reg RISC_Visit(const koopa_raw_integer_t &integer)
{
    // integer stored in registers, thus returns register number
    int32_t int_val = integer.value;
    struct Reg result_var = {-1, -1};
    if (int_val == 0)
    {
        result_var.reg_name = 15;
        return result_var;
    }
    result_var.reg_name = find_reg(0);
    cout << "  " << "li " << reg_names[result_var.reg_name] << ", " << to_string(int_val) << "\n";

    return result_var;
}

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

Reg RISC_Visit(const koopa_raw_load_t &load)
{
    koopa_raw_value_t src = load.src;
    int reg_name = find_reg(1), reg_add = value_map[src].reg_add;
    struct Reg result_var = {reg_name, reg_add};
    cout << "  " << "lw " << reg_names[reg_name] << ", " << to_string(reg_add) << "(sp)" << "\n";

    return result_var;
}

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
