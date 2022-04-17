/********************************************************************************************************
 *
 *      PROJECT NAME    :   ASMI Demo Contest 2022 
 *
 *      FILE NAME       :   peri_code.cpp
 *
 *      OUTLINE         :   Code Processing and Execution
 *
 *      REQUIRED FILES  :   peri.h, peri_prot.h
 *
 *      EDITOR : Taichi KATO,   Advanced Sensing & Machine Intelligence Group,  Chukyo Univ.
 *
 *      LAST UPDATED : Apl. 17, 2022
 *
 *      Copyright © 2022 Taichi KATO. All rights reserved.
 *
*********************************************************************************************************/
/* Header File */
#include "peri.h"
#include "peri_prot.h"


CodeSet code;                           //  Code Set 
int startPc;                            //  Execution Start Line
int Pc = -1;                            //  Program Counter   -1 : in progress
int baseReg;                            //  Base Register
int spReg;                              //  Stack Pointer
int maxLine;                            //  Program End Line
vector<char*> intercode;                //  Converted Internal Code Storage
char *code_ptr;                         //  Pointer for Internal Code Analysis
double returnValue;                     //  Function's Return Value
bool break_Flg, return_Flg, exit_Flg;   //  Controls Flags
Mymemory Dmem;                          //  Main Memory
vector<string> strLITERAL;              //  String Literal Storage
vector<double> nbrLITERAL;              //  Numerical Value Literal Storage
bool syntaxChk_mode = false;            //  TRUE if Syntax Check System is ON
extern vector<SymTbl> Gtable;           //  Global Symbols Table


/* Rapper of "stack<double>" */
class Mystack { 
private:
  stack<double> st;
public:
  void push(double n) { st.push(n); }   //  Pushing
  int size() { return (int)st.size(); } //  Size
  bool empty() { return st.empty(); }   //  Judge empty or not
  double pop() {                        //  Pop & Delete (Caution! :Different from the original Pop)
    if (st.empty()) err_exit("stack underflow");
    double d = st.top();                //  Top value
    st.pop(); return d;                 //  Delete one
  }
};
Mystack stk;                            //  Oporand Stack


/* Syntax Check */
void syntaxChk() {

  syntaxChk_mode = true;

  for (Pc=1; Pc<(int)intercode.size(); Pc++) {

    code = firstCode(Pc);

    switch (code.kind) {
    case Func: case Option: case Var:                       //  Already Checked
      break;
    case Else: case End: case Exit:
      code = nextCode(); chk_EofLine();
      break;
    case If: case Elif: case While:
      code = nextCode(); (void)get_expression(0, EofLine);  //  Expression Value
      break;
    case For:
      code = nextCode();
      (void)get_memAdrs(code);                              //  Control Variable Adress
      (void)get_expression('=', 0);                         //  Initial Value
      (void)get_expression(To, 0);                          //  Last    Value
      if (code.kind == Step) (void)get_expression(Step,0);  //  Step    Value
      chk_EofLine();
      break;
    case Fcall:                                             //  Function Calls without Assignments
      fncCall_syntax(code.symNbr);
      chk_EofLine();
      (void)stk.pop();                                      //  No Return Value Required
      break;
    case Print: case Println:
      sysFncExec_syntax(code.kind);
      break;
    case Gvar: case Lvar:                                   //  Assinnment Statements
      (void)get_memAdrs(code);                              //  Left  Side Adress
      (void)get_expression('=', EofLine);                   //  Right Side Expression Value
      break;  
    case Return:
      code = nextCode(); 
      if (code.kind!='?' && code.kind!=EofLine) (void)get_expression();   //  Return Value
      if (code.kind == '?') (void)get_expression('?', 0);
      chk_EofLine();
      break;
    case Break:
      code = nextCode();
      if (code.kind == '?') (void)get_expression('?', 0);
      chk_EofLine();
      break;
    case EofLine:
      break;
    default:
      err_exit("Incorrect Value : ", kind_to_s(code.kind));
    }
  }
  syntaxChk_mode = false;
}


