/********************************************************************************************************
 *
 *      PROJECT NAME    :   ASMI Demo Contest 2022 
 *
 *      FILE NAME       :   peri_tkn.cpp
 *
 *      OUTLINE         :   Token Processing
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


/* Managing lexical and typographical correspondence */
struct KeyWord {
  const char *keyName;  //  if, for, ...
  TknKind keyKind;      //  Corresponding value; TknKind is managed in bbi.h
};


/* Table of correspondence between reserved words and symbols and their types */
KeyWord KeyWdTbl[] = { 
  {"func"   , Func  }, {"var"    , Var    },
  {"if"     , If    }, {"elif"   , Elif   },
  {"else"   , Else  }, {"for"    , For    },
  {"to"     , To    }, {"step"   , Step   },
  {"while"  , While }, {"end"    , End    },
  {"break"  , Break }, {"return" , Return },
  {"print"  , Print }, {"println", Println},
  {"option" , Option}, {"input"  , Input  },
  {"toint"  , Toint }, {"exit"   , Exit   },
  {"("  , Lparen    }, {")"  , Rparen   },
  {"["  , Lbracket  }, {"]"  , Rbracket },
  {"+"  , Plus      }, {"-"  , Minus    },
  {"*"  , Multi     }, {"/"  , Divi     },
  {"==" , Equal     }, {"!=" , NotEq    },
  {"<"  , Less      }, {"<=" , LessEq   },
  {">"  , Great     }, {">=" , GreatEq  },
  {"&&" , And       }, {"||" , Or       },
  {"!"  , Not       }, {"%"  , Mod      },
  {"?"  , Ifsub     }, {"="  , Assign   },
  {"\\" , IntDivi   }, {","  , Comma    },
  {"\"" , DblQ      },
  {"@dummy", END_KeyList},
};


/* Declaration of variables */
int srcLineno;          //  Source line No.
TknKind ctyp[256];      //  Array of character type tables
char *token_p;          //  1 Text acquisition with text location
bool endOfFile_F;       //  End-of-file flag
char buf[LIN_SIZ+5];    //  Location of source loading
ifstream fin;           //  Input Stream


/* Define */
#define MAX_LINE 2000   //  Maximum number of program lines


/* Setting the character type table */
void initChTyp() {

  int i;

  for (i =  0 ; i <  256; i++) { ctyp[i] = Others; }
  for (i = '0'; i <= '9'; i++) { ctyp[i] = Digit;  }
  for (i = 'A'; i <= 'Z'; i++) { ctyp[i] = Letter; }
  for (i = 'a'; i <= 'z'; i++) { ctyp[i] = Letter; }

  ctyp['_']  = Letter;    ctyp['$']  = Doll;
  ctyp['(']  = Lparen;    ctyp[')']  = Rparen;
  ctyp['[']  = Lbracket;  ctyp[']']  = Rbracket;
  ctyp['<']  = Less;      ctyp['>']  = Great;
  ctyp['+']  = Plus;      ctyp['-']  = Minus;
  ctyp['*']  = Multi;     ctyp['/']  = Divi;
  ctyp['!']  = Not;       ctyp['%']  = Mod;
  ctyp['?']  = Ifsub;     ctyp['=']  = Assign;
  ctyp['\\'] = IntDivi;   ctyp[',']  = Comma;
  ctyp['\"'] = DblQ;
  
}


/* Opening File */
void fileOpen(char *fname) {
  srcLineno = 0;
  endOfFile_F = false;
  fin.open(fname);
  if (!fin) { cout << fname << "cannot open.(File)\n"; exit(1); }
}


/* Get next line */
void nextLine() {
  string s;

  if (endOfFile_F) return;
  fin.getline(buf, LIN_SIZ+5);    //  Get a line
  if (fin.eof()) {                //  End of file
    fin.clear(); fin.close();     //  "clear" is ready to re-open
    endOfFile_F = true; return;   
  }

  if (strlen(buf) > LIN_SIZ)
    err_exit("The program should be written within ", LIN_SIZ, " characters per line.");
  if (++srcLineno > MAX_LINE)
    err_exit("The program is over ", MAX_LINE, " lines long.");
  token_p = buf;                  //  Pointer for token analysis is placed at the top of buf
}


