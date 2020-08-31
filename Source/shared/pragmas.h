#ifndef PRAGMAS_H_DEFINED
#define PRAGMAS_H_DEFINED

#ifdef __INTEL_COMPILER
#ifdef _DEBUG
#pragma float_control( precise, on)
#pragma float_control( except, on )
#endif
#pragma warning (disable:177)
#pragma warning (disable:444)   // destructor for base class
#pragma warning (disable:494)   // omission of "class"is nonstandard
#pragma warning (disable:695)   // calling convention is ignored for this type
#pragma warning (disable:810)   // conversion from xx to yy may lose sig bits
#pragma warning (disable:869)   // parameter was never referenced
#pragma warning (disable:981)   // operands are evaluated in unspecified order
#pragma warning (disable:1418)  // external function definition with no prior declaration
#pragma warning (disable:1419)  // external declaration in primary source file
#pragma warning (disable:1478)
#pragma warning (disable:1572)  // floating-point equality and inequality comparisons are unreliable
#pragma warning (disable:1599)  // declaration hides variable
#pragma warning (disable:1678)  // cannot enable speculation unless fenv_access and exception_semantics are disabled
#pragma warning (disable:1786)
#pragma warning (disable:2259)
#pragma warning (disable:2557)  // comparison between signed and unsigned operands
#pragma warning (disable:3457)  // conversion from a string literal to "char *" is deprecated
#pragma warning (disable:4018)  // signed/unsigned match
#pragma warning (disable:4127)
#pragma warning (disable:4206)  // translation unit empty
#pragma warning (disable:4244)
#pragma warning (disable:4267)  // size_t to int possible loss of data
#pragma warning (disable:4310)
#pragma warning (disable:4389)  // signed/unsigned mis-match
#pragma warning (disable:4701)
#pragma warning (disable:4996)
#endif

#endif