/* Setting Start Line */
void set_startPc(int n) {
  startPc = n;
}


/* Execution */
void execute() {
  baseReg = 0;                      //  Base  Register's Initial Value
  spReg = Dmem.size();              //  Stack Register's Initial Value
  Dmem.resize(spReg+1000);          //  First Secure of Main Memory Area
  break_Flg = return_Flg = exit_Flg = false;

  Pc = startPc;
  maxLine = intercode.size() - 1;
  while (Pc<=maxLine && !exit_Flg) {
    statement();
  }
  Pc = -1;                          //  Non-execute Mode
}


/* Statement, String */
void statement() {
  CodeSet save;
  int top_line, end_line, varAdrs;
  double wkVal, endDt, stepDt;

  if (Pc > maxLine || exit_Flg) return;                   //  Program Terminations
  code = save = firstCode(Pc);
  top_line = Pc; end_line = code.jmpAdrs;                 //  Beginning and Termination of the Control Range
  if (code.kind == If ) end_line = endline_of_If(Pc);     //  Termination for if statements

  switch (code.kind) {
  case If:                                                //  If
    if (get_expression(If, 0)) {                          //  If TRUE, execute and exit.
      ++Pc; block(); Pc = end_line + 1; 
      return; 
    }
    Pc = save.jmpAdrs;                                    //  Go Next...
    while (lookCode(Pc) == Elif) {
      save = firstCode(Pc); code = nextCode();
      if (get_expression()) {                             //  If TRUE, execute and exit.
        ++Pc; block(); Pc = end_line + 1;
        return; 
      }
      Pc = save.jmpAdrs;                                  //  Go Next...
    }
    if (lookCode(Pc) == Else) {                           //  If TRUE, execute and exit.
      ++Pc; block(); Pc = end_line + 1;
      return; 
    }
    ++Pc;
    break;

  case While:
    for (;;) {
      if (!get_expression(While, 0)) break;               //  False End
      ++Pc; block();
      if (break_Flg || return_Flg || exit_Flg) {
        break_Flg = false; break;                         //  Break
      }  
      Pc = top_line; code = firstCode(Pc);                //  Go Head
    } 
    Pc = end_line + 1;
    break;

  case For:
    save = nextCode();
    varAdrs = get_memAdrs(save);                          //  Get Control Variable Adress

    expression('=', 0);                                   //  Initial Value
    set_dtTyp(save, DBL_T);                               //  Determine Type
    Dmem.set(varAdrs, stk.pop());                         //  Set Initial Value

    endDt = get_expression(To, 0);                        //  Store the Last Value

    if (code.kind == Step) stepDt = get_expression(Step, 0); else stepDt = 1.0;   //  Store the Step Value
    for (;; Pc=top_line) { 
      if (stepDt >= 0) {   
        if (Dmem.get(varAdrs) > endDt) break;             //  if False, Break
      } else { 
        if (Dmem.get(varAdrs) < endDt) break;             //  if False, Break
      } 
      ++Pc; block();
      if (break_Flg || return_Flg || exit_Flg) {
        break_Flg = false; break;                         //  Break
      } 
      Dmem.add(varAdrs, stepDt);                          //  Update Value
    } 
    Pc = end_line + 1;
    break;

  case Fcall:                                             //  Function Calls without Assignments
    fncCall(code.symNbr);
    (void)stk.pop();                                      //  No Return Value Required  
    ++Pc;
    break;

  case Func:                                              //  Skipping Fuction Definition
    Pc = end_line + 1;
    break;

  case Print: case Println:
    sysFncExec(code.kind);
    ++Pc;
    break;

  case Gvar: case Lvar:                                   //  Assignment Statements
    varAdrs = get_memAdrs(code);
    expression('=', 0);
    set_dtTyp(save, DBL_T);                               //  Determinate Type at Assignment
    Dmem.set(varAdrs, stk.pop());
    ++Pc;
    break;

  case Return:
    wkVal = returnValue;
    code = nextCode();
    if (code.kind!='?' && code.kind!=EofLine)             //  If it has an Expression, calculate the return path.
      wkVal = get_expression();
    post_if_set(return_Flg);                              //  If it has a "?", process
    if (return_Flg) returnValue = wkVal;
    if (!return_Flg) ++Pc;
    break;

  case Break:
    code = nextCode(); post_if_set(break_Flg);            //  If it has a "?", process
    if (!break_Flg) ++Pc;
    break;

  case Exit:
    code = nextCode(); exit_Flg = true;
    break;

  case Option: case Var: case EofLine:                    //  Ignored at Runtime
    ++Pc;
    break;

  default:
    err_exit("Incorrect description: ", kind_to_s(code.kind));
  }
}


