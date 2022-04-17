/********************************************************************************************************
 *
 *      PROJECT NAME    :   ASMI Demo Contest 2022 
 *
 *      FILE NAME       :   peri_pars.cpp
 *
 *      OUTLINE         :   Parsing
 *
 *      REQUIRED FILES  :   peri.h, peri_prot.h
 *
 *      EDITOR : Taichi KATO,   Advanced Sensing & Machine Intelligence Group,  Chukyo Univ.
 *
 *      LAST UPDATED : Feb. 17, 2022
 *
 *      Copyright © 2022 Taichi KATO. All rights reserved.
 *
*********************************************************************************************************/
/* Header File */
#include "peri.h"
#include "peri_prot.h"


/* Define */
#define NO_FIX_ADRS 0                  //  Mark that still undecided about the address
Token token;                           //  Token currently being processed
SymTbl tmpTb;                          //  Temporary storage symbol table
int blkNest;                           //  Depth of block
int localAdrs;                         //  Local variable address administration
int mainTblNbr;                        //  Location of the symbol table for the main function, if any
int loopNest;                          //  Loop nest
bool fncDecl_F;                        //  TRUE if function definition is being processed
bool explicit_F;                       //  If TRUE, force the variable declaration
char codebuf[LIN_SIZ+1], *codebuf_p;   //  For internally generated code work
extern vector<char*> intercode;        //  Converted internal code storage


/* Initial value setting */
void init() {
  initChTyp();  //  character class table
  mainTblNbr = -1;
  blkNest = loopNest = 0;
  fncDecl_F = explicit_F = false;
  codebuf_p = codebuf;
}


/* Code conversion */
void convert_to_internalCode(char *fname) {
  
  init();  //  Initialize the character type table, etc.

  /* Register only the function definition name first */
  fileOpen(fname);
  while (token=nextLine_tkn(), token.kind != EofProg) {
    if (token.kind == Func) {
      token = nextTkn(); set_name(); enter(tmpTb, fncId);
    }
  }

  /* Conversion to internal code */
  push_intercode();   //  Line 0 is unnecessary and should be buried.
  fileOpen(fname);
  token = nextLine_tkn();
  while (token.kind != EofProg) {
    convert();
  }

  /* Set the call code of the main function if there is one. */
  set_startPc(1);                    //  Start execution from line 1.
  if (mainTblNbr != -1) {
    set_startPc(intercode.size());   //  Start execution from main
    setCode(Fcall, mainTblNbr); setCode('('); setCode(')');
    push_intercode();
  }
}

/* Processes codes that appear only at the beginning. 
   The rest of the code is processed by convert_rest(). */
void convert() {

  switch (token.kind) {
  case Option: optionSet(); break;  //  Option setting
  case Var:    varDecl();   break;  //  Variables declaration
  case Func:   fncDecl();   break;  //  Definition of function
  case While: case For:
        ++loopNest;
        convert_block_set(); setCode_End();
        --loopNest;
        break;
  case If:
        convert_block_set();                                //  if
        while (token.kind == Elif) { convert_block_set(); } //  elif
        if (token.kind == Else)    { convert_block_set(); } //  else
        setCode_End();                                      //  end
        break;
  case Break:
        if (loopNest <= 0) err_exit("Incorrect 'break' error.");
        setCode(token.kind); token = nextTkn(); convert_rest();
        break;
  case Return:
        if (!fncDecl_F) err_exit("Incorrect 'return' error.");
        setCode(token.kind); token = nextTkn(); convert_rest();
        break;
  case Exit:
        setCode(token.kind); token = nextTkn(); convert_rest();
        break;
  case Print: case Println:
        setCode(token.kind); token = nextTkn(); convert_rest();
        break;
  case End:
       err_exit("Incorrect 'end' error.");  //  'end' is never used by itself.
        break;
  default: convert_rest(); break;
  }
}


