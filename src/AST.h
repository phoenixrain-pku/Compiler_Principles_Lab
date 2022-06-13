#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <variant>
#include <sstream>
using namespace std;

static int symbol_num = 0;
static vector<map<string, variant<int, string> > > symbol_tables;
static map<string, int> var_names;

static int if_else_num = 0;
static int other_num = 0;

class BaseAST 
{
 public:
  virtual ~BaseAST() = default;
};
class ExpAst
{
    public:
    string symbolexp;
    virtual ~ExpAst() = default;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST 
{
 public:
  // 用智能指针管理对象
    unique_ptr<BaseAST> func_def;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
 public:
    unique_ptr<BaseAST> func_type;
    string ident;
    unique_ptr<BaseAST> block;
};

class FuncTypeAST : public BaseAST
{
 public:
    string functype;
};

class BlockAST : public BaseAST
{
 public:
    vector<unique_ptr<BaseAST> > block_item_list; 
};

class StmtAST : public BaseAST
{
public:
    string type; // "if", "ifelse" or "simple"
    unique_ptr<BaseAST> exp_simple;
    unique_ptr<BaseAST> if_stmt;
    unique_ptr<BaseAST> else_stmt;
};

class SimpleStmtAST : public BaseAST
{
public:
    string type; // "lval", "exp", "block" or "ret"
    string l_val;
    unique_ptr<BaseAST> block_exp;
};

class ExpAST : public BaseAST
{
public:
    unique_ptr<BaseAST> lor_exp;
};

class PrimaryExpAST : public BaseAST
{
public:
    string type; // "exp", "number" or "lval"
    unique_ptr<BaseAST> exp;
    string l_val;
    int number;
};

class UnaryExpAST : public BaseAST
{
public:
    string type; // "primary" or "unary"
    unique_ptr<BaseAST> exp;
    string op;
};

class MulExpAST : public BaseAST
{
public:
    string op; // "*", "/", "%" or ""
    unique_ptr<BaseAST> unary_exp;
    unique_ptr<BaseAST> mul_exp;
};

class AddExpAST : public BaseAST
{
public:
    string op; // "+", "-" or ""
    unique_ptr<BaseAST> add_exp;
    unique_ptr<BaseAST> mul_exp;
};

class RelExpAST : public BaseAST
{
public:
    string op; // "<", ">", "<=", ">=" or ""
    unique_ptr<BaseAST> add_exp;
    unique_ptr<BaseAST> rel_exp;
};

class EqExpAST : public BaseAST
{
public:
    string op; // "==", "!=" or ""
    unique_ptr<BaseAST> eq_exp;
    unique_ptr<BaseAST> rel_exp;
};

class LAndExpAST : public BaseAST
{
public:
    string op; // "&&" or ""
    unique_ptr<BaseAST> eq_exp;
    unique_ptr<BaseAST> land_exp;
};

class LOrExpAST : public BaseAST
{
public:
    string op; // "||" or ""
    unique_ptr<BaseAST> land_exp;
    unique_ptr<BaseAST> lor_exp;
};

class DeclAST : public BaseAST
{
public:
    string type; // "const_decl" or "var_decl"
    unique_ptr<BaseAST> decl;
};

class ConstDeclAST : public BaseAST
{
public:
    string b_type;
    vector<unique_ptr<BaseAST> > const_def_list;
};

class ConstDefAST : public BaseAST
{
public:
    string ident;
    unique_ptr<BaseAST> const_init_val;
};

class ConstInitValAST : public BaseAST
{
public:
    unique_ptr<BaseAST> const_exp;
};

class BlockItemAST : public BaseAST
{
public:
    string type; // "decl" or "stmt"
    unique_ptr<BaseAST> content;
};

class ConstExpAST : public BaseAST
{
public:
    unique_ptr<BaseAST> exp;
};

class VarDeclAST : public BaseAST
{
public:
    string b_type;
    vector<unique_ptr<BaseAST> > var_def_list;
};

class VarDefAST : public BaseAST
{
public:
    string ident;
    bool has_init_val;
    unique_ptr<BaseAST> init_val;
};

class InitValAST : public BaseAST
{
public:
    unique_ptr<BaseAST> exp;
};


static void DumpIR(const CompUnitAST *comp_unit);
static void DumpIR(const FuncDefAST *func_def);
static void DumpIR(const BlockAST *block);
static void DumpIR(const StmtAST *stmt);
static void DumpIR(const SimpleStmtAST *stmt);
static void DumpIR(const DeclAST *decl);
static void DumpIR(const BlockItemAST *block_item);
static void DumpIR(const ConstDeclAST *const_decl);
static void DumpIR(const ConstDefAST *const_def);
static void DumpIR(const VarDeclAST *var_decl);
static void DumpIR(const VarDefAST *var_def);
static string DumpIR(const ExpAST *exp);
static string DumpIR(const UnaryExpAST *unary_exp);
static string DumpIR(const PrimaryExpAST *primary_exp);
static string DumpIR(const MulExpAST *mul_exp);
static string DumpIR(const AddExpAST *add_exp);
static string DumpIR(const RelExpAST *rel_exp);
static string DumpIR(const EqExpAST *eq_exp);
static string DumpIR(const LAndExpAST *land_exp);
static string DumpIR(const LOrExpAST *lor_exp);
static string DumpIR(const InitValAST *init_val);
static int DumpIR(const ConstInitValAST *const_init_val);
static int DumpEXP(const ConstExpAST *const_exp);
static int DumpEXP(const ExpAST *exp);
static int DumpEXP(const UnaryExpAST *unary_exp);
static int DumpEXP(const PrimaryExpAST *primary_exp);
static int DumpEXP(const MulExpAST *mul_exp);
static int DumpEXP(const AddExpAST *add_exp);
static int DumpEXP(const RelExpAST *rel_exp);
static int DumpEXP(const EqExpAST *eq_exp);
static int DumpEXP(const LAndExpAST *land_exp);
static int DumpEXP(const LOrExpAST *lor_exp);
static variant<int, string> look_up_symbol_tables(string l_val);

static variant<int, string> look_up_symbol_tables(string l_val)
{
    int size = symbol_tables.size();
    for (int i = size - 1; i >= 0; --i)
{
        if (symbol_tables[i].count(l_val))
            return symbol_tables[i][l_val];
    }

    assert(false);
    return -1;
}

static void DumpIR(const CompUnitAST *comp_unit)
{
    DumpIR((FuncDefAST *)(comp_unit->func_def.get()));
}

static void DumpIR(const FuncDefAST *func_def)
{
    //改变输出流到缓冲区，再存到字符串里
    stringstream ss;
    streambuf *cout_buf = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    cout << "fun @" << func_def->ident << "(): ";
    string type = ((FuncTypeAST *)(func_def->func_type.get()))->functype;
    assert(type=="int");
    cout << "i32{" << "\n";
    cout << "%" << "entry" << ":\n"; // Blocks will have their names in the future
    DumpIR((BlockAST *)(func_def->block.get()));

    string ir_str = ss.str(), last_line = "";
    int pt = ir_str.length() - 2;
    while(ir_str[pt] != '\n')
        last_line = ir_str[pt--] + last_line;
    if (last_line.substr(0, 6) == "%other") // deal with empty ret block
        ir_str = ir_str.substr(0, pt+1);
    cout.rdbuf(cout_buf);

    cout << ir_str << "}\n";
}

static void DumpIR(const BlockAST *block)
{
    map<string, variant<int, string> > symbol_table;//在本block内建立符号表
    symbol_tables.push_back(symbol_table);
    int symbol_tables_size = block->block_item_list.size();
    for (int i = 0; i < symbol_tables_size; ++i)
        DumpIR((BlockItemAST *)(block->block_item_list[i].get()));
    symbol_tables.pop_back();
}

static void DumpIR(const SimpleStmtAST *stmt)
{
    if (stmt->type == "ret")
{
        if (stmt->block_exp == nullptr)
            cout << "  " << "ret\n";
        else
        {
            string result_var = DumpIR((ExpAST *)(stmt->block_exp.get()));
            cout << "  " << "ret " << result_var << "\n";
        }
        string other_label = "\%other_" + to_string(other_num++);
        cout << other_label << ":\n";
    }
    else if (stmt->type == "lval")
    {
        string result_var = DumpIR((ExpAST *)(stmt->block_exp.get()));
        variant<int, string> value = look_up_symbol_tables(stmt->l_val);
        assert(value.index() == 1);
        cout << "  " << "store " << result_var << ", " << get<1>(value) << "\n";
    }
    else if (stmt->type == "exp")
    {
        if (stmt->block_exp != nullptr)
            DumpIR((ExpAST *)(stmt->block_exp.get()));
    }
    else if (stmt->type == "block")
        DumpIR((BlockAST *)(stmt->block_exp.get()));
    else
        assert(false);
}

static void DumpIR(const StmtAST *stmt)
{
    if (stmt->type == "simple")
        DumpIR((SimpleStmtAST *)(stmt->exp_simple.get()));
    else if (stmt->type == "if")
    {
        string if_result = DumpIR((ExpAST *)(stmt->exp_simple.get()));
        string label_then = "\%then_" + to_string(if_else_num);
        string label_end = "\%end_" + to_string(if_else_num);
        if_else_num++;
        cout << "  " << "br " << if_result << ", " << label_then << ", " << label_end << "\n";
        cout << label_then << ":" << "\n";
        DumpIR((StmtAST *)(stmt->if_stmt.get()));
        cout << "  " << "jump " << label_end << "\n";
        cout << label_end << ":\n";
    }
    else if (stmt->type == "ifelse")
    {
        string if_result = DumpIR((ExpAST *)(stmt->exp_simple.get()));
        string label_then = "\%then_" + to_string(if_else_num);
        string label_else = "\%else_" + to_string(if_else_num);
        string label_end = "\%end_" + to_string(if_else_num);
        if_else_num++;
        cout << "  " << "br " << if_result << ", " << label_then << ", " << label_else << "\n";
        cout << label_then << ":\n";
        DumpIR((StmtAST *)(stmt->if_stmt.get()));
        cout << "  " << "jump " << label_end << "\n";
        cout << label_else << ":\n";
        DumpIR((StmtAST *)(stmt->else_stmt.get()));
        cout << "  " << "jump " << label_end << "\n";
        cout << label_end << ":\n";
    }
    else
        assert(false);
}

static string DumpIR(const ExpAST *exp)
{
    string result_var = DumpIR((LOrExpAST *)(exp->lor_exp.get()));
    return result_var;
}

static string DumpIR(const UnaryExpAST *unary_exp)
{
    if (unary_exp->type == "primary")
    {
        string result_var = DumpIR((PrimaryExpAST *)(unary_exp->exp.get()));
        return result_var;
    }
    else if (unary_exp->type == "unary")
    {
        string result_var = DumpIR((UnaryExpAST *)(unary_exp->exp.get()));
        string next_var = "%" + to_string(symbol_num);

        if (unary_exp->op[0] == '+')
            return result_var;
        else if (unary_exp->op[0] == '-')
            cout << "  " << next_var << " = sub 0, " << result_var << "\n";
        else if (unary_exp->op[0] == '!')
            cout << "  " << next_var << " = eq " << result_var << ", 0" << "\n";
        else
            assert(false);
        symbol_num++;
        return next_var;
    }
    else
    {
        assert(false);
    }
    return "";
}

static string DumpIR(const PrimaryExpAST *primary_exp)
{
    string result_var = "";
    if (primary_exp->type == "exp")
        result_var = DumpIR((ExpAST *)(primary_exp->exp.get()));
    else if (primary_exp->type == "number")
        result_var = to_string(primary_exp->number);
    else if (primary_exp->type == "lval")
    {
        variant<int, string> value = look_up_symbol_tables(primary_exp->l_val);
        if (value.index() == 0) // const_var
            result_var = to_string(get<0>(value));
        else
        { // var
            result_var = "%" + to_string(symbol_num);
            ++symbol_num;
            cout << "  " << result_var << " = load " << get<1>(value) << "\n";
        }
    }
    else
        assert(false);

    return result_var;
}

static string DumpIR(const MulExpAST *mul_exp)
{
    string result_var = "";
    if (mul_exp->op == "")
        result_var = DumpIR((UnaryExpAST *)(mul_exp->unary_exp.get()));
    else
    {
    string left_result = DumpIR((MulExpAST *)(mul_exp->mul_exp.get()));
    string right_result = DumpIR((UnaryExpAST *)(mul_exp->unary_exp.get()));
    result_var = "%" + to_string(symbol_num++);
        if (mul_exp->op[0] == '*')
            cout << "  " << result_var << " = mul " << left_result << ", " << right_result << "\n";
        else if (mul_exp->op[0] == '/')
            cout << "  " << result_var << " = div " << left_result << ", " << right_result << "\n";
        else if (mul_exp->op[0] == '%')
            cout << "  " << result_var << " = mod " << left_result << ", " << right_result << "\n";
        else
            assert(false);
    }

    return result_var;
}

static string DumpIR(const AddExpAST *add_exp)
{
    string result_var = "";
    if (add_exp->op == "")
        result_var = DumpIR((MulExpAST *)(add_exp->mul_exp.get()));
    else
    {
    string left_result = DumpIR((AddExpAST *)(add_exp->add_exp.get()));
    string right_result = DumpIR((MulExpAST *)(add_exp->mul_exp.get()));
    result_var = "%" + to_string(symbol_num++);
        if (add_exp->op[0] == '+')
            cout << "  " << result_var << " = add " << left_result << ", " << right_result << "\n";
        else if (add_exp->op[0] == '-')
            cout << "  " << result_var << " = sub " << left_result << ", " << right_result << "\n";
     else
        assert(false);
    }

    return result_var;
}

static string DumpIR(const RelExpAST *rel_exp)
{
    string result_var = "";
    if (rel_exp->op == "")
        result_var = DumpIR((AddExpAST *)(rel_exp->add_exp.get()));
    else
    {
        string left_result = DumpIR((RelExpAST *)(rel_exp->rel_exp.get()));
        string right_result = DumpIR((AddExpAST *)(rel_exp->add_exp.get()));
        result_var = "%" + to_string(symbol_num++);
        if (rel_exp->op == "<")
            cout << "  " << result_var << " = lt " << left_result << ", " << right_result << "\n";
        else if (rel_exp->op == ">")
            cout << "  " << result_var << " = gt " << left_result << ", " << right_result << "\n";
        else if (rel_exp->op == "<=")
            cout << "  " << result_var << " = le " << left_result << ", " << right_result << "\n";
        else if (rel_exp->op == ">=")
            cout << "  " << result_var << " = ge " << left_result << ", " << right_result << "\n";
        else
            assert(false);
    }

    return result_var;
}

static string DumpIR(const EqExpAST *eq_exp)
{
    string result_var = "";
    if (eq_exp->op == "")
        result_var = DumpIR((RelExpAST *)(eq_exp->rel_exp.get()));
    else
    {
        string left_result = DumpIR((EqExpAST *)(eq_exp->eq_exp.get()));
        string right_result = DumpIR((RelExpAST *)(eq_exp->rel_exp.get()));
        result_var = "%" + to_string(symbol_num);
        ++symbol_num;
        if (eq_exp->op == "==")
            cout << "  " << result_var << " = eq " << left_result << ", " << right_result << "\n";
        else if (eq_exp->op == "!=")
            cout << "  " << result_var << " = ne " << left_result << ", " << right_result << "\n";
        else
            assert(false);
    }

    return result_var;
}

static string DumpIR(const LAndExpAST *land_exp)
{
    string result_var = "";
    if (land_exp->op == "")
        result_var = DumpIR((EqExpAST *)(land_exp->eq_exp.get()));
    else if (land_exp->op == "&&")
{
        string left_result = DumpIR((LAndExpAST *)(land_exp->land_exp.get()));
        string label_then = "\%then_" + to_string(if_else_num);
        string label_else = "\%else_" + to_string(if_else_num);
        string label_end = "\%end_" + to_string(if_else_num);
        ++if_else_num;

        string result_var_ptr = "%" + to_string(symbol_num);
        ++symbol_num;
        string temp_result_var = "%" + to_string(symbol_num);
        ++symbol_num;
        string right_result = DumpIR((EqExpAST *)(land_exp->eq_exp.get()));
        cout << "  " << result_var_ptr << " = alloc i32" << "\n";
        cout << "  " << "br " << left_result << ", " << label_then << ", " << label_else << "\n";
        cout << label_then << ":" << "\n";
        cout << "  " << temp_result_var << " = ne " << right_result << ", 0" << "\n";
        cout << "  " << "store " << temp_result_var << ", " << result_var_ptr << "\n";
        cout << "  " << "jump " << label_end << "\n";
        cout << label_else << ":" << "\n";
        cout << "  " << "store 0, " << result_var_ptr << "\n";
        cout << "  " << "jump " << label_end << "\n";
        cout << label_end << ":" << "\n";
        result_var = "%" + to_string(symbol_num++);
        cout << "  " << result_var << " = load " << result_var_ptr << "\n";
    }
    else
        assert(false);

    return result_var;
}

static string DumpIR(const LOrExpAST *lor_exp)
{
    string result_var = "";
    if (lor_exp->op == "")
        result_var = DumpIR((LAndExpAST *)(lor_exp->land_exp.get()));
    else if (lor_exp->op == "||")
    {
        string left_result = DumpIR((LOrExpAST *)(lor_exp->lor_exp.get()));
        string label_then = "\%then_" + to_string(if_else_num);
        string label_else = "\%else_" + to_string(if_else_num);
        string label_end = "\%end_" + to_string(if_else_num);
        ++if_else_num;

        string result_var_ptr = "\%" + to_string(symbol_num);
        ++symbol_num;
        cout << "  " << result_var_ptr << " = alloc i32" << "\n";
        cout << "  " << "br " << left_result << ", " << label_then << ", " << label_else << "\n";
        cout << label_then << ":" << "\n";
        cout << "  " << "store 1, " << result_var_ptr << "\n";
        cout << "  " << "jump " << label_end << "\n";
        cout << label_else << ":" << "\n";
        string temp_result_var = "%" + to_string(symbol_num++);
        string right_result = DumpIR((LAndExpAST *)(lor_exp->land_exp.get()));
        cout << "  " << temp_result_var << " = ne " << right_result << ", 0" << "\n";
        cout << "  " << "store " << temp_result_var << ", " << result_var_ptr << "\n";
        cout << "  " << "jump " << label_end << "\n";
        cout << label_end << ":" << "\n";
        result_var = "%" + to_string(symbol_num++);
        cout << "  " << result_var << " = load " << result_var_ptr << "\n";
    }
    else
        assert(false);

    return result_var;
}

static int DumpEXP(const ExpAST *exp)
{
    int result = DumpEXP((LOrExpAST *)(exp->lor_exp.get()));
    return result;
}

static int DumpEXP(const UnaryExpAST *unary_exp)
{
    int result = 0;
    if (unary_exp->type == "primary")
        result = DumpEXP((PrimaryExpAST *)(unary_exp->exp.get()));
    else if (unary_exp->type == "unary")
    {
        int temp = DumpEXP((UnaryExpAST *)(unary_exp->exp.get()));

            if (unary_exp->op[0]=='+')
                result = temp;
            else if (unary_exp->op[0]=='-')
                result = -temp;
            else if (unary_exp->op[0]=='!')
                result = !temp;
            else
                assert(false);

    }
    else
        assert(false);

    return result;
}

static int DumpEXP(const PrimaryExpAST *primary_exp)
{
    int result = 0;
    if (primary_exp->type == "exp")
        result = DumpEXP((ExpAST *)(primary_exp->exp.get()));
    else if (primary_exp->type == "number")
        result = primary_exp->number;
    else if (primary_exp->type == "lval")
    {
        variant<int, string> value = look_up_symbol_tables(primary_exp->l_val);
        assert(value.index() == 0);
        result = get<0>(value);
    }
    else
        assert(false);

    return result;
}

static int DumpEXP(const MulExpAST *mul_exp)
{
    int result = 0;
    if (mul_exp->op == "")
        result = DumpEXP((UnaryExpAST *)(mul_exp->unary_exp.get()));
    else
    {
    int left_result = DumpEXP((MulExpAST *)(mul_exp->mul_exp.get()));
    int right_result = DumpEXP((UnaryExpAST *)(mul_exp->unary_exp.get()));
        if(mul_exp->op[0] == '*')
            result = left_result * right_result;
        else if(mul_exp->op[0] == '/')
            result = left_result / right_result;
        else if(mul_exp->op[0] == '%')
            result = left_result % right_result;
        else
            assert(false);
    }

    return result;
}

static int DumpEXP(const AddExpAST *add_exp)
{
    int result = 0;
    if (add_exp->op == "")
        result = DumpEXP((MulExpAST *)(add_exp->mul_exp.get()));
    else
    {
    int left_result = DumpEXP((AddExpAST *)(add_exp->add_exp.get()));
    int right_result = DumpEXP((MulExpAST *)(add_exp->mul_exp.get()));
        if(add_exp->op[0] == '+')
            result = left_result + right_result;
        else if(add_exp->op[0] == '-')
            result = left_result - right_result;
        else
            assert(false);
    }

    return result;
}

static int DumpEXP(const RelExpAST *rel_exp)
{
    int result = 0;
    if (rel_exp->op == "")
        result = DumpEXP((AddExpAST *)(rel_exp->add_exp.get()));
    else
    {
        int left_result = DumpEXP((RelExpAST *)(rel_exp->rel_exp.get()));
        int right_result = DumpEXP((AddExpAST *)(rel_exp->add_exp.get()));
        if (rel_exp->op == ">")
            result = (bool)(left_result > right_result);
        else if (rel_exp->op == ">=")
            result = (bool)(left_result >= right_result);
        else if (rel_exp->op == "<")
            result = (bool)(left_result < right_result);
        else if (rel_exp->op == "<=")
            result = (bool)(left_result <= right_result);
        else
            assert(false);
    }

    return result;
}

static int DumpEXP(const EqExpAST *eq_exp)
{
    int result = 0;
    if (eq_exp->op == "")
        result = DumpEXP((RelExpAST *)(eq_exp->rel_exp.get()));
    else
    {
        int left_result = DumpEXP((EqExpAST *)(eq_exp->eq_exp.get()));
        int right_result = DumpEXP((RelExpAST *)(eq_exp->rel_exp.get()));
        if (eq_exp->op == "==")
            result = (bool)(left_result == right_result);
        else if (eq_exp->op == "!=")
            result = (bool)(left_result != right_result);
        else
            assert(false);
    }

    return result;
}

static int DumpEXP(const LAndExpAST *land_exp)
{
    int result = 0;
    if (land_exp->op == "")
        result = DumpEXP((EqExpAST *)(land_exp->eq_exp.get()));
    else if (land_exp->op == "&&")
    {
        int left_result = DumpEXP((LAndExpAST *)(land_exp->land_exp.get()));
        if (left_result == 0)
            return 0;
        result = (DumpEXP((EqExpAST *)(land_exp->eq_exp.get())) != 0);
    }
    else
        assert(false);

    return result;
}

static int DumpEXP(const LOrExpAST *lor_exp)
{
    int result = 1;
    if (lor_exp->op == "")
        result = DumpEXP((LAndExpAST *)(lor_exp->land_exp.get()));
    else if (lor_exp->op == "||")
    {
        int left_result = DumpEXP((LOrExpAST *)(lor_exp->lor_exp.get()));
        if (left_result)
            return 1;
        result = (DumpEXP((LAndExpAST *)(lor_exp->land_exp.get())) != 0);
    }
    else
        assert(false);

    return result;
}

static void DumpIR(const DeclAST *decl)
{
    if (decl->type == "const_decl")
        DumpIR((ConstDeclAST *)(decl->decl.get()));
    else if (decl->type == "var_decl")
        DumpIR((VarDeclAST *)(decl->decl.get()));
    else
        assert(false);
}

static void DumpIR(const BlockItemAST *block_item)
{
    if (block_item->type == "decl")
        DumpIR((DeclAST *)(block_item->content.get()));
    else if (block_item->type == "stmt")
        DumpIR((StmtAST *)(block_item->content.get()));
    else
        assert(false);
}

static void DumpIR(const ConstDeclAST *const_decl)
{
    assert(const_decl->b_type == "int"); // Only support int at present
    int size = const_decl->const_def_list.size();
    for (int i = 0; i < size; ++i)
        DumpIR((ConstDefAST *)(const_decl->const_def_list[i].get()));
}

static void DumpIR(const ConstDefAST *const_def)
{
    int i = symbol_tables.size() - 1;
    symbol_tables[i][const_def->ident] = DumpIR((ConstInitValAST *)(const_def->const_init_val.get()));
}

int DumpIR(const ConstInitValAST *const_init_val)
{
    return DumpEXP((ConstExpAST *)(const_init_val->const_exp.get()));
}

static int DumpEXP(const ConstExpAST *const_exp)
{
    return DumpEXP((ExpAST *)(const_exp->exp.get()));
}

static void DumpIR(const VarDeclAST *var_decl)
{
    assert(var_decl->b_type == "int"); // Only support int type
    int size = var_decl->var_def_list.size();
    for (int i = 0; i < size; ++i)
        DumpIR((VarDefAST *)(var_decl->var_def_list[i].get()));
}

static void DumpIR(const VarDefAST *var_def)
{
    string var_name = "@" + var_def->ident;
    string name = var_name + "_" + to_string(var_names[var_name]++);
    cout << "  " << name << " = alloc i32" << "\n";
    int i = symbol_tables.size() - 1;
    symbol_tables[i][var_def->ident] = name;
    if (var_def->has_init_val)
    {
        string value = DumpIR((InitValAST *)(var_def->init_val.get()));
        cout << "  store " << value << ", " << name << "\n";
    }
}

static string DumpIR(const InitValAST *init_val)
{
    return DumpIR((ExpAST *)(init_val->exp.get()));
}