/* Execute the statement up to the end of the block */
void block() {
  TknKind k;
  while (!break_Flg && !return_Flg && !exit_Flg) {    //  End at "break", "return", "exit" 
    k = lookCode(Pc);                                 //  Next Initial Code
    if (k==Elif || k==Else || k==End) break;          //  Block Successful Completion
    statement();
  }
}


/* Set the following default arguments in the function declaration */
//double get_expression(int kind1 = 0, int kind2 = 0)
double get_expression(int kind1, int kind2) {   //  Expression to return the result
  expression(kind1, kind2); return stk.pop();
}


/* Expression with Checking */
void expression(int kind1, int kind2) {
  if (kind1 != 0) code = chk_nextCode(code, kind1);
  expression();
  if (kind2 != 0) code = chk_nextCode(code, kind2);
}


/* Expression */
void expression() {
  term(1);
}


/* n is the Order of Priority */
void term(int n) {
  TknKind op;
  if (n == 7) { factor(); return; }
  term(n + 1);
  while (n == opOrder(code.kind)) {   //  Followed by Operends of Equal Strength
    op = code.kind;
    code = nextCode(); term(n+1);
    if (syntaxChk_mode) { stk.pop(); stk.pop(); stk.push(1.0); }  //  When Syntax Checking
    else binaryExpr(op);
  }
}


/* Factor */
void factor() {                                                   //  When Syntax Checking
  TknKind kd = code.kind;

  if (syntaxChk_mode) {
    switch (kd) {
    case Not: case Minus: case Plus:
        code = nextCode(); factor(); stk.pop(); stk.push(1.0);
        break;
    case Lparen:
        expression('(', ')');
        break;
    case IntNum: case DblNum:
        stk.push(1.0); code = nextCode();
        break;
    case Gvar: case Lvar:
        (void)get_memAdrs(code); stk.push(1.0);
        break;
    case Toint: case Input:
        sysFncExec_syntax(kd);
        break;
    case Fcall:
        fncCall_syntax(code.symNbr);
        break;
    case EofLine:
        err_exit("Incorrect Expression");
    default:
        err_exit("Expression Error:", kind_to_s(code));         //  Occurs for a, +, =, etc...
    }
    return;
  }

  /* Runtime */
  switch (kd) { 
  case Not: case Minus: case Plus:
      code = nextCode(); factor();              //  Get next value and ...
      if (kd == Not)   stk.push(!stk.pop());    //  ! Processing
      if (kd == Minus) stk.push(-stk.pop());    //  - Processing
      break;                                    //  If unary +, do nothing
  case Lparen:
      expression('(', ')');
      break;
  case IntNum: case DblNum:
      stk.push(code.dblVal); code = nextCode();
      break;
  case Gvar: case Lvar:
      chk_dtTyp(code);                          //  Check if the variable has been set to a value
      stk.push(Dmem.get(get_memAdrs(code)));
      break;
  case Toint: case Input:
      sysFncExec(kd);
      break;
  case Fcall:
      fncCall(code.symNbr);

      break;
  }
}


