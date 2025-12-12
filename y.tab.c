/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 3 "./parse.y"

/* Some yaccs insist on including stdlib.h */
#include "es.h"
#include "input.h"
#include "syntax.h"

#line 78 "y.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    WORD = 258,                    /* WORD  */
    QWORD = 259,                   /* QWORD  */
    LOCAL = 260,                   /* LOCAL  */
    LET = 261,                     /* LET  */
    FOR = 262,                     /* FOR  */
    CLOSURE = 263,                 /* CLOSURE  */
    FN = 264,                      /* FN  */
    REDIR = 265,                   /* REDIR  */
    DUP = 266,                     /* DUP  */
    ANDAND = 267,                  /* ANDAND  */
    BACKBACK = 268,                /* BACKBACK  */
    BBFLAT = 269,                  /* BBFLAT  */
    BFLAT = 270,                   /* BFLAT  */
    EXTRACT = 271,                 /* EXTRACT  */
    CALL = 272,                    /* CALL  */
    COUNT = 273,                   /* COUNT  */
    FLAT = 274,                    /* FLAT  */
    OROR = 275,                    /* OROR  */
    PRIM = 276,                    /* PRIM  */
    SUB = 277,                     /* SUB  */
    NL = 278,                      /* NL  */
    ENDFILE = 279,                 /* ENDFILE  */
    ERROR = 280,                   /* ERROR  */
    MATCH = 281,                   /* MATCH  */
    PIPE = 282                     /* PIPE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 27 "./parse.y"

	Tree *tree;
	char *str;
	NodeKind kind;

#line 161 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_WORD = 3,                       /* WORD  */
  YYSYMBOL_QWORD = 4,                      /* QWORD  */
  YYSYMBOL_LOCAL = 5,                      /* LOCAL  */
  YYSYMBOL_LET = 6,                        /* LET  */
  YYSYMBOL_FOR = 7,                        /* FOR  */
  YYSYMBOL_CLOSURE = 8,                    /* CLOSURE  */
  YYSYMBOL_FN = 9,                         /* FN  */
  YYSYMBOL_REDIR = 10,                     /* REDIR  */
  YYSYMBOL_DUP = 11,                       /* DUP  */
  YYSYMBOL_ANDAND = 12,                    /* ANDAND  */
  YYSYMBOL_BACKBACK = 13,                  /* BACKBACK  */
  YYSYMBOL_BBFLAT = 14,                    /* BBFLAT  */
  YYSYMBOL_BFLAT = 15,                     /* BFLAT  */
  YYSYMBOL_EXTRACT = 16,                   /* EXTRACT  */
  YYSYMBOL_CALL = 17,                      /* CALL  */
  YYSYMBOL_COUNT = 18,                     /* COUNT  */
  YYSYMBOL_FLAT = 19,                      /* FLAT  */
  YYSYMBOL_OROR = 20,                      /* OROR  */
  YYSYMBOL_PRIM = 21,                      /* PRIM  */
  YYSYMBOL_SUB = 22,                       /* SUB  */
  YYSYMBOL_NL = 23,                        /* NL  */
  YYSYMBOL_ENDFILE = 24,                   /* ENDFILE  */
  YYSYMBOL_ERROR = 25,                     /* ERROR  */
  YYSYMBOL_MATCH = 26,                     /* MATCH  */
  YYSYMBOL_27_ = 27,                       /* '^'  */
  YYSYMBOL_28_ = 28,                       /* '='  */
  YYSYMBOL_29_ = 29,                       /* ')'  */
  YYSYMBOL_30_ = 30,                       /* '!'  */
  YYSYMBOL_PIPE = 31,                      /* PIPE  */
  YYSYMBOL_32_ = 32,                       /* '$'  */
  YYSYMBOL_33_ = 33,                       /* ';'  */
  YYSYMBOL_34_ = 34,                       /* '&'  */
  YYSYMBOL_35_ = 35,                       /* '('  */
  YYSYMBOL_36_ = 36,                       /* '~'  */
  YYSYMBOL_37_ = 37,                       /* '{'  */
  YYSYMBOL_38_ = 38,                       /* '}'  */
  YYSYMBOL_39_ = 39,                       /* '@'  */
  YYSYMBOL_40_ = 40,                       /* '`'  */
  YYSYMBOL_YYACCEPT = 41,                  /* $accept  */
  YYSYMBOL_es = 42,                        /* es  */
  YYSYMBOL_end = 43,                       /* end  */
  YYSYMBOL_line = 44,                      /* line  */
  YYSYMBOL_body = 45,                      /* body  */
  YYSYMBOL_cmdsa = 46,                     /* cmdsa  */
  YYSYMBOL_cmdsan = 47,                    /* cmdsan  */
  YYSYMBOL_cmd = 48,                       /* cmd  */
  YYSYMBOL_cases = 49,                     /* cases  */
  YYSYMBOL_case = 50,                      /* case  */
  YYSYMBOL_simple = 51,                    /* simple  */
  YYSYMBOL_args = 52,                      /* args  */
  YYSYMBOL_redir = 53,                     /* redir  */
  YYSYMBOL_bindings = 54,                  /* bindings  */
  YYSYMBOL_binding = 55,                   /* binding  */
  YYSYMBOL_assign = 56,                    /* assign  */
  YYSYMBOL_fn = 57,                        /* fn  */
  YYSYMBOL_first = 58,                     /* first  */
  YYSYMBOL_sword = 59,                     /* sword  */
  YYSYMBOL_word = 60,                      /* word  */
  YYSYMBOL_comword = 61,                   /* comword  */
  YYSYMBOL_param = 62,                     /* param  */
  YYSYMBOL_params = 63,                    /* params  */
  YYSYMBOL_words = 64,                     /* words  */
  YYSYMBOL_nlwords = 65,                   /* nlwords  */
  YYSYMBOL_nl = 66,                        /* nl  */
  YYSYMBOL_caret = 67,                     /* caret  */
  YYSYMBOL_binder = 68,                    /* binder  */
  YYSYMBOL_keyword = 69                    /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  76
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   605

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  29
/* YYNRULES -- Number of rules.  */
#define YYNRULES  95
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  155

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   282


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    30,     2,     2,    32,     2,    34,     2,
      35,    29,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    33,
       2,    28,     2,     2,    39,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    27,     2,    40,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    37,     2,    38,    36,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    31
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    43,    43,    44,    46,    47,    49,    50,    52,    53,
      55,    56,    58,    59,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    75,    76,    77,
      79,    80,    82,    83,    85,    86,    87,    88,    90,    91,
      93,    94,    95,    97,    98,    99,   101,   103,   104,   106,
     107,   109,   110,   112,   113,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   130,
     131,   133,   134,   136,   137,   139,   140,   141,   143,   144,
     146,   147,   149,   150,   151,   152,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "WORD", "QWORD",
  "LOCAL", "LET", "FOR", "CLOSURE", "FN", "REDIR", "DUP", "ANDAND",
  "BACKBACK", "BBFLAT", "BFLAT", "EXTRACT", "CALL", "COUNT", "FLAT",
  "OROR", "PRIM", "SUB", "NL", "ENDFILE", "ERROR", "MATCH", "'^'", "'='",
  "')'", "'!'", "PIPE", "'$'", "';'", "'&'", "'('", "'~'", "'{'", "'}'",
  "'@'", "'`'", "$accept", "es", "end", "line", "body", "cmdsa", "cmdsan",
  "cmd", "cases", "case", "simple", "args", "redir", "bindings", "binding",
  "assign", "fn", "first", "sword", "word", "comword", "param", "params",
  "words", "nlwords", "nl", "caret", "binder", "keyword", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-83)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-81)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     155,    77,   -83,   -83,   -83,   -83,   -83,   -83,   459,   459,
     -83,   459,   459,   459,   459,   459,   459,   459,     7,   459,
      -6,   459,   -83,   459,   345,   -83,   459,    17,    77,   345,
      57,   -83,   345,   -83,   193,   -83,   -83,   -83,   -83,   -83,
     -83,   -83,   -83,   -83,   -83,   -83,   -83,   -83,   -83,   -83,
     -83,   -83,    24,   -83,   -83,    25,   383,   383,   -83,    25,
     -83,   -83,   -83,   -83,    25,   -83,   345,    36,   231,    25,
      47,   -83,   345,    53,    34,   -83,   -83,   -83,   -83,   -83,
     -83,   -83,   -83,   -83,    67,   497,   269,   -83,   -83,    25,
      75,    33,   459,    46,   -83,   -83,   459,    58,    67,   -83,
     -83,   -83,    25,   459,   -83,   -83,   -83,   345,   -83,   307,
     307,   307,   -83,   -83,    25,    -6,   -83,   535,   -83,   345,
      25,   459,   421,    68,    67,    67,   -83,   -83,   -15,   -83,
     -83,    78,    70,    66,   -83,   565,   -83,   -83,   459,   535,
     -83,   535,   -83,   -83,   459,   -83,   459,    85,   -83,   307,
     -83,   -83,   -83,   459,    43
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,    69,    70,    82,    83,    84,    85,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      80,     0,    75,     0,    14,    71,     0,     0,     0,    14,
       6,    15,    14,    18,    32,    49,    55,    78,     4,     5,
       3,    90,    91,    92,    94,    93,    89,    95,    88,    86,
      87,    53,    48,    51,    52,    39,     0,     0,    66,    73,
      61,    62,    63,    64,    78,    81,    14,    59,     0,    73,
       0,    12,    14,     8,     0,    65,     1,     2,     7,    78,
      78,    78,    10,    11,    16,    81,    33,    35,    17,    34,
       0,     0,     0,     0,    67,    68,    25,     0,    23,    73,
      77,    56,    76,    24,    57,     9,    13,    14,    72,    14,
      14,    14,    50,    37,    36,    80,    79,    43,    54,    14,
      74,    30,     0,     0,    20,    21,    22,    73,     0,    40,
      44,    80,     0,     0,    27,     0,    60,    58,    46,    43,
      78,    43,    45,    47,    30,    26,    30,    31,    42,    14,
      41,    29,    28,     0,    19
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -83,   -83,    76,    87,   -59,    15,   -83,     4,   -83,   -66,
     -83,   -83,   -32,   -83,   -47,   -13,   -77,   -82,   -10,    11,
       0,   -62,    69,   -60,   -83,   -38,   -19,   -83,   -83
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    27,    40,    28,    70,    71,    72,    73,   133,   134,
      31,    86,    32,   128,   129,    88,    33,    34,    51,   120,
      53,    36,    74,    96,    68,    91,    90,    37,    54
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      35,    66,    87,    58,    30,    60,    61,    62,   139,   103,
      63,    67,   108,   105,   140,    29,    75,    76,   141,    52,
      55,    65,    56,    57,    35,    59,    97,   -71,   -71,    35,
      64,   108,    35,    30,    69,   131,    84,     2,     3,   122,
     130,   109,   110,   111,    29,    89,    94,    95,   123,     2,
       3,    92,    92,   147,   113,    79,   116,   131,    99,   131,
     132,   -71,   130,    80,   130,    79,    35,   138,   117,    79,
      98,   107,    35,    80,    81,   112,   106,    80,   151,   102,
     152,   116,   118,   119,    81,   104,    82,    83,    81,   144,
      82,    83,   148,   121,   150,   145,   127,   114,    81,   146,
      38,    39,   149,   115,    77,    85,   137,    35,   143,    35,
      35,    35,   153,   124,   125,   126,    78,    35,   142,    35,
       0,    93,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   135,     0,     0,    35,     0,     0,     0,    35,
       0,    35,     0,   112,     0,     0,     0,     0,     0,    35,
       0,     0,     0,   154,     0,   135,     1,   135,     2,     3,
       4,     5,     6,     7,     8,     9,    10,   -14,    11,    12,
      13,    14,    15,    16,    17,   -14,    18,     0,   -14,   -14,
       0,    19,     0,     0,     0,    20,   -14,    21,   -14,   -14,
      22,    23,    24,     0,    25,    26,     2,     3,    41,    42,
      43,    44,    45,     9,    10,     0,    11,    12,    13,    46,
      15,    16,    17,     0,    18,     0,     0,     0,     0,    47,
      85,   -80,     0,    49,     0,    21,     0,     0,    22,    50,
      24,     0,    25,    26,     2,     3,    41,    42,    43,    44,
      45,     0,     0,     0,    11,    12,    13,    46,    15,    16,
      17,     0,    18,     0,   100,     0,     0,    47,     0,    48,
     101,    49,     0,    21,     0,     0,    22,    50,    24,     0,
      25,    26,     2,     3,    41,    42,    43,    44,    45,     9,
      10,     0,    11,    12,    13,    46,    15,    16,    17,     0,
      18,     0,     0,     0,     0,    47,     0,    48,     0,    49,
       0,    21,     0,     0,    22,    50,    24,     0,    25,    26,
       2,     3,     4,     5,     6,     7,     8,     9,    10,     0,
      11,    12,    13,    14,    15,    16,    17,     0,    18,     0,
     116,     0,     0,    19,     0,     0,     0,    20,     0,    21,
       0,     0,    22,    23,    24,     0,    25,    26,     2,     3,
       4,     5,     6,     7,     8,     9,    10,     0,    11,    12,
      13,    14,    15,    16,    17,     0,    18,     0,     0,     0,
       0,    19,     0,     0,     0,    20,     0,    21,     0,     0,
      22,    23,    24,     0,    25,    26,     2,     3,    41,    42,
      43,    44,    45,     0,     0,     0,    11,    12,    13,    46,
      15,    16,    17,     0,    18,     0,     0,     0,     0,    47,
      92,    48,     0,    49,     0,    21,     0,     0,    22,    50,
      24,     0,    25,    26,     2,     3,    41,    42,    43,    44,
      45,     0,     0,     0,    11,    12,    13,    46,    15,    16,
      17,     0,    18,     0,     0,     0,     0,    47,     0,    48,
     136,    49,     0,    21,     0,     0,    22,    50,    24,     0,
      25,    26,     2,     3,    41,    42,    43,    44,    45,     0,
       0,     0,    11,    12,    13,    46,    15,    16,    17,     0,
      18,     0,     0,     0,     0,    47,     0,    48,     0,    49,
       0,    21,     0,     0,    22,    50,    24,     0,    25,    26,
       2,     3,    41,    42,    43,    44,    45,     0,     0,     0,
      11,    12,    13,    46,    15,    16,    17,     0,    18,     0,
       0,     0,     0,    47,     0,     0,     0,    49,     0,    21,
       0,     0,    22,    50,    24,     0,    25,    26,     2,     3,
       0,     0,     0,     0,     8,     0,     0,     0,    11,    12,
      13,     0,    15,    16,    17,     0,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     2,     3,
      22,     0,    24,     0,    25,    26,     0,     0,    11,    12,
      13,     0,    15,    16,    17,     0,    18,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    21,     0,     0,
      22,     0,    24,     0,    25,    26
};