/* Reading the next line and returns the next token */
Token nextLine_tkn() {
  nextLine();
  return nextTkn();
}


/* Define */
#define CH (*token_p)
#define C2 (*(token_p+1))
#define NEXT_CH()  ++token_p


/* Next token */
Token nextTkn() {
  TknKind kd;
  string txt = "";

  if (endOfFile_F) return Token(EofProg);            //  End of file
  while (isspace(CH)) NEXT_CH();                     //  Read away the blanks
  if (CH == '\0')  return Token(EofLine);            //  End of line

  switch (ctyp[CH]) {
  case Doll: case Letter:
    txt += CH; NEXT_CH();
    while (ctyp[CH]==Letter || ctyp[CH]==Digit) { txt += CH; NEXT_CH(); }
    break;
  case Digit:                                         //  Numerical constant
    kd = IntNum;
    while (ctyp[CH] == Digit)   { txt += CH; NEXT_CH(); }
    if (CH == '.') { kd = DblNum; txt += CH; NEXT_CH(); }
    while (ctyp[CH] == Digit)   { txt += CH; NEXT_CH(); }
    return Token(kd, txt, atof(txt.c_str()));         //  "IntNum" is also stored as a double type
  case DblQ:                                          //  String constant
    NEXT_CH();
    while (CH!='\0' && CH!='"') { txt += CH; NEXT_CH(); }
    if (CH == '"') NEXT_CH(); else err_exit("String literal is not closed.");
    return Token(String, txt);
  default:
    if (CH=='/' && C2=='/') return Token(EofLine);    //  Comments
    if (is_ope2(CH, C2)) { txt += CH; txt += C2; NEXT_CH(); NEXT_CH(); }
    else                 { txt += CH; NEXT_CH(); }
  }
  kd = get_kind(txt);                                 //  Set the kind

  if (kd == Others) err_exit("Incorrect token : ", txt);
  return Token(kd, txt);
}


/* TRUE if 2 character operand */
bool is_ope2(char c1, char c2) {
  char s[] = "    ";
  if (c1=='\0' || c2=='\0') return false;
  s[1] = c1; s[2] = c2;
  return strstr(" ++ -- <= >= == != && || ", s) != NULL;
}


/* Setting kind of tokens */
TknKind get_kind(const string& s) {
  for (int i=0; KeyWdTbl[i].keyKind != END_KeyList; i++) {
    if (s == KeyWdTbl[i].keyName) return KeyWdTbl[i].keyKind;
  }
  if (ctyp[s[0]]==Letter || ctyp[s[0]]==Doll) return Ident;
  if (ctyp[s[0]] == Digit)  return DblNum;
  return Others;   // nothing ...
}


/* Getting token with confirmation */
Token chk_nextTkn(const Token& tk, int kind2) {
  if (tk.kind != kind2) err_exit(err_msg(tk.text, kind_to_s(kind2)));
  return nextTkn();
}


/* Setting pointer of token processing */
void set_token_p(char *p) {
  token_p = p;
}


/* Kind -> String */
string kind_to_s(int kd) {
  for (int i=0; ; i++) {
    if (KeyWdTbl[i].keyKind == END_KeyList) break;
    if (KeyWdTbl[i].keyKind == kd) return KeyWdTbl[i].keyName;
  }
  return "";
}


/* Kind -> String */
string kind_to_s(const CodeSet& cd) {
  switch (cd.kind) {
  case Lvar: case Gvar: case Fcall: return tableP(cd)->name;
  case IntNum: case DblNum: return dbl_to_s(cd.dblVal);
  case String: return string("\"") + cd.text + "\"";
  case EofLine: return "";
  }
  return kind_to_s(cd.kind);
}


/* Line No. being read or executed */
int get_lineNo() {
  extern int Pc;
  return (Pc == -1) ? srcLineno : Pc;   //  Analyzing : Executing
}