/* Precedence of Binary Operands */
int opOrder(TknKind kd) {
    switch (kd) {
    case Multi: case Divi: case Mod:
    case IntDivi:                    return 6;    //  * / % ¥
    case Plus:  case Minus:          return 5;    //  + -
    case Less:  case LessEq:
    case Great: case GreatEq:        return 4;    //  < <=  > >=
    case Equal: case NotEq:          return 3;    //  ==  !=
    case And:                        return 2;    //  &&
    case Or:                         return 1;    //  ||
    default:                         return 0;    //  Not applicable
    }
}


/* Binary Expression */
void binaryExpr(TknKind op) {
  double d = 0, d2 = stk.pop(), d1 = stk.pop();

  if ((op == Divi || op == Mod || op == IntDivi) && d2==0)
    err_exit("Division by Zero");

  switch (op) {
  case Plus:    d = d1 + d2;  break;
  case Minus:   d = d1 - d2;  break;
  case Multi:   d = d1 * d2;  break;
  case Divi:    d = d1 / d2;  break;
  case Mod:     d = (int)d1 % (int)d2; break;
  case IntDivi: d = (int)d1 / (int)d2; break;
  case Less:    d = d1 <  d2; break;
  case LessEq:  d = d1 <= d2; break;
  case Great:   d = d1 >  d2; break;
  case GreatEq: d = d1 >= d2; break;
  case Equal:   d = d1 == d2; break;
  case NotEq:   d = d1 != d2; break;
  case And:     d = d1 && d2; break;
  case Or:      d = d1 || d2; break;
  }
  stk.push(d);
}


/* ? Expression */
void post_if_set(bool& flg) {
  if (code.kind == EofLine) { flg = true; return; }   //  TRUE for flg if there is no ?
  if (get_expression('?', 0)) flg = true;             //  Processing with conditional expressions
}


/* Checking Function Call */
void fncCall_syntax(int fncNbr) {
  int argCt = 0;

  code = nextCode(); code = chk_nextCode(code, '(');
  if (code.kind != ')') {                       //  There are Arguments
    for (;; code=nextCode()) {
      (void)get_expression(); ++argCt;          //  Argument expression processing and argument count
      if (code.kind != ',') break;              //  If there is a "," , argument follows.
    }
  }
  code = chk_nextCode(code, ')');               //  It should be ")"
  if (argCt != Gtable[fncNbr].args)             //  Checking the number of arguments
    err_exit(Gtable[fncNbr].name, "The number of arguments for the function is wrong.");
  stk.push(1.0);                                //  Push Safety Value
}


/* Function Call */
void fncCall(int fncNbr) {
  int  n, argCt = 0;
  vector<double> vc;

  nextCode(); code = nextCode();    //  Function name   ( Skipping
  if (code.kind != ')') {           //  There are Arguments
    for (;; code=nextCode()) {
      expression(); ++argCt;        //  Argument expression processing and argument count
      if (code.kind != ',') break;  //  If there is a "," , argument follows.
    }
  }
  code = nextCode();                //  ) Skipping

  //  Change of Arguments Loading Order
  for (n = 0; n < argCt; n++) vc.push_back(stk.pop());  //  Correct to load arguments from the back
  for (n = 0; n < argCt; n++) { stk.push(vc[n]); }

  fncExec(fncNbr);                                      //  Function Executes
}


