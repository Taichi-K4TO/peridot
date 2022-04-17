/********************************************************************************************************
 *
 *      PROJECT NAME    :   ASMI Demo Contest 2022 
 *
 *      FILE NAME       :   peri.h
 *
 *      OUTLINE         :   Definitions of token codes, symbol tables, macros, etc...
 *
 *      REQUIRED FILES  :   None
 *
 *      EDITOR : Taichi KATO,   Advanced Sensing & Machine Intelligence Group,  Chukyo Univ.
 *
 *      LAST UPDATED : Feb. 17, 2022
 *
 *      Copyright © 2022 Taichi KATO. All rights reserved.
 *
*********************************************************************************************************/
/*  Header file  */
#include <iostream>
#include <fstream>  // File handling
#include <sstream>  // String streams
#include <string>
#include <vector>
#include <stack>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cctype>


using namespace std;


/* Define */
#define SHORT_SIZ  sizeof(short int)      // Size of short int type
#define SHORT_P(p) (short int *)(p)       // Convert to a pointer of short int type
#define UCHAR_P(p) (unsigned char *)(p)   // Convert to a pointer of unsigned char type
#define LIN_SIZ 255                       // Maximum size of one line of source


/* Enum Struct of Token Kind */
/* Token Elements */
enum TknKind {
  Lparen='(', Rparen=')', Lbracket='[', Rbracket=']', Plus='+',   Minus='-',
  Multi='*',  Divi='/',   Mod='%',      Not='!',      Ifsub='?',  Assign='=',
  IntDivi='\\', Comma=',',     DblQ='"',
  Func=150, Var,   If,     Elif,   Else,   For, To, Step,  While,
  End,      Break, Return, Option, Print,  Println, Input, Toint,
  Exit,     Equal, NotEq,  Less,   LessEq, Great,   GreatEq, And, Or,
  END_KeyList,
  Ident,      IntNum, DblNum, String,   Letter, Doll, Digit,
  Gvar, Lvar, Fcall,  EofProg, EofLine, Others
};


/* Struct Token */
/* Token management */
struct Token {

  TknKind kind;     // Kind of Tokens
  string  text;     // Text of Tokens
  double  dblVal;   // Value in the case of numeric constants

  Token() {  kind = Others; text = ""; dblVal = 0.0; }
  Token (TknKind k)                            { kind = k; text = ""; dblVal = 0.0; }
  Token (TknKind k, double d)                  { kind = k; text = ""; dblVal = d; }
  Token (TknKind k, const string& s)           { kind = k; text = s;  dblVal = 0.0; }
  Token (TknKind k, const string& s, double d) { kind = k; text = s;  dblVal = d; }
};


/* Type of registered name in symbol table */
enum SymKind { noId, varId, fncId, paraId };
/* Name of the type */
enum DtType  { NON_T, DBL_T };


/* Symbol table configuration */
struct SymTbl {

  string  name;     //  Names of variables and functions
  SymKind nmKind;   //  Kind
  char    dtTyp;    //  Variable type ( NON_T, DBL_T )
  int     aryLen;   //  Length of array
  short   args;     //  Number of function arguments
  int     adrs;     //  Variable and function numbers
  int     frame;    //  Frame size for function

  SymTbl() { clear(); }

  void clear() {
    name=""; nmKind=noId; dtTyp=NON_T;
    aryLen=0; args=0; adrs=0; frame=0;
  }
};


/* Code management */
struct CodeSet {

  TknKind kind;       //  Kind
  const char *text;   //  Position for character literals
  double dblVal;      //  Value in the case of numeric constants
  int    symNbr;      //  Position of subscript in symbol table
  int    jmpAdrs;     //  Jumping address

  CodeSet() { clear(); }
  CodeSet(TknKind k)                    { clear(); kind = k; }
  CodeSet(TknKind k, double d)          { clear(); kind = k; dblVal = d; }
  CodeSet(TknKind k, const char *s)     { clear(); kind = k; text = s; }
  CodeSet(TknKind k, int sym, int jmp)  { clear(); kind = k; symNbr = sym; jmpAdrs = jmp; }
  void clear() { kind=Others; text=""; dblVal=0.0; jmpAdrs=0; symNbr=-1; }
};


/* obj with type information */
struct Tobj {

  char type;  //  Retractable type  'd':double  's':string  '-':undefined
  double d;
  string s;

  Tobj()                 { type = '-'; d = 0.0; s = ""; }
  Tobj(double dt)        { type = 'd'; d = dt;  s = ""; }
  Tobj(const string& st) { type = 's'; d = 0.0; s = st; }
  Tobj(const char *st)   { type = 's'; d = 0.0; s = st; }
};


class Mymemory {

private:
  vector<double> mem;
public:
  void auto_resize(int n) {   //  Secure more to reduce the number of re-securing
    if (n >= (int)mem.size()) { n = (n/256 + 1) * 256; mem.resize(n); }
  }
  void set(int adrs, double dt) { mem[adrs] =  dt; }        //  Memory writing
  void add(int adrs, double dt) { mem[adrs] += dt; }        //  Memory addition
  double get(int adrs)          { return mem[adrs]; }       //  Memory reading
  int size()                    { return (int)mem.size(); } //  Storage size
  void resize(unsigned int n)   { mem.resize(n); }          //  Securing size

};

