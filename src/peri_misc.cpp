/********************************************************************************************************
 *
 *      PROJECT NAME    :   ASMI Demo Contest 2022 
 *
 *      FILE NAME       :   peri_misc.cpp
 *
 *      OUTLINE         :   Error Output
 *
 *      REQUIRED FILES  :   peri_misc.cpp
 *
 *      EDITOR : Taichi KATO,   Advanced Sensing & Machine Intelligence Group,  Chukyo Univ.
 *
 *      LAST UPDATED : Apl. 17, 2022
 *
 *      Copyright © 2022 Taichi KATO. All rights reserved.
 *
*********************************************************************************************************/
#include "peri.h"
#include "peri_prot.h"


/* Number to String */
string dbl_to_s(double d) {
  ostringstream ostr;         //  Secure Output Stream
  ostr << d;                  //  Write to the Output Stream
  return ostr.str();          //  Passes the buffer contents
}


/* Generate Error Structure */
string err_msg(const string& a, const string& b) {
  if (a == "") return b + " is required.";
  if (b == "") return a + " is incorrect.";
  return b + " is required before " + a + "";
}


/* Specify the following default arguments in the function declaration */
/* void err_exit(Tobj a = "¥1", Tobj b = "¥1", Tobj c = "¥1", Tobj d = "¥1") */
/* Display Error */
void err_exit(Tobj a, Tobj b, Tobj c, Tobj d) {
  Tobj ob[5];
  ob[1] = a; ob[2] = b; ob[3] = c; ob[4] = d;
  cerr << "line:" << get_lineNo() << " ERROR ";

  for (int i=1; i<=4 && ob[i].s!="\1"; i++) {
    if (ob[i].type == 'd') cout << ob[i].d;     //  Numerical Value Information
    if (ob[i].type == 's') cout << ob[i].s;     //  String Information
  }
  cout << endl;
  exit(1);
}
