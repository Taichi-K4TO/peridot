/********************************************************************************************************
 *
 *      PROJECT NAME    :   ASMI Demo Contest 2022 
 *
 *      FILE NAME       :   peri_prot.h
 *
 *      OUTLINE         :   Prototype declarations for all functions.
 *
 *      REQUIRED FILES  :   None
 *
 *      EDITOR : Taichi KATO,   Advanced Sensing & Machine Intelligence Group,  Chukyo Univ.
 *
 *      LAST UPDATED : Apl. 17, 2022
 *
 *      Copyright © 2022 Taichi KATO. All rights reserved.
 *
*********************************************************************************************************/
/* peri_pars.cpp (PARSING) */
void init();
void convert_to_internalCode(char *fname);
void convert();
void convert_block_set();
void convert_block();
void convert_rest();
void optionSet();
void varDecl();
void var_namechk(const Token& tk);
void set_name();
void set_aryLen();
void fncDecl();
void backPatch(int line, int n);
void setCode(int cd);
int setCode(int cd, int nbr);
void setCode_rest();
void setCode_End();
void setCode_EofLine();
void push_intercode();
bool is_localScope();
void DBG_dump_src(char *s);
void DBG_all_prog_disp();


/* peri_tkn.cpp (TOKEN PROCESSING) */
void initChTyp();
void fileOpen(char *fname);
void nextLine();
Token nextLine_tkn();
Token nextTkn();
bool is_ope2(char c1, char c2);
TknKind get_kind(const string& s);
Token chk_nextTkn(const Token& tk, int kind2);
void set_token_p(char *p);
string DBG_kind_to_s(int kd);
string kind_to_s(int kd);
string kind_to_s(const CodeSet& cd);
int get_lineNo();


/* peri_tbl.cpp (TABLE SYMBOL PROCESSING) */
int enter(SymTbl& tb, SymKind kind);
void set_startLtable();
bool is_localName(const string& name, SymKind kind);
int searchName(const string& s, int mode);
vector<SymTbl>::iterator tableP(const CodeSet& cd);
void DBG_disp_Memory();
void DBG_disp_SymTbl();


/* peri_code.cpp (MEMORY MANAGEMENT & SYNTAX CHECKING & EXECUTION) */
void syntaxChk();
void set_startPc(int n);
void execute();
void statement();
void block();
double get_expression(int kind1=0, int kind2=0);
void expression(int kind1, int kind2);
void expression();
void term(int n);
void factor();
int opOrder(TknKind kd);
void binaryExpr(TknKind op);
void post_if_set(bool& flg);
void fncCall_syntax(int fncNbr);
void fncCall(int fncNbr);
void fncExec(int fncNbr);
void sysFncExec_syntax(TknKind kd);
void sysFncExec(TknKind kd);
int get_memAdrs(const CodeSet& cd);
int get_topAdrs(const CodeSet& cd);
int endline_of_If(int line);
void chk_EofLine();
TknKind lookCode(int line);
CodeSet chk_nextCode(const CodeSet& cd, int kind2);
CodeSet firstCode(int line);
CodeSet nextCode();
void chk_dtTyp(const CodeSet& cd);
void set_dtTyp(const CodeSet& cd, char typ);
int set_LITERAL(double d);
int set_LITERAL(const string& s);
void DBG_stk();

/* peri_misc.cpp (ERROR HANDLING) */
string dbl_to_s(double d);
string err_msg(const string& a, const string& b);
void err_exit(Tobj a="\1", Tobj b="\1", Tobj c="\1", Tobj d="\1");
void prt(Tobj a="\1", Tobj b="\1", Tobj c="\1", Tobj d="\1", Tobj e="\1", Tobj f="\1", Tobj g="\1", Tobj h="\1");

