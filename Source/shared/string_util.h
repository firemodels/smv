#ifndef STRING_UTIL_H_DEFINED
#define STRING_UTIL_H_DEFINED

#ifdef IN_STRING_UTIL
#define STREXTERN
#define STRDECL(var,val)  var=val
#else
#define STREXTERN extern CCC
#define STRDECL(var,val)  var
#endif
#include "stdio_buffer.h"

// vvvvvvvvvvvvvvvvvvvvvvvv header files vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

#include <stdio.h>
#include "options.h"
#include "file_util.h"

// vvvvvvvvvvvvvvvvvvvvvvvv structures vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

/* --------------------------  flowlabels ------------------------------------ */

typedef struct {
  char *longlabel, *shortlabel, *unit;
} flowlabels;

// vvvvvvvvvvvvvvvvvvvvvvvv preprocessing directives vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

#define _A(String1,String2) AppendString((char *)String1,(char *)String2)

#define MATCH 1
#define NOTMATCH 0

#define LABEL_OK  0
#define LABEL_EOF 1
#define LABEL_ERR 2

// avoid compiler warnings on C11 and later
#ifdef pp_C11

#ifdef pp_OSX_HIGHRES
// 90 - 256
#define DEG_SYMBOL -160
#else
// 176 - 256
#define DEG_SYMBOL -80
#endif

#else

#ifdef pp_OSX_HIGHRES
#define DEG_SYMBOL 96
#else
#define DEG_SYMBOL 176
#endif

#endif

#ifdef pp_HASH
#define HASH_NONE   0
#define HASH_MD5    1
#define HASH_SHA1   2
#define HASH_SHA256 3
#define HASH_ALL    4
#endif
#define HELP_SUMMARY 1
#define HELP_ALL 2

#ifndef pp_COMPVER
#if defined(__VERSION__) && defined(__GNUC__) && !defined(__clang__)
#define pp_COMPVER "GCC " __VERSION__
#elif defined(__VERSION__)
#define pp_COMPVER __VERSION__
#elif defined(__VERSION)
#define pp_COMPVER __VERSION
#elif defined(_MSC_VER)
// These macros are to convert the MSVC version number to a string
#define xstr(s) str(s)
#define str(s) #s
#define pp_COMPVER "MSVC " xstr(_MSC_VER)
#else
#define pp_COMPVER "unknown"
#endif
#endif

typedef struct {
  int show_help;
  int show_version;
  int first_arg;
#ifdef pp_HASH
  int hash_option;
#endif
} common_opts;

// vvvvvvvvvvvvvvvvvvvvvvvv headers vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
#if defined(_WIN32) && defined(pp_UNICODE_PATHS)
EXTERNCPP wchar_t *convert_utf8_to_utf16(const char *path);
EXTERNCPP char *convert_utf16_to_utf8(const wchar_t *path);
#endif
EXTERNCPP char          *ConcatLabels(char *label1, char *label2, char *label3, char *label);
EXTERNCPP char          *GetCharPtr(char *label);
EXTERNCPP char          *GetStringPtr(char *buffer);
EXTERNCPP char          *GetStringPtr(char *buffer);
EXTERNCPP char          *GetFloatLabel(float val, char *label);
EXTERNCPP char          *GetIntLabel(int val, char *label);
EXTERNCPP char          *AppendString(const char *S1, const char *S2);
EXTERNCPP void           UsageCommon(int option);
EXTERNCPP common_opts    ParseCommonOptions(int argc, char **argv);
EXTERNCPP void           InitRandAB(int size);
EXTERNCPP float          RandAB(int seed, float minval, float maxval);
EXTERNCPP void           ToLower(char *string);
EXTERNCPP char          *STRCHRR(char *strbeg, char *searchbeg, int c);
EXTERNCPP unsigned int   DiffDate(char *token, char *tokenbase);
EXTERNCPP unsigned int   Time2Sec(char *tokenorig);
EXTERNCPP unsigned int   Date2Sec(char *tokenorig);
EXTERNCPP unsigned int   Date2Sec2(char *tokenorig);
EXTERNCPP unsigned int   Date2Day(char *tokenorig);
EXTERNCPP int            SetLabels(flowlabels *flowlabel, char *longlabel, char *shortlabel, char *unit);
EXTERNCPP int            SetLabelsIso(flowlabels *flowlabel, char *longlabel, char *shortlabel, char *unit, float *levels, int nlevels);
EXTERNCPP int            ReadLabelsBNDS(flowlabels *flowlabel, BFILE *stream, char *bufferD, char *bufferE, char *bufferF, char *suffix_label);
EXTERNCPP int            ReadLabels(flowlabels *flow_label, BFILE *stream, char *suffix_label);
EXTERNCPP int            AppendLabels(flowlabels *flow_label, char *suffix_label);

