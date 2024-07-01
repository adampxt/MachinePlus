#include <iostream>
#include <vector>
#include <memory>
#include <variant>
#define asts vector<node>
#include <fstream>
#include <algorithm>
#define lib
#include "asm.cpp"


struct s_call;
struct s_args;
struct s_pos;
struct s_flag;
struct s_ret;
struct s_assign;
struct s_gettype;
struct s_body;
struct s_num;
struct s_id;
struct s_mod;
struct s_div;
struct s_mult;
struct s_minus;
struct s_plus;

struct s_fun_decl;
struct s_fun_defn;

struct s_fun_args;

struct s_var_decl;
struct s_var_defn;

struct s_flags;

using namespace std;

string filename = "";
vector<string> _split(string str, char separator = '\n');
string code = "";
#define YYSTYPE node
struct node;
#include "grammar.h"

map<string, node> objects = {};

template<typename T> static T* copy(T* src) {
    return new T{*src};
}

template<typename T> static T* move(T* src) {
    auto temp = *src;
    free(src);
    return new T{temp};
}

enum comtype {
    _num,
    _id,
    _plus,
    _minus,
    _mult,
    _div,
    _mod,
    _fun_decl,
    _fun_defn,
    _fun_args,
    _var_decl,
    _var_defn,
    _body,
    _gettype,
    _assign,
    _ret,
    _flags,
    _flag,
    _pos,
    _args,
    _call,
    _null
};

struct node {
    comtype type;
    YYLTYPE loc;
    union {
        s_num* num;
        s_id* id;
        s_plus* plus;
        s_minus* minus;
        s_mult* mult;
        s_div* div;
        s_mod* mod;

        s_fun_decl* fun_decl;
        s_fun_defn* fun_defn;
        s_fun_args* fun_args;
        s_var_decl* var_decl;
        s_var_defn* var_defn;

        s_body* body;
        s_gettype* gettype;

        s_assign* assign;
        s_ret* ret;

        s_flag* flag;
        s_flags* flags;

        s_pos* pos;
        s_args* args;
        s_call* call;
    };
};
#define YYERROR_VERBOSE
#define UNPACK( ... ) __VA_ARGS__
#define null new node {_null, {}}
#define newnode(name, value) node {_##name, yylloc, .name = new s_##name{UNPACK value}}
#define NODE(name, body) struct s_##name {body}
#define elem node*

struct flag{enum {reserve, noreserve, fast, stack, immortal, locate, none} type; vector<elem> params = {};};
NODE(flags, vector<s_flag*> flags; );
NODE(flag, flag flag; );

NODE(num, string value; );
NODE(id, string value; );
NODE(plus, elem a; elem b; );
NODE(minus, elem a; elem b; );
NODE(mult, elem a; elem b; );
NODE(div, elem a; elem b; );
NODE(mod, elem a; elem b; );

NODE(fun_args, vector<s_var_decl*> args; );
NODE(args, vector<elem> args; );
NODE(call, elem func; s_args args; );

NODE(body, vector<node> body; );



NODE(fun_decl, s_flags flags; elem rettype; s_id name; s_fun_args params;);
NODE(fun_defn, s_flags flags; elem rettype; s_id name; s_fun_args params; s_body body; );
NODE(var_decl, s_flags flags; elem vartype; s_id name; );
NODE(var_defn, s_flags flags; elem vartype; s_id name; elem value; );

NODE(gettype, elem value; );
NODE(assign, elem a; elem b; );
NODE(ret, elem expr; );

NODE(pos, s_id id = {}; bool isprev = false; bool instart = false; bool inend = false; );

template<typename T = int> T stop(T val = {}) {
    return val;
}

void error(string err, string file, YYLTYPE _pos);
void error(string err, YYLTYPE _pos);

vector<string> tnames = {
    "byte",
    "short",
    "int",
    "long"
};

#include "lexer.c"

asts c = {};
#include "grammar.c"