static const yytype_int16 yycheck[] =
{
       0,    20,    34,    13,     0,    15,    16,    17,    23,    69,
       3,    21,    74,    72,    29,     0,    26,     0,    33,     8,
       9,    27,    11,    12,    24,    14,    64,     3,     4,    29,
      19,    93,    32,    29,    23,   117,    32,     3,     4,    99,
     117,    79,    80,    81,    29,    34,    56,    57,   107,     3,
       4,    27,    27,   135,    86,    12,    23,   139,    22,   141,
     119,    37,   139,    20,   141,    12,    66,   127,    35,    12,
      66,    37,    72,    20,    31,    85,    23,    20,   144,    68,
     146,    23,    92,    37,    31,    38,    33,    34,    31,    23,
      33,    34,   139,    35,   141,    29,   115,    86,    31,    33,
      23,    24,   140,    28,    28,    27,    38,   107,    38,   109,
     110,   111,    27,   109,   110,   111,    29,   117,   131,   119,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,    -1,    -1,   135,    -1,    -1,    -1,   139,
      -1,   141,    -1,   153,    -1,    -1,    -1,    -1,    -1,   149,
      -1,    -1,    -1,   149,    -1,   144,     1,   146,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    23,    24,
      -1,    26,    -1,    -1,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    -1,    13,    14,    15,    16,
      17,    18,    19,    -1,    21,    -1,    -1,    -1,    -1,    26,
      27,    28,    -1,    30,    -1,    32,    -1,    -1,    35,    36,
      37,    -1,    39,    40,     3,     4,     5,     6,     7,     8,
       9,    -1,    -1,    -1,    13,    14,    15,    16,    17,    18,
      19,    -1,    21,    -1,    23,    -1,    -1,    26,    -1,    28,
      29,    30,    -1,    32,    -1,    -1,    35,    36,    37,    -1,
      39,    40,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    -1,    13,    14,    15,    16,    17,    18,    19,    -1,
      21,    -1,    -1,    -1,    -1,    26,    -1,    28,    -1,    30,
      -1,    32,    -1,    -1,    35,    36,    37,    -1,    39,    40,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    -1,
      13,    14,    15,    16,    17,    18,    19,    -1,    21,    -1,
      23,    -1,    -1,    26,    -1,    -1,    -1,    30,    -1,    32,
      -1,    -1,    35,    36,    37,    -1,    39,    40,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    -1,    13,    14,
      15,    16,    17,    18,    19,    -1,    21,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    -1,    30,    -1,    32,    -1,    -1,
      35,    36,    37,    -1,    39,    40,     3,     4,     5,     6,
       7,     8,     9,    -1,    -1,    -1,    13,    14,    15,    16,
      17,    18,    19,    -1,    21,    -1,    -1,    -1,    -1,    26,
      27,    28,    -1,    30,    -1,    32,    -1,    -1,    35,    36,
      37,    -1,    39,    40,     3,     4,     5,     6,     7,     8,
       9,    -1,    -1,    -1,    13,    14,    15,    16,    17,    18,
      19,    -1,    21,    -1,    -1,    -1,    -1,    26,    -1,    28,
      29,    30,    -1,    32,    -1,    -1,    35,    36,    37,    -1,
      39,    40,     3,     4,     5,     6,     7,     8,     9,    -1,
      -1,    -1,    13,    14,    15,    16,    17,    18,    19,    -1,
      21,    -1,    -1,    -1,    -1,    26,    -1,    28,    -1,    30,
      -1,    32,    -1,    -1,    35,    36,    37,    -1,    39,    40,
       3,     4,     5,     6,     7,     8,     9,    -1,    -1,    -1,
      13,    14,    15,    16,    17,    18,    19,    -1,    21,    -1,
      -1,    -1,    -1,    26,    -1,    -1,    -1,    30,    -1,    32,
      -1,    -1,    35,    36,    37,    -1,    39,    40,     3,     4,
      -1,    -1,    -1,    -1,     9,    -1,    -1,    -1,    13,    14,
      15,    -1,    17,    18,    19,    -1,    21,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,     3,     4,
      35,    -1,    37,    -1,    39,    40,    -1,    -1,    13,    14,
      15,    -1,    17,    18,    19,    -1,    21,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      35,    -1,    37,    -1,    39,    40
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     1,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    13,    14,    15,    16,    17,    18,    19,    21,    26,
      30,    32,    35,    36,    37,    39,    40,    42,    44,    46,
      48,    51,    53,    57,    58,    61,    62,    68,    23,    24,
      43,     5,     6,     7,     8,     9,    16,    26,    28,    30,
      36,    59,    60,    61,    69,    60,    60,    60,    59,    60,
      59,    59,    59,     3,    60,    27,    67,    59,    65,    60,
      45,    46,    47,    48,    63,    59,     0,    43,    44,    12,
      20,    31,    33,    34,    48,    27,    52,    53,    56,    60,
      67,    66,    27,    63,    59,    59,    64,    66,    48,    22,
      23,    29,    60,    64,    38,    45,    23,    37,    62,    66,
      66,    66,    59,    53,    60,    28,    23,    35,    59,    37,
      60,    35,    64,    45,    48,    48,    48,    67,    54,    55,
      57,    58,    45,    49,    50,    60,    29,    38,    64,    23,
      29,    33,    56,    38,    23,    29,    33,    58,    55,    66,
      55,    50,    50,    27,    48
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    41,    42,    42,    43,    43,    44,    44,    45,    45,
      46,    46,    47,    47,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    49,    49,    49,
      50,    50,    51,    51,    52,    52,    52,    52,    53,    53,
      54,    54,    54,    55,    55,    55,    56,    57,    57,    58,
      58,    59,    59,    60,    60,    61,    61,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    61,    62,
      62,    63,    63,    64,    64,    65,    65,    65,    66,    66,
      67,    67,    68,    68,    68,    68,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     2,     1,     1,     1,     2,     1,     2,
       2,     2,     1,     2,     0,     1,     2,     2,     1,     7,
       4,     4,     4,     3,     3,     3,     6,     1,     3,     3,
       0,     2,     1,     2,     1,     1,     2,     2,     1,     2,
       1,     3,     3,     0,     1,     2,     4,     6,     2,     1,
       3,     1,     1,     1,     3,     1,     3,     3,     5,     2,
       5,     2,     2,     2,     2,     2,     2,     3,     3,     1,
       1,     0,     2,     0,     2,     0,     2,     2,     0,     2,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* es: line end  */
#line 43 "./parse.y"
                                { input->parsetree = (yyvsp[-1].tree); YYACCEPT; }
#line 1406 "y.tab.c"
    break;

  case 3: /* es: error end  */
#line 44 "./parse.y"
                                { yyerrok; input->parsetree = NULL; YYABORT; }
#line 1412 "y.tab.c"
    break;

  case 4: /* end: NL  */
#line 46 "./parse.y"
                                { if (!readheredocs(FALSE)) YYABORT; }
#line 1418 "y.tab.c"
    break;

  case 5: /* end: ENDFILE  */
#line 47 "./parse.y"
                                { if (!readheredocs(TRUE)) YYABORT; }
#line 1424 "y.tab.c"
    break;

  case 6: /* line: cmd  */
#line 49 "./parse.y"
                                { (yyval.tree) = (yyvsp[0].tree); }
#line 1430 "y.tab.c"
    break;

  case 7: /* line: cmdsa line  */
#line 50 "./parse.y"
                                { (yyval.tree) = mkseq("%seq", (yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1436 "y.tab.c"
    break;

  case 8: /* body: cmd  */
#line 52 "./parse.y"
                                { (yyval.tree) = (yyvsp[0].tree); }
#line 1442 "y.tab.c"
    break;

  case 9: /* body: cmdsan body  */
#line 53 "./parse.y"
                                { (yyval.tree) = mkseq("%seq", (yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1448 "y.tab.c"
    break;

  case 10: /* cmdsa: cmd ';'  */
#line 55 "./parse.y"
                                { (yyval.tree) = (yyvsp[-1].tree); }
#line 1454 "y.tab.c"
    break;

  case 11: /* cmdsa: cmd '&'  */
#line 56 "./parse.y"
                                { (yyval.tree) = prefix("%background", mk(nList, thunkify((yyvsp[-1].tree)), NULL)); }
#line 1460 "y.tab.c"
    break;

  case 12: /* cmdsan: cmdsa  */
#line 58 "./parse.y"
                                { (yyval.tree) = (yyvsp[0].tree); }
#line 1466 "y.tab.c"
    break;

  case 13: /* cmdsan: cmd NL  */
#line 59 "./parse.y"
                                { (yyval.tree) = (yyvsp[-1].tree); if (!readheredocs(FALSE)) YYABORT; }
#line 1472 "y.tab.c"
    break;

  case 14: /* cmd: %empty  */
#line 61 "./parse.y"
                                                { (yyval.tree) = NULL; }
#line 1478 "y.tab.c"
    break;

  case 15: /* cmd: simple  */
#line 62 "./parse.y"
                                                { (yyval.tree) = redirect((yyvsp[0].tree)); if ((yyval.tree) == &errornode) YYABORT; }
#line 1484 "y.tab.c"
    break;

  case 16: /* cmd: redir cmd  */
#line 63 "./parse.y"
                                                { (yyval.tree) = redirect(mk(nRedir, (yyvsp[-1].tree), (yyvsp[0].tree))); if ((yyval.tree) == &errornode) YYABORT; }
#line 1490 "y.tab.c"
    break;

  case 17: /* cmd: first assign  */
#line 64 "./parse.y"
                                                { (yyval.tree) = mk(nAssign, (yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1496 "y.tab.c"
    break;

  case 18: /* cmd: fn  */
#line 65 "./parse.y"
                                                { (yyval.tree) = (yyvsp[0].tree); }
#line 1502 "y.tab.c"
    break;

  case 19: /* cmd: binder nl '(' bindings ')' nl cmd  */
#line 66 "./parse.y"
                                                { (yyval.tree) = mk((yyvsp[-6].kind), (yyvsp[-3].tree), (yyvsp[0].tree)); }
#line 1508 "y.tab.c"
    break;

  case 20: /* cmd: cmd ANDAND nl cmd  */
#line 67 "./parse.y"
                                                { (yyval.tree) = mkseq("%and", (yyvsp[-3].tree), (yyvsp[0].tree)); }
#line 1514 "y.tab.c"
    break;

  case 21: /* cmd: cmd OROR nl cmd  */
#line 68 "./parse.y"
                                                { (yyval.tree) = mkseq("%or", (yyvsp[-3].tree), (yyvsp[0].tree)); }
#line 1520 "y.tab.c"
    break;

  case 22: /* cmd: cmd PIPE nl cmd  */
#line 69 "./parse.y"
                                                { (yyval.tree) = mkpipe((yyvsp[-3].tree), (yyvsp[-2].tree)->u[0].i, (yyvsp[-2].tree)->u[1].i, (yyvsp[0].tree)); }
#line 1526 "y.tab.c"
    break;

  case 23: /* cmd: '!' caret cmd  */
#line 70 "./parse.y"
                                                { (yyval.tree) = prefix("%not", mk(nList, thunkify((yyvsp[0].tree)), NULL)); }
#line 1532 "y.tab.c"
    break;

  case 24: /* cmd: '~' word words  */
#line 71 "./parse.y"
                                                { (yyval.tree) = mk(nMatch, (yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1538 "y.tab.c"
    break;

  case 25: /* cmd: EXTRACT word words  */
#line 72 "./parse.y"
                                                { (yyval.tree) = mk(nExtract, (yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1544 "y.tab.c"
    break;

  case 26: /* cmd: MATCH word nl '(' cases ')'  */
#line 73 "./parse.y"
                                                { (yyval.tree) = mkmatch((yyvsp[-4].tree), (yyvsp[-1].tree)); }
#line 1550 "y.tab.c"
    break;

  case 27: /* cases: case  */
#line 75 "./parse.y"
                                        { (yyval.tree) = treecons((yyvsp[0].tree), NULL); }
#line 1556 "y.tab.c"
    break;

  case 28: /* cases: cases ';' case  */
#line 76 "./parse.y"
                                        { (yyval.tree) = treeconsend((yyvsp[-2].tree), (yyvsp[0].tree)); }
#line 1562 "y.tab.c"
    break;

  case 29: /* cases: cases NL case  */
#line 77 "./parse.y"
                                        { (yyval.tree) = treeconsend((yyvsp[-2].tree), (yyvsp[0].tree)); }
#line 1568 "y.tab.c"
    break;

  case 30: /* case: %empty  */
#line 79 "./parse.y"
                                        { (yyval.tree) = NULL; }
#line 1574 "y.tab.c"
    break;

  case 31: /* case: word first  */
#line 80 "./parse.y"
                                        { (yyval.tree) = mk(nMatch, (yyvsp[-1].tree), thunkify((yyvsp[0].tree))); }
#line 1580 "y.tab.c"
    break;

  case 32: /* simple: first  */
#line 82 "./parse.y"
                                        { (yyval.tree) = treecons((yyvsp[0].tree), NULL); }
#line 1586 "y.tab.c"
    break;

  case 33: /* simple: first args  */
#line 83 "./parse.y"
                                        { (yyval.tree) = firstprepend((yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1592 "y.tab.c"
    break;

  case 34: /* args: word  */
#line 85 "./parse.y"
                                        { (yyval.tree) = treecons((yyvsp[0].tree), NULL); }
#line 1598 "y.tab.c"
    break;

  case 35: /* args: redir  */
#line 86 "./parse.y"
                                        { (yyval.tree) = redirappend(NULL, (yyvsp[0].tree)); }
#line 1604 "y.tab.c"
    break;

  case 36: /* args: args word  */
#line 87 "./parse.y"
                                        { (yyval.tree) = treeconsend((yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1610 "y.tab.c"
    break;

  case 37: /* args: args redir  */
#line 88 "./parse.y"
                                        { (yyval.tree) = redirappend((yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1616 "y.tab.c"
    break;

  case 38: /* redir: DUP  */
#line 90 "./parse.y"
                                        { (yyval.tree) = (yyvsp[0].tree); }
#line 1622 "y.tab.c"
    break;

  case 39: /* redir: REDIR word  */
#line 91 "./parse.y"
                                        { (yyval.tree) = mkredir((yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1628 "y.tab.c"
    break;

  case 40: /* bindings: binding  */
#line 93 "./parse.y"
                                        { (yyval.tree) = treecons((yyvsp[0].tree), NULL); }
#line 1634 "y.tab.c"
    break;

  case 41: /* bindings: bindings ';' binding  */
#line 94 "./parse.y"
                                        { (yyval.tree) = treeconsend((yyvsp[-2].tree), (yyvsp[0].tree)); }
#line 1640 "y.tab.c"
    break;

  case 42: /* bindings: bindings NL binding  */
#line 95 "./parse.y"
                                        { (yyval.tree) = treeconsend((yyvsp[-2].tree), (yyvsp[0].tree)); }
#line 1646 "y.tab.c"
    break;

  case 43: /* binding: %empty  */
#line 97 "./parse.y"
                                        { (yyval.tree) = NULL; }
#line 1652 "y.tab.c"
    break;

  case 44: /* binding: fn  */
#line 98 "./parse.y"
                                        { (yyval.tree) = (yyvsp[0].tree); }
#line 1658 "y.tab.c"
    break;

  case 45: /* binding: first assign  */
#line 99 "./parse.y"
                                        { (yyval.tree) = mk(nAssign, (yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1664 "y.tab.c"
    break;

  case 46: /* assign: caret '=' caret words  */
#line 101 "./parse.y"
                                        { (yyval.tree) = (yyvsp[0].tree); }
#line 1670 "y.tab.c"
    break;

  case 47: /* fn: FN word params '{' body '}'  */
#line 103 "./parse.y"
                                        { (yyval.tree) = fnassign((yyvsp[-4].tree), mklambda((yyvsp[-3].tree), (yyvsp[-1].tree))); }
#line 1676 "y.tab.c"
    break;

  case 48: /* fn: FN word  */
#line 104 "./parse.y"
                                        { (yyval.tree) = fnassign((yyvsp[0].tree), NULL); }
#line 1682 "y.tab.c"
    break;

  case 49: /* first: comword  */
#line 106 "./parse.y"
                                        { (yyval.tree) = (yyvsp[0].tree); }
#line 1688 "y.tab.c"
    break;

  case 50: /* first: first '^' sword  */
#line 107 "./parse.y"
                                        { (yyval.tree) = mk(nConcat, (yyvsp[-2].tree), (yyvsp[0].tree)); }
#line 1694 "y.tab.c"
    break;

  case 51: /* sword: comword  */
#line 109 "./parse.y"
                                        { (yyval.tree) = (yyvsp[0].tree); }
#line 1700 "y.tab.c"
    break;

  case 52: /* sword: keyword  */
#line 110 "./parse.y"
                                        { (yyval.tree) = mk(nWord, (yyvsp[0].str)); }
#line 1706 "y.tab.c"
    break;

  case 53: /* word: sword  */
#line 112 "./parse.y"
                                        { (yyval.tree) = (yyvsp[0].tree); }
#line 1712 "y.tab.c"
    break;

  case 54: /* word: word '^' sword  */
#line 113 "./parse.y"
                                        { (yyval.tree) = mk(nConcat, (yyvsp[-2].tree), (yyvsp[0].tree)); }
#line 1718 "y.tab.c"
    break;

  case 55: /* comword: param  */
#line 115 "./parse.y"
                                        { (yyval.tree) = (yyvsp[0].tree); }
#line 1724 "y.tab.c"
    break;

  case 56: /* comword: '(' nlwords ')'  */
#line 116 "./parse.y"
                                        { (yyval.tree) = (yyvsp[-1].tree); }
#line 1730 "y.tab.c"
    break;

  case 57: /* comword: '{' body '}'  */
#line 117 "./parse.y"
                                        { (yyval.tree) = thunkify((yyvsp[-1].tree)); }
#line 1736 "y.tab.c"
    break;

  case 58: /* comword: '@' params '{' body '}'  */
#line 118 "./parse.y"
                                        { (yyval.tree) = mklambda((yyvsp[-3].tree), (yyvsp[-1].tree)); }
#line 1742 "y.tab.c"
    break;

  case 59: /* comword: '$' sword  */
#line 119 "./parse.y"
                                        { (yyval.tree) = mk(nVar, (yyvsp[0].tree)); }
#line 1748 "y.tab.c"
    break;

  case 60: /* comword: '$' sword SUB words ')'  */
#line 120 "./parse.y"
                                        { (yyval.tree) = mk(nVarsub, (yyvsp[-3].tree), (yyvsp[-1].tree)); }
#line 1754 "y.tab.c"
    break;

  case 61: /* comword: CALL sword  */
#line 121 "./parse.y"
                                        { (yyval.tree) = mk(nCall, (yyvsp[0].tree)); }
#line 1760 "y.tab.c"
    break;

  case 62: /* comword: COUNT sword  */
#line 122 "./parse.y"
                                        { (yyval.tree) = mk(nCall, prefix("%count", treecons(mk(nVar, (yyvsp[0].tree)), NULL))); }
#line 1766 "y.tab.c"
    break;

  case 63: /* comword: FLAT sword  */
#line 123 "./parse.y"
                                        { (yyval.tree) = flatten(mk(nVar, (yyvsp[0].tree)), " "); }
#line 1772 "y.tab.c"
    break;

  case 64: /* comword: PRIM WORD  */
#line 124 "./parse.y"
                                        { (yyval.tree) = mk(nPrim, (yyvsp[0].str)); }
#line 1778 "y.tab.c"
    break;

  case 65: /* comword: '`' sword  */
#line 125 "./parse.y"
                                        { (yyval.tree) = backquote(mk(nVar, mk(nWord, "ifs")), (yyvsp[0].tree)); }
#line 1784 "y.tab.c"
    break;

  case 66: /* comword: BFLAT sword  */
#line 126 "./parse.y"
                                        { (yyval.tree) = flatten(backquote(mk(nVar, mk(nWord, "ifs")), (yyvsp[0].tree)), " "); }
#line 1790 "y.tab.c"
    break;

  case 67: /* comword: BACKBACK word sword  */
#line 127 "./parse.y"
                                        { (yyval.tree) = backquote((yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1796 "y.tab.c"
    break;

  case 68: /* comword: BBFLAT word sword  */
#line 128 "./parse.y"
                                        { (yyval.tree) = flatten(backquote((yyvsp[-1].tree), (yyvsp[0].tree)), " "); }
#line 1802 "y.tab.c"
    break;

  case 69: /* param: WORD  */
#line 130 "./parse.y"
                                        { (yyval.tree) = mk(nWord, (yyvsp[0].str)); }
#line 1808 "y.tab.c"
    break;

  case 70: /* param: QWORD  */
#line 131 "./parse.y"
                                        { (yyval.tree) = mk(nQword, (yyvsp[0].str)); }
#line 1814 "y.tab.c"
    break;

  case 71: /* params: %empty  */
#line 133 "./parse.y"
                                        { (yyval.tree) = NULL; }
#line 1820 "y.tab.c"
    break;

  case 72: /* params: params param  */
#line 134 "./parse.y"
                                        { (yyval.tree) = treeconsend((yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1826 "y.tab.c"
    break;

  case 73: /* words: %empty  */
#line 136 "./parse.y"
                                        { (yyval.tree) = NULL; }
#line 1832 "y.tab.c"
    break;

  case 74: /* words: words word  */
#line 137 "./parse.y"
                                        { (yyval.tree) = treeconsend((yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1838 "y.tab.c"
    break;

  case 75: /* nlwords: %empty  */
#line 139 "./parse.y"
                                        { (yyval.tree) = NULL; }
#line 1844 "y.tab.c"
    break;

  case 76: /* nlwords: nlwords word  */
#line 140 "./parse.y"
                                        { (yyval.tree) = treeconsend((yyvsp[-1].tree), (yyvsp[0].tree)); }
#line 1850 "y.tab.c"
    break;

  case 77: /* nlwords: nlwords NL  */
#line 141 "./parse.y"
                                        { (yyval.tree) = (yyvsp[-1].tree); }
#line 1856 "y.tab.c"
    break;

  case 82: /* binder: LOCAL  */
#line 149 "./parse.y"
                        { (yyval.kind) = nLocal; }
#line 1862 "y.tab.c"
    break;

  case 83: /* binder: LET  */
#line 150 "./parse.y"
                        { (yyval.kind) = nLet; }
#line 1868 "y.tab.c"
    break;

  case 84: /* binder: FOR  */
#line 151 "./parse.y"
                        { (yyval.kind) = nFor; }
#line 1874 "y.tab.c"
    break;

  case 85: /* binder: CLOSURE  */
#line 152 "./parse.y"
                        { (yyval.kind) = nClosure; }
#line 1880 "y.tab.c"
    break;

  case 86: /* keyword: '!'  */
#line 154 "./parse.y"
                        { (yyval.str) = "!"; }
#line 1886 "y.tab.c"
    break;

  case 87: /* keyword: '~'  */
#line 155 "./parse.y"
                        { (yyval.str) = "~"; }
#line 1892 "y.tab.c"
    break;

  case 88: /* keyword: '='  */
#line 156 "./parse.y"
                        { (yyval.str) = "="; }
#line 1898 "y.tab.c"
    break;

  case 89: /* keyword: EXTRACT  */
#line 157 "./parse.y"
                        { (yyval.str) = "~~"; }
#line 1904 "y.tab.c"
    break;

  case 90: /* keyword: LOCAL  */
#line 158 "./parse.y"
                        { (yyval.str) = "local"; }
#line 1910 "y.tab.c"
    break;

  case 91: /* keyword: LET  */
#line 159 "./parse.y"
                        { (yyval.str) = "let"; }
#line 1916 "y.tab.c"
    break;

  case 92: /* keyword: FOR  */
#line 160 "./parse.y"
                        { (yyval.str) = "for"; }
#line 1922 "y.tab.c"
    break;

  case 93: /* keyword: FN  */
#line 161 "./parse.y"
                        { (yyval.str) = "fn"; }
#line 1928 "y.tab.c"
    break;

  case 94: /* keyword: CLOSURE  */
#line 162 "./parse.y"
                        { (yyval.str) = "%closure"; }
#line 1934 "y.tab.c"
    break;

  case 95: /* keyword: MATCH  */
#line 163 "./parse.y"
                        { (yyval.str) = "match"; }
#line 1940 "y.tab.c"
    break;


#line 1944 "y.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

