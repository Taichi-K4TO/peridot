/********************************************************************************************************
 *
 *      PROJECT NAME    :   ASMI Demo Contest 2022 
 *
 *      FILE NAME       :   peri_tbl.cpp
 *
 *      OUTLINE         :   Symbolic Table Processing
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

/* Table */
vector<SymTbl> Gtable;   //  Global Symbols Table
vector<SymTbl> Ltable;   //  Local  Symbols Table
int startLtable;         //  Start position for LST


/* Registration of symbol table */
int enter(SymTbl& tb, SymKind kind) {
  int n, mem_size;
  bool isLocal = is_localName(tb.name, kind);
  extern int localAdrs;   //  Managements adress of local variables
  extern Mymemory Dmem;   //  Main memory


  //  Checking
  mem_size = tb.aryLen;
  if (mem_size == 0) mem_size = 1;          // In the case of simple variavle
  if (kind != varId && tb.name[0] == '$')   // In the case of using "$"
    err_exit("You cannot use $ except for variables name : ", tb.name);
  tb.nmKind = kind;
  n = -1;                 //  Duplicate checking 
  if (kind == fncId)  n = searchName(tb.name, 'G');
  if (kind == paraId) n = searchName(tb.name, 'L');
  if (n != -1) err_exit(" : ", tb.name);

  //  Setting adress
  if (kind == fncId) tb.adrs = get_lineNo();                      // Line that function starts
  else {
    if (isLocal) { tb.adrs = localAdrs; localAdrs += mem_size; }  //  Local
    else {
      tb.adrs = Dmem.size();                                      //  Global
      Dmem.resize(Dmem.size() + mem_size);                        //  Global area secure
    }
  }

  if (isLocal) { n = Ltable.size(); Ltable.push_back(tb); }       //  Local
  else         { n = Gtable.size(); Gtable.push_back(tb); }       //  Global
  return n;                                                       //  Location of Registrated
}


/* Start of Local Symbols Table */
void set_startLtable() {
  startLtable = Ltable.size();
}


/* TRUE if it's local name */
bool is_localName(const string& name, SymKind kind) {
  if (kind == paraId) return true;
  if (kind == varId) {
    if (is_localScope() && name[0]!='$') return true; else return false;
  }
  return false;         //  fncID
}


/* Searching name */
int searchName(const string& s, int mode) {
  int n;
  switch (mode) {
  case 'G':                                             //  Searching in Global Symbols Table
      for (n = 0; n < (int)Gtable.size(); n++) {
        if (Gtable[n].name == s) return n;
        }
      break;
  case 'L':                                             //  Searching in Local Symbols Table
      for (n = startLtable; n < (int)Ltable.size(); n++) {
        if (Ltable[n].name == s) return n;
      }
      break;
  case 'F':                                             //  Searching in Function Name 
      n = searchName(s, 'G');
      if (n != -1 && Gtable[n].nmKind == fncId) return n;
      break;
  case 'V':                                             //  Searching in Variables Name
      if (searchName(s, 'F') != -1) err_exit("This is a duplicate of the function name : ", s);
      if (s[0] == '$')     return searchName(s, 'G');
      if (is_localScope()) return searchName(s, 'L');   //  in Processing in the Local 
      else                 return searchName(s, 'G');   //  in Processing in the Global
  }
  return -1;                                            //  Cannot Find...
}

vector<SymTbl>::iterator tableP(const CodeSet& cd) {
  if (cd.kind == Lvar) return Ltable.begin() + cd.symNbr;             /* Lvar */
  return Gtable.begin() + cd.symNbr;                            /* Gvar Fcall */
}
