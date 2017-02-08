#ifndef PRAGMAS_H_DEFINED
#define PRAGMAS_H_DEFINED
#ifdef pp_INTEL
#pragma warning (disable:695)  // calling convention if ignored for this type 
#pragma warning (disable:2557) // comparison between signed and unsigned operands 
#pragma warning (disable:869)  // parameter was never referenced 
#pragma warning (disable:1418) // external function definition with no prior declaration
#pragma warning (disable:3457) // conversion from a string literal to "char *" is deprecated 
#pragma warning (disable:981)  // operands are evaluated in unspecified order
#pragma warning (disable:1572)  // floating-point equality and inequality comparisons are unreliable
#pragma warning (disable:2259)  // non-pointer conversion from "double" to "float" may lose significant bits
#endif
#endif
