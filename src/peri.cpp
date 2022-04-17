/********************************************************************************************************
 *
 *      PROJECT NAME    :   ASMI Demo Contest 2022 
 *
 *      FILE NAME       :   peri.cpp
 *
 *      OUTLINE         :   Main Processing
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

/* Main Function */
int main(int argc, char *argv[]) {
  if (argc == 1) { cout << "Usage: peri filename\n"; exit(1); }
  convert_to_internalCode(argv[1]);
  syntaxChk();
  execute();
  return 0;
}