using namespace std;

bool noprep = false;
bool preponly = false;
bool bin = false;
bool s = false;
bool bpp = false;
bool b16 = false;
bool b32 = false;
bool b64 = false;

bool havefile = false;

ofstream out("test.asm");

struct expr {string op; vector<string> opers; bool islabel = false;};
map<string, expr> cs = {};

bool isexpr(node in) {
    return in.type == _id || in.type == _num;
}

string execTree(node tree) {

    //1+2*(3/4);
    /*
     *_r1 = 3 / 4
     *_r2 = 2 * _r1
     *_r3 = 1 + _r2
     *result = _r3
     *
     */

    /*
     *mov   ax, 3
     *idiv, ax, 4
     *
     *imul  ax, 2
     *
     *add ax, 1
     *ret
     */

/*
 *mul
 *  | 1
 *  | plus
 *  |   | 2
 *  |   | 3
 *
 *  (1+2)*(3+4)
 *
 *  1 + 3
 *  in (1+3 return ) + 4
 *
 *  _r1 = 1 + 3
 *  _r2 = _r1 + 4
 *
 *  _r1 = 2 + 3
 *  _r2 = 1 * _r1
 *  result = _r2
 */

    setlocale(0, "");

    string nodename = "-"s +"r" + to_string(cs.size());

    switch (tree.type) {
        case _num:
        case _id: {
            auto temp = tree.id;
            return temp->value;
        }
        case _plus: cs[nodename] = {"+"}; goto binary;
        case _minus: cs[nodename] = {"-"}; goto binary;
        case _mult: cs[nodename] = {"*"}; goto binary;
        case _div: cs[nodename] = {"/"}; goto binary;
        case _mod: cs[nodename] = {"%"}; {
        binary:
            auto temp = tree.plus;
            cs[nodename].opers = {execTree(*temp->a), execTree(*temp->b)};
            break;
        }
        case _fun_decl:
            break;
        case _fun_defn: {
            auto temp = tree.fun_defn;
            cs[nodename] = {temp->name.value, .islabel = true};
        }

            break;
        case _fun_args:
            break;
        case _var_decl:
            break;
        case _var_defn:
            break;
        case _body:
            break;
        case _gettype:
            break;
        case _assign:
            break;
        case _ret:
            break;
        case _flags:
            break;
        case _flag:
            break;
        case _pos:
            break;
        case _args:
            break;
        case _call:
            break;
        case _null:
            break;
    }
    return nodename;
}
// var = 9;
int main(int argc, char ** argv) {
    --argc;
    ++argv;
    for(int i=0; i<argc; ++i)
    {
#define flag(name) if (string{argv[i]} == "--"#name){name = true; continue;}
        flag(noprep)
        flag(preponly)
        flag(bin)
        flag(s)
        flag(bpp)
        flag(b16)
        flag(b32)
        flag(b64)

        havefile = true;
        filename = argv[i];
    }
    if (!havefile) {
        fprintf(stderr, "First parameter <file> is missing.");
        exit(1);
    }
    ifstream file(filename);
    char ch;

    while (file.get(ch)) code += ch;

    //yy_switch_to_buffer();
    yy_switch_to_buffer(yy_scan_string(code.c_str()));
    // string e;
    // cin >> e;
    // e+=";";
    // yy_switch_to_buffer(yy_scan_string(e.c_str()));
newnode(call, ());
    //printf("Hello, World!\n");
    yyparse();
    if (haveerror) exit(1);
    for (auto i : c)
        execTree(i);//����� ����� printf �����; cout �����



    for (auto i = cs.rbegin(); i != cs.rend(); ++i) {
        if (i->second.opers.size() == 2)
            cout << i->first << " = " << i->second.opers[0] << " " << i->second.op << " " << i->second.opers[1] << endl;
        else cout << i->first << " = " << i->second.op << " " << i->second.opers[0];
    }
}
