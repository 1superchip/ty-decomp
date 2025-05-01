#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

// Unknown function declaration

#define UNKNOWN_FUNCTION(name) void name(void)

// Forward declares a function to allow it to be mangled, and adds extern "C"
// to it to prevent any functions it references being mangled
// Should be used for any C++ inline asm includes

#ifdef __cplusplus
    #define MANGLED_ASM(prototype) \
        prototype; \
        extern "C" asm prototype
#endif

// Data dummy helpers

void __dummy_str(const char *);
void __dummy_float(float);
void __dummy_double(double);
void __dummy_pointer(const void *);

// Force a symbol to be stripped by elf2rel/elf2dol

#pragma section RX "forcestrip"
#ifndef __INTELLISENSE__ 
    #define FORCESTRIP __declspec(section "forcestrip")
#else
    #define FORCESTRIP
#endif

// Wrap in force_active pragmas to force a piece of data active
#define DUMMY_POINTER(name) \
    void dummy_ptr_##name(); \
    void FORCESTRIP dummy_ptr_##name() \
    { \
        __dummy_pointer((const void *)&name); \
    }

// Unfortunately these don't work on older compilers

#if __MWERKS__ >= 0x4199

// Disable deadstripping for a region

#define FORCEACTIVE_START _Pragma("push") \
                          _Pragma("force_active on")
#define FORCEACTIVE_END _Pragma("pop")

// Disable deadstripping for a bit of data

#define FORCEACTIVE_DATA(name) \
    FORCEACTIVE_START \
    DUMMY_POINTER(name) \
    FORCEACTIVE_END

#endif

// Rel symbol definition

#pragma section RW "relsymdef"

typedef struct
{
    unsigned long addr;
    const void * ref;
} __RelSymbolDef;

#define REL_SYMBOL_AT(name, addr) \
    __declspec(section "relsymdef") __RelSymbolDef rel_sym_##name = \
    {addr, (const void *)&name}; \
    FORCEACTIVE_DATA(rel_sym_##name)

// BSS ordering hack

#define ORDER_BSS_DATA static asm void order_bss()
#define ORDER_BSS(s) lis r3, s@ha

// Dummy signatures for functions used in inline asm

#define qr0 0

void _savegpr_14(void);
void _savegpr_15(void);
void _savegpr_16(void);
void _savegpr_17(void);
void _savegpr_18(void);
void _savegpr_19(void);
void _savegpr_20(void);
void _savegpr_21(void);
void _savegpr_22(void);
void _savegpr_23(void);
void _savegpr_24(void);
void _savegpr_25(void);
void _savegpr_26(void);
void _savegpr_27(void);

void _restgpr_14(void);
void _restgpr_15(void);
void _restgpr_16(void);
void _restgpr_17(void);
void _restgpr_18(void);
void _restgpr_19(void);
void _restgpr_20(void);
void _restgpr_21(void);
void _restgpr_22(void);
void _restgpr_23(void);
void _restgpr_24(void);
void _restgpr_25(void);
void _restgpr_26(void);
void _restgpr_27(void);

void _savefpr_14(void);
void _savefpr_15(void);
void _savefpr_16(void);
void _savefpr_17(void);
void _savefpr_18(void);
void _savefpr_19(void);
void _savefpr_20(void);
void _savefpr_21(void);
void _savefpr_22(void);
void _savefpr_23(void);
void _savefpr_24(void);
void _savefpr_25(void);
void _savefpr_26(void);
void _savefpr_27(void);

void _restfpr_14(void);
void _restfpr_15(void);
void _restfpr_16(void);
void _restfpr_17(void);
void _restfpr_18(void);
void _restfpr_19(void);
void _restfpr_20(void);
void _restfpr_21(void);
void _restfpr_22(void);
void _restfpr_23(void);
void _restfpr_24(void);
void _restfpr_25(void);
void _restfpr_26(void);
void _restfpr_27(void);

void __div2u(void);
void __div2i(void);
void __mod2u(void);
void __mod2i(void);
void __shl2i(void);

void __cvt_sll_flt(void);
void __cvt_ull_flt(void);
void __cvt_dbl_usll(void);
void __cvt_dbl_ull(void);
void __cvt_fp2unsigned(void);

void __unexpected(void);

#ifdef __cplusplus
    }
#endif