/* Function Executes */
void fncExec(int fncNbr) {

  //  Function Entrance Processing
  int save_Pc         = Pc;         //  Store the current execution line
  int save_baseReg    = baseReg;    //  Store the current baseReg
  int save_spReg      = spReg;      //  Store the current spReg
  char *save_code_ptr = code_ptr;   //  Store the current execution line analysis pointer
  CodeSet save_code   = code;       //  Store the current code

  Pc = Gtable[fncNbr].adrs;         //  Set the new Pc
  baseReg = spReg;                  //  Set the new baseReg
  spReg += Gtable[fncNbr].frame;    //  Secure the frame
  Dmem.auto_resize(spReg);          //  Secure the effective area of main memory
  returnValue = 1.0;                //  Return ruled Value
  code = firstCode(Pc);             //  Get initial code

  nextCode(); code = nextCode();                //  Func  ( Skipping
  if (code.kind != ')') {                       //  There are arguments
    for (;; code=nextCode()) {
      set_dtTyp(code, DBL_T);                   //  Determine the type when assigning
      Dmem.set(get_memAdrs(code), stk.pop());   //  Storage actual arguments value
      if (code.kind != ',') break;              //  End of arguments
    }
  }
  code = nextCode();                            //  Skipping

  //  Function body processing
  ++Pc; block(); return_Flg = false;            //  Function body processing

  //  Function exit processing
  stk.push(returnValue);          //  Set return value
  Pc       = save_Pc;             //  Restore the environment before the call
  baseReg  = save_baseReg;
  spReg    = save_spReg;
  code_ptr = save_code_ptr;
  code     = save_code;
}


/* Checking Built-in Function */
void sysFncExec_syntax(TknKind kd) {
  switch (kd) {
  case Toint:
      code = nextCode(); (void)get_expression('(', ')');
      stk.push(1.0);
      break;
  case Input:
      code = nextCode();
      code = chk_nextCode(code, '('); code = chk_nextCode(code, ')');
      stk.push(1.0);                                  //  Push Safety Value
      break;
  case Print: case Println:
      do {
        code = nextCode();
        if (code.kind == String) code = nextCode();   //  Check string output
        else (void)get_expression();                  //  Check number output
      } while (code.kind == ',');                     //  If there is "," , parameter follows
      chk_EofLine();
      break;
  }
}


/* Built-in Function Execute */
void sysFncExec(TknKind kd) {
  double d;
  string s;

  switch (kd) {
  case Toint:
      code = nextCode();
      stk.push((int)get_expression('(', ')'));    //  Rounding down of fractions
      break;
  case Input:
      nextCode(); nextCode(); code = nextCode();  //  Input  ( ) Skipping
      getline(cin, s);                            //  Get 1 line
      stk.push(atof(s.c_str()));                  //  Convert to numbers and store
      break; 
  case Print: case Println:
      do {
        code = nextCode();
        if (code.kind == String) {
          cout << code.text; code = nextCode();   //  Output strings
        } else {
          d = get_expression();                   //  Possibility of exit inside the function
          if (!exit_Flg) cout << d;               //  Output numerical number
        }
      } while (code.kind == ',');                 //  If there is "," , parameter follows
      if (kd == Println) cout << endl;            //  If it's println , break the line  

      break;
  }
}


/* Return the address of a simple variable or array element */
int get_memAdrs(const CodeSet& cd) {
  int adr=0, index, len;
  double d;

  adr = get_topAdrs(cd);
  len = tableP(cd)->aryLen;
  code = nextCode();
  if (len == 0) return adr;

  d = get_expression('[', ']');
  if ((int)d != d) err_exit("Specify the index as a number without fractions.");
  if (syntaxChk_mode) return adr;

  index = (int)d;
  if (index < 0 || len <= index)
    err_exit(index, " is outside of index range (index range:0-)", len-1, ")");
  return adr + index; 
}


/* Returns the address of the beginning of the variable (or the beginning of the array in the case of an array). */
int get_topAdrs(const CodeSet& cd) {
  switch (cd.kind) {
  case Gvar: return tableP(cd)->adrs;
  case Lvar: return tableP(cd)->adrs + baseReg;
  default: err_exit("Variable name is required: ", kind_to_s(cd));
  }
  return 0;     //  Not in here
}