/* Block process management */
void convert_block_set() {

  int patch_line;

  patch_line = setCode(token.kind, NO_FIX_ADRS); token = nextTkn();
  convert_rest();
  convert_block();                      //  Block processing
  backPatch(patch_line, get_lineNo());  //  Fix NO_FIX_ADRS (end line number)　
}


/* Block Processing */
void convert_block() {

  TknKind k;
  ++blkNest;    //  Parse the sentence to the end of the block  
  while(k=token.kind, k!=Elif && k!=Else && k!=End && k!=EofProg) {
    convert();
  }
  --blkNest;
}


/* Processing the rest of the statement */
void convert_rest() {
  int tblNbr;

  for (;;) {
    if (token.kind == EofLine) break;
    /* These keywords do not appear in the text. */
    switch (token.kind) {
    case If: case Elif: case Else: case For: case While: case Break:
    case Func:  case Return:  case Exit:  case Print:  case Println:
    case Option: case Var: case End:
        err_exit("Incorrect description error.", token.text);
        break;
    case Ident:
        set_name();                                         //  Function call, variables 
        if ((tblNbr=searchName(tmpTb.name, 'F')) != -1) {   //  If the function is registered
          if (tmpTb.name == "main") err_exit("main function cannot be called.");
          setCode(Fcall, tblNbr); continue;
        }
        if ((tblNbr=searchName(tmpTb.name, 'V')) == -1) {   //  If the variable is not registered 
          if (explicit_F) err_exit("Variable declaration is required : ", tmpTb.name);
          tblNbr = enter(tmpTb, varId);                     //  Auto variable registration  
        }
        if (is_localName(tmpTb.name, varId)) setCode(Lvar, tblNbr);
        else                                 setCode(Gvar, tblNbr);
        continue;
    case IntNum: case DblNum:                               //  Integers are also stored as double type 
        setCode(token.kind, set_LITERAL(token.dblVal));
        break;
    case String:
        setCode(token.kind, set_LITERAL(token.text));
        break;
    default:
        setCode(token.kind);                                //  * + - / < <= > >=   etc... //
        break;
    }
    token = nextTkn();
  }
  push_intercode();
  token = nextLine_tkn();
}


/* Option Setting */
void optionSet() {

  setCode(Option);    //  This line is non-executable, so the only code translation is Option  
  setCode_rest();     //  Store the rest in the original 
  token = nextTkn();  //  Force variable declaration
  if (token.kind==String && token.text=="var") explicit_F = true;
  else err_exit("The Option specification is incorrect.");
  token = nextTkn();
  setCode_EofLine();
}


/* Declaring variables that use var */
void varDecl() {
  setCode(Var);                     //  This line is non-executable, so the only code translation is Var
  setCode_rest();                   //  Store the rest in the original
  for (;;) {
    token = nextTkn();
    var_namechk(token);             //  Name checking
    set_name(); set_aryLen();       //  For arrays, set the length  
    enter(tmpTb, varId);            //  Variable registration, address setting  
    if (token.kind != ',') break;   //  End of declarations
  }
  setCode_EofLine();
}


/* Name Checking */
void var_namechk(const Token& tk) {
  if (tk.kind != Ident) err_exit(err_msg(tk.text, "Identifier"));
  if (is_localScope() && tk.text[0] == '$')
    err_exit("Names with $ cannot be specified in var declarations in functions : ", tk.text);
  if (searchName(tk.text, 'V') != -1)
    err_exit("Identifiers are duplicated : ", tk.text);
}


/* Name Setting */
void set_name() {
  if (token.kind != Ident) err_exit("Identifier required : ", token.text);
  tmpTb.clear(); tmpTb.name = token.text;   //  Name setting
  token = nextTkn();
}


/* Length of Array Setting */
void set_aryLen() {
  tmpTb.aryLen = 0;
  if (token.kind != '[') return;  //  Not Array

  token = nextTkn();
  if (token.kind != IntNum)
    err_exit("The array length should be specified as a positive integer constant : ", token.text);
  tmpTb.aryLen = (int)token.dblVal + 1;   //var a[5] is +1 since subscripts 0~5 are valid  
  token = chk_nextTkn(nextTkn(), ']');
  if (token.kind == '[') err_exit("Multi-dimensional arrays cannot be declared.");
}


