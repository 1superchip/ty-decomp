
#ifdef __cplusplus
extern "C" {
#endif

void __init_cpp_exceptions(void);
extern void __fini_cpp_exceptions(void);
extern void suspend(void);

#ifdef __cplusplus
}
#endif

#pragma force_active on
__declspec(section
           ".ctors") static void* const __init_cpp_exceptions_reference = __init_cpp_exceptions;

// extern void* __init_cpp_exceptions_reference;