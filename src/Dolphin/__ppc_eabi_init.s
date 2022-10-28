#needed to decompile files with ctors
# this is __init_cpp

.include "macros.inc"

.global func_801dc058
func_801dc058:
/* 801DC058 7C0802A6 */ mflr        r0
/* 801DC05C 90010004 */ stw         r0, 4(r1)
/* 801DC060 9421FFF0 */ stwu        r1, -0x10(r1)
/* 801DC064 93E1000C */ stw         r31, 0xc(r1)
/* 801DC068 3C608024 */ lis         r3, _ctors@ha
/* 801DC06C 3803C780 */ addi        r0, r3, _ctors@l
/* 801DC070 7C1F0378 */ mr          r31, r0
/* 801DC074 48000004 */ b           lbl_801dc078
lbl_801dc078:
/* 801DC078 48000004 */ b           lbl_801dc07c
lbl_801dc07c:
/* 801DC07C 48000010 */ b           lbl_801dc08c
lbl_801dc080:
/* 801DC080 7D8803A6 */ mtlr        r12
/* 801DC084 4E800021 */ blrl        
/* 801DC088 3BFF0004 */ addi        r31, r31, 0x4
lbl_801dc08c:
/* 801DC08C 819F0000 */ lwz         r12, 0(r31)
/* 801DC090 280C0000 */ cmplwi      r12, 0
/* 801DC094 4082FFEC */ bne+        lbl_801dc080
/* 801DC098 80010014 */ lwz         r0, 0x14(r1)
/* 801DC09C 83E1000C */ lwz         r31, 0xc(r1)
/* 801DC0A0 38210010 */ addi        r1, r1, 0x10
/* 801DC0A4 7C0803A6 */ mtlr        r0
/* 801DC0A8 4E800020 */ blr         