/* Function Def */
void fncDecl() {
  extern vector<SymTbl> Gtable;             //  Global symbol table  
  int tblNbr, patch_line, fncTblNbr;

  if(blkNest > 0) err_exit("The position of the function definition is incorrect.");
  fncDecl_F = true;                         //  Function processing start flag  
  localAdrs = 0;                            //  Local area allocation counter initialization  
  set_startLtable();                        //  Local symbol table start position  
  patch_line = setCode(Func, NO_FIX_ADRS);  //  The end line number will be stored later  
  token = nextTkn();

  fncTblNbr = searchName(token.text, 'F');  //  Function names are registered at the beginning
  Gtable[fncTblNbr].dtTyp = DBL_T;          //  Function type is fixed to double  


  token = nextTkn();                        //  Dummy argument analysis
  token = chk_nextTkn(token, '(');          //  It should be "("      
  setCode('(');
  if (token.kind != ')') {                  //  There are arguments 
    for (;; token=nextTkn()) {
      set_name();
      tblNbr = enter(tmpTb, paraId);        //  Arguments registration  
      setCode(Lvar, tblNbr);                //  Arguments are processed as Lvar  
      ++Gtable[fncTblNbr].args;             //  Increase the number of arguments by 1
      if (token.kind != ',') break;         //  End of declarations
      setCode(',');
    }
  }
  token = chk_nextTkn(token, ')');          //  It should be ")"    
  setCode(')'); setCode_EofLine();
  convert_block();                          //  Function body processing  

  backPatch(patch_line, get_lineNo());
  setCode_End();
  Gtable[fncTblNbr].frame = localAdrs;      //  Frame size

  if (Gtable[fncTblNbr].name == "main") {   //  main function processing
    mainTblNbr = fncTblNbr;
    if (Gtable[mainTblNbr].args != 0)
      err_exit("Temporary arguments cannot be specified in main function.");
  }
  fncDecl_F = false;                        //  End of functin processing 
}


/* Set the n for "line" */
void backPatch(int line, int n) {
  *SHORT_P(intercode[line] + 1) = (short)n;
}


/* Store code */
void setCode(int cd) {
  *codebuf_p++ = (char)cd;
}


/* Store code & SHORT value */
int setCode(int cd, int nbr) {
  *codebuf_p++ = (char)cd;
  *SHORT_P(codebuf_p) = (short)nbr; codebuf_p += SHORT_SIZ;
  return get_lineNo();    //  Return the storing line for "backpatch"
}


/* Store the rest of the text as is */
void setCode_rest() {
  extern char *token_p;
  strcpy(codebuf_p, token_p);
  codebuf_p += strlen(token_p) + 1;
}


/* Storing "end" processing */
void setCode_End() {
  if (token.kind != End) err_exit(err_msg(token.text, "end"));
  setCode(End); token = nextTkn(); setCode_EofLine();
}


/* Final storing processing */
void setCode_EofLine() {
  if (token.kind != EofLine) err_exit("Incorrect description : ", token.text);
  push_intercode();
  token = nextLine_tkn();
}


/* Storing the converted internal code */
void push_intercode() {
  int len;
  char *p;

  *codebuf_p++ = '\0';
  if ((len = codebuf_p-codebuf) >= LIN_SIZ)
    err_exit("The converted internal code is incorrect. Please shorten the expression.");

  try {
    p = new char[len];                  //  Securing memory
    memcpy(p, codebuf, len);
    intercode.push_back(p);
  }
  catch (bad_alloc) { err_exit("Unable to allocate memory."); }
  codebuf_p = codebuf;                  //  Positioning at the beginning of the storage location for the next process.
}


/* TRUE if the function is in process */
bool is_localScope(){
  return fncDecl_F;
}