#ifdef pp_HASH
EXTERNCPP unsigned char *GetHashMD5(char *file);
EXTERNCPP unsigned char *GetHashSHA256(char *file);
EXTERNCPP unsigned char *GetHashSHA1(char *file);
#endif
EXTERNCPP int            MatchWild(char *pTameText, char *pWildText);
EXTERNCPP int            Match(char *buffer, const char *key);
EXTERNCPP int            MatchINI(char *buffer, const char *key);
EXTERNCPP int            MatchSMV(char *buffer, const char *key);
EXTERNCPP int            MatchUpper(char *buffer, const char *key);
EXTERNCPP int            MatchSSF(char *buffer, const char *key);
EXTERNCPP int            RandInt(int min, int max);
EXTERNCPP void           FParseCSV(char *buffer, float *vals, int *valids, int ncols, int *ntokens);
EXTERNCPP void           ParseCSV(char *buffer, char *buffer_temp, char **tokens, int *ntokens);
EXTERNCPP void           StripQuotes(char *buffer);
EXTERNCPP void           StripCommas(char *buffer);
EXTERNCPP int            GetRowCols(FILE *stream, int *nrows, int *ncols);

EXTERNCPP void           RemoveComment(char *buffer);
EXTERNCPP void           TrimBack(char *line);
EXTERNCPP void           TrimCommas(char *line);
EXTERNCPP char          *TrimFront(char *line);
EXTERNCPP const char    *TrimFrontConst(const char *line);
EXTERNCPP void           TrimZeros(char *line);
EXTERNCPP char          *TrimFrontZeros(char *line);

EXTERNCPP char           *Val2String(float val, char *string);
EXTERNCPP void           TrimMZeros(char *line);
EXTERNCPP char          *Strstr(char *c, char *key);
EXTERNCPP char          *STRSTR(char *c, const char *key);
EXTERNCPP void           ScaleString(const char *stringfrom, char *stringto, const float *scale);
EXTERNCPP void           ScaleFloat2String(float floatfrom, char *stringto, const float *scale, int ndigits, int fixedpoint_labels);
EXTERNCPP float          ScaleFloat(float floatfrom, const float *scale);
EXTERNCPP void           Num2String(char *string, float tval);
EXTERNCPP void           Float2String(char *string, float tval, int ndecimals, int fixed_point);
EXTERNCPP void           Floats2Strings(char **c_vals, float *vals, int nvals, int ndigits, int fixedpoint_labels, int exponential_labels, int decimal_labels, int zero_pad, char *exp_offset_label);
EXTERNCPP char          *TrimFrontBack(char *buffer);
EXTERNCPP int            STRCMP(const char *s1, const char *s2);
EXTERNCPP char          *GetChid(char *file, char *buffer);
#ifdef pp_GPU
EXTERNCPP int            LogBase2(float xx);
#endif
EXTERNCPP void           Array2String(float *vals, int nvals, char *string);
EXTERNCPP float          GetMantissaExponent(float x, int *exp10);
EXTERNCPP void           GetGitInfo(char *githash, char *gitdate);
EXTERNCPP char          *GetString(char *buffer);
EXTERNCPP char          *Time2TimeLabel(float time, float dt, char *timelabel, int fixed_point);
EXTERNCPP char          *Time2RenderLabel(float time, float dt, float maxtime, char *timelabel);
EXTERNCPP char          *RandStr(char* str, int length);
EXTERNCPP void           GetBaseTitle(char *progname, char *title_base);
EXTERNCPP void           GetTitle(char *progname, char *fulltitle);
#ifdef pp_HASH
EXTERNCPP void           PRINTversion(char *progname, int hash_option);
#else
EXTERNCPP void           PRINTversion(char *progname);
#endif
EXTERNCPP void           EncodeData(unsigned char *buffer, int nbuffer, unsigned char *data, int ndata, int skip, int channel);
EXTERNCPP                unsigned char *DecodeData(unsigned char *buffer, int nbuffer, int *ndataptr, int skip, int channel);
EXTERNCPP                unsigned char *FlipImage(unsigned char *image, int width, int height, int skip);

// vvvvvvvvvvvvvvvvvvvvvvvv variables vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

#ifdef _WIN32
STREXTERN char STRDECL(dirseparator[],"\\");
#else
STREXTERN char STRDECL(dirseparator[],"/");
#endif
#endif