/* The end position corresponding to the if statement */
int endline_of_If(int line) {
  CodeSet cd;
  char *save = code_ptr;

  cd = firstCode(line);
  for (;;) {
    line = cd.jmpAdrs;
    cd = firstCode(line);
    if (cd.kind == Elif || cd.kind == Else) continue;
    if (cd.kind == End) break;
  }
  code_ptr = save;    //  Addition of index number
  return line;
}


/* Checking code */
void chk_EofLine() {
  if (code.kind != EofLine) err_exit("Incorrect description: ", kind_to_s(code));
}


/* First code of the line */
TknKind lookCode(int line) {
  return (TknKind)(unsigned char)intercode[line][0];
}


/* Getting code with Checking */
CodeSet chk_nextCode(const CodeSet& cd, int kind2) {
  if (cd.kind != kind2) {
    if (kind2   == EofLine) err_exit("Incorrect description: ", kind_to_s(cd));
    if (cd.kind == EofLine) err_exit(kind_to_s(kind2), " is required");
    err_exit(kind_to_s(kind2) + " is required before " + kind_to_s(cd) + "");
  }
  return nextCode();
}


/* Getting first code */
CodeSet firstCode(int line) {
  code_ptr = intercode[line];
  return nextCode();
}


/* Getting code */
CodeSet nextCode() {
  TknKind kd;
  short int jmpAdrs, tblNbr;

  if (*code_ptr == '\0') return CodeSet(EofLine);
  kd = (TknKind)*UCHAR_P(code_ptr++);
  switch (kd) {
  case Func:
  case While: case For: case If: case Elif: case Else:
      jmpAdrs = *SHORT_P(code_ptr); code_ptr += SHORT_SIZ;
      return CodeSet(kd, -1, jmpAdrs);                        //  Jumping adress
  case String:
      tblNbr = *SHORT_P(code_ptr); code_ptr += SHORT_SIZ;
      return CodeSet(kd, strLITERAL[tblNbr].c_str());         //  String Literal Position
  case IntNum: case DblNum:
      tblNbr = *SHORT_P(code_ptr); code_ptr += SHORT_SIZ;     //  Number Literal Number
      return CodeSet(kd, nbrLITERAL[tblNbr]); 
  case Fcall: case Gvar: case Lvar:
      tblNbr = *SHORT_P(code_ptr); code_ptr += SHORT_SIZ;
      return CodeSet(kd, tblNbr, -1);
  default:                                                    //  Code with no accompanying information
      return CodeSet(kd);
  }
}


/* Checking with Type */
void chk_dtTyp(const CodeSet& cd) {
  if (tableP(cd)->dtTyp == NON_T)
    err_exit("An uninitialized variable has been used: ", kind_to_s(cd));
}

/* Setting Type */
void set_dtTyp(const CodeSet& cd, char typ) {
  int memAdrs = get_topAdrs(cd);
  vector<SymTbl>::iterator p = tableP(cd);

  if (p->dtTyp != NON_T) return;                              //  The type has already been determined
  p->dtTyp = typ;
  if (p->aryLen != 0) {                                       //  If it's an array, initialize the contents to zero
    for (int n=0; n < p->aryLen; n++) { Dmem.set(memAdrs+n, 0); }
  }
}


/* Number Literal */
int set_LITERAL(double d) {
  for (int n = 0; n < (int)nbrLITERAL.size(); n++) {
    if (nbrLITERAL[n] == d) return n;                         //  Return the same index num
  }
  nbrLITERAL.push_back(d);                                    //  Store the Number Literal
  return nbrLITERAL.size() - 1;                               //  Index position of stored Number Literal
}


/* String Literal */
int set_LITERAL(const string& s) {  
  for (int n = 0; n < (int)strLITERAL.size(); n++) {
    if (strLITERAL[n] == s) return n;
  }
  strLITERAL.push_back(s);          //  Store String Literal
  return strLITERAL.size() - 1;     //  Index Position of Stored String Literal
}
