/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 22 "ldgram.y"

/*

 */

#define DONTDECLARE_MALLOC

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "ld.h"
#include "ldexp.h"
#include "ldver.h"
#include "ldlang.h"
#include "ldfile.h"
#include "ldemul.h"
#include "ldmisc.h"
#include "ldmain.h"
#include "mri.h"
#include "ldctor.h"
#include "ldlex.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

#ifndef YYDEBUG
#define YYDEBUG 1
#endif

static enum section_type sectype;
static lang_memory_region_type *region;

bfd_boolean ldgram_had_keep = FALSE;
char *ldgram_vers_current_lang = NULL;

#define ERROR_NAME_MAX 20
static char *error_names[ERROR_NAME_MAX];
static int error_index;
#define PUSH_ERROR(x) if (error_index < ERROR_NAME_MAX) error_names[error_index] = x; error_index++;
#define POP_ERROR()   error_index--;


/* Line 189 of yacc.c  */
#line 115 "ldgram.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT = 258,
     NAME = 259,
     LNAME = 260,
     OREQ = 261,
     ANDEQ = 262,
     RSHIFTEQ = 263,
     LSHIFTEQ = 264,
     DIVEQ = 265,
     MULTEQ = 266,
     MINUSEQ = 267,
     PLUSEQ = 268,
     OROR = 269,
     ANDAND = 270,
     NE = 271,
     EQ = 272,
     GE = 273,
     LE = 274,
     RSHIFT = 275,
     LSHIFT = 276,
     UNARY = 277,
     END = 278,
     ALIGN_K = 279,
     BLOCK = 280,
     BIND = 281,
     QUAD = 282,
     SQUAD = 283,
     LONG = 284,
     SHORT = 285,
     BYTE = 286,
     SECTIONS = 287,
     PHDRS = 288,
     INSERT_K = 289,
     AFTER = 290,
     BEFORE = 291,
     DATA_SEGMENT_ALIGN = 292,
     DATA_SEGMENT_RELRO_END = 293,
     DATA_SEGMENT_END = 294,
     SORT_BY_NAME = 295,
     SORT_BY_ALIGNMENT = 296,
     SORT_NONE = 297,
     SORT_BY_INIT_PRIORITY = 298,
     SIZEOF_HEADERS = 299,
     OUTPUT_FORMAT = 300,
     FORCE_COMMON_ALLOCATION = 301,
     OUTPUT_ARCH = 302,
     INHIBIT_COMMON_ALLOCATION = 303,
     SEGMENT_START = 304,
     INCLUDE = 305,
     MEMORY = 306,
     REGION_ALIAS = 307,
     LD_FEATURE = 308,
     NOLOAD = 309,
     DSECT = 310,
     COPY = 311,
     INFO = 312,
     OVERLAY = 313,
     DEFINED = 314,
     TARGET_K = 315,
     SEARCH_DIR = 316,
     MAP = 317,
     ENTRY = 318,
     NEXT = 319,
     SIZEOF = 320,
     ALIGNOF = 321,
     ADDR = 322,
     LOADADDR = 323,
     MAX_K = 324,
     MIN_K = 325,
     STARTUP = 326,
     HLL = 327,
     SYSLIB = 328,
     FLOAT = 329,
     NOFLOAT = 330,
     NOCROSSREFS = 331,
     ORIGIN = 332,
     FILL = 333,
     LENGTH = 334,
     CREATE_OBJECT_SYMBOLS = 335,
     INPUT = 336,
     GROUP = 337,
     OUTPUT = 338,
     CONSTRUCTORS = 339,
     ALIGNMOD = 340,
     AT = 341,
     OVERFLOW_INTO = 342,
     SUBALIGN = 343,
     HIDDEN = 344,
     PROVIDE = 345,
     PROVIDE_HIDDEN = 346,
     AS_NEEDED = 347,
     CHIP = 348,
     LIST = 349,
     SECT = 350,
     ABSOLUTE = 351,
     LOAD = 352,
     NEWLINE = 353,
     ENDWORD = 354,
     ORDER = 355,
     NAMEWORD = 356,
     ASSERT_K = 357,
     LOG2CEIL = 358,
     FORMAT = 359,
     PUBLIC = 360,
     DEFSYMEND = 361,
     BASE = 362,
     ALIAS = 363,
     TRUNCATE = 364,
     REL = 365,
     INPUT_SCRIPT = 366,
     INPUT_MRI_SCRIPT = 367,
     INPUT_DEFSYM = 368,
     CASE = 369,
     EXTERN = 370,
     START = 371,
     VERS_TAG = 372,
     VERS_IDENTIFIER = 373,
     GLOBAL = 374,
     LOCAL = 375,
     VERSIONK = 376,
     INPUT_VERSION_SCRIPT = 377,
     KEEP = 378,
     ONLY_IF_RO = 379,
     ONLY_IF_RW = 380,
     SPECIAL = 381,
     INPUT_SECTION_FLAGS = 382,
     ALIGN_WITH_INPUT = 383,
     EXCLUDE_FILE = 384,
     CONSTANT = 385,
     INPUT_DYNAMIC_LIST = 386
   };
#endif
/* Tokens.  */
#define INT 258
#define NAME 259
#define LNAME 260
#define OREQ 261
#define ANDEQ 262
#define RSHIFTEQ 263
#define LSHIFTEQ 264
#define DIVEQ 265
#define MULTEQ 266
#define MINUSEQ 267
#define PLUSEQ 268
#define OROR 269
#define ANDAND 270
#define NE 271
#define EQ 272
#define GE 273
#define LE 274
#define RSHIFT 275
#define LSHIFT 276
#define UNARY 277
#define END 278
#define ALIGN_K 279
#define BLOCK 280
#define BIND 281
#define QUAD 282
#define SQUAD 283
#define LONG 284
#define SHORT 285
#define BYTE 286
#define SECTIONS 287
#define PHDRS 288
#define INSERT_K 289
#define AFTER 290
#define BEFORE 291
#define DATA_SEGMENT_ALIGN 292
#define DATA_SEGMENT_RELRO_END 293
#define DATA_SEGMENT_END 294
#define SORT_BY_NAME 295
#define SORT_BY_ALIGNMENT 296
#define SORT_NONE 297
#define SORT_BY_INIT_PRIORITY 298
#define SIZEOF_HEADERS 299
#define OUTPUT_FORMAT 300
#define FORCE_COMMON_ALLOCATION 301
#define OUTPUT_ARCH 302
#define INHIBIT_COMMON_ALLOCATION 303
#define SEGMENT_START 304
#define INCLUDE 305
#define MEMORY 306
#define REGION_ALIAS 307
#define LD_FEATURE 308
#define NOLOAD 309
#define DSECT 310
#define COPY 311
#define INFO 312
#define OVERLAY 313
#define DEFINED 314
#define TARGET_K 315
#define SEARCH_DIR 316
#define MAP 317
#define ENTRY 318
#define NEXT 319
#define SIZEOF 320
#define ALIGNOF 321
#define ADDR 322
#define LOADADDR 323
#define MAX_K 324
#define MIN_K 325
#define STARTUP 326
#define HLL 327
#define SYSLIB 328
#define FLOAT 329
#define NOFLOAT 330
#define NOCROSSREFS 331
#define ORIGIN 332
#define FILL 333
#define LENGTH 334
#define CREATE_OBJECT_SYMBOLS 335
#define INPUT 336
#define GROUP 337
#define OUTPUT 338
#define CONSTRUCTORS 339
#define ALIGNMOD 340
#define AT 341
#define OVERFLOW_INTO 342
#define SUBALIGN 343
#define HIDDEN 344
#define PROVIDE 345
#define PROVIDE_HIDDEN 346
#define AS_NEEDED 347
#define CHIP 348
#define LIST 349
#define SECT 350
#define ABSOLUTE 351
#define LOAD 352
#define NEWLINE 353
#define ENDWORD 354
#define ORDER 355
#define NAMEWORD 356
#define ASSERT_K 357
#define LOG2CEIL 358
#define FORMAT 359
#define PUBLIC 360
#define DEFSYMEND 361
#define BASE 362
#define ALIAS 363
#define TRUNCATE 364
#define REL 365
#define INPUT_SCRIPT 366
#define INPUT_MRI_SCRIPT 367
#define INPUT_DEFSYM 368
#define CASE 369
#define EXTERN 370
#define START 371
#define VERS_TAG 372
#define VERS_IDENTIFIER 373
#define GLOBAL 374
#define LOCAL 375
#define VERSIONK 376
#define INPUT_VERSION_SCRIPT 377
#define KEEP 378
#define ONLY_IF_RO 379
#define ONLY_IF_RW 380
#define SPECIAL 381
#define INPUT_SECTION_FLAGS 382
#define ALIGN_WITH_INPUT 383
#define EXCLUDE_FILE 384
#define CONSTANT 385
#define INPUT_DYNAMIC_LIST 386




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 63 "ldgram.y"

  bfd_vma integer;
  struct big_int
    {
      bfd_vma integer;
      char *str;
    } bigint;
  fill_type *fill;
  char *name;
  const char *cname;
  struct wildcard_spec wildcard;
  struct wildcard_list *wildcard_list;
  struct name_list *name_list;
  struct flag_info_list *flag_info_list;
  struct flag_info *flag_info;
  int token;
  union etree_union *etree;
  struct phdr_info
    {
      bfd_boolean filehdr;
      bfd_boolean phdrs;
      union etree_union *at;
      union etree_union *flags;
    } phdr;
  struct lang_nocrossref *nocrossref;
  struct lang_output_section_phdr_list *section_phdr;
  struct bfd_elf_version_deps *deflist;
  struct bfd_elf_version_expr *versyms;
  struct bfd_elf_version_tree *versnode;



/* Line 214 of yacc.c  */
#line 446 "ldgram.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 458 "ldgram.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  17
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1939

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  155
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  130
/* YYNRULES -- Number of rules.  */
#define YYNRULES  371
/* YYNRULES -- Number of states.  */
#define YYNSTATES  808

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   386

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   153,     2,     2,     2,    34,    21,     2,
      37,   150,    32,    30,   148,    31,     2,    33,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    16,   149,
      24,     6,    25,    15,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   151,     2,   152,    20,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    58,    19,    59,   154,     2,     2,     2,
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
       5,     7,     8,     9,    10,    11,    12,    13,    14,    17,
      18,    22,    23,    26,    27,    28,    29,    35,    36,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     9,    12,    15,    18,    20,    21,
      26,    27,    30,    34,    35,    38,    43,    45,    47,    50,
      52,    57,    62,    66,    69,    74,    78,    83,    88,    93,
      98,   103,   106,   109,   112,   117,   122,   125,   128,   131,
     134,   135,   141,   144,   145,   149,   152,   153,   155,   159,
     161,   165,   166,   168,   172,   173,   176,   178,   181,   185,
     186,   189,   192,   193,   195,   197,   199,   201,   203,   205,
     207,   209,   211,   213,   218,   223,   228,   233,   242,   247,
     249,   251,   256,   257,   263,   268,   269,   275,   280,   285,
     289,   293,   300,   305,   307,   311,   314,   316,   320,   323,
     324,   330,   331,   339,   340,   347,   352,   355,   358,   359,
     364,   367,   368,   376,   378,   380,   382,   384,   390,   395,
     400,   405,   413,   421,   429,   437,   446,   451,   453,   457,
     462,   465,   467,   471,   473,   475,   478,   482,   487,   492,
     498,   500,   501,   507,   510,   512,   514,   516,   521,   523,
     528,   533,   534,   543,   544,   550,   553,   555,   556,   558,
     560,   562,   564,   566,   568,   570,   573,   574,   576,   578,
     580,   582,   584,   586,   588,   590,   592,   594,   598,   602,
     609,   616,   623,   625,   626,   631,   633,   634,   638,   640,
     641,   649,   650,   656,   660,   664,   665,   669,   671,   674,
     676,   679,   684,   689,   693,   697,   699,   704,   708,   709,
     711,   713,   714,   717,   721,   722,   725,   728,   732,   737,
     740,   743,   746,   750,   754,   758,   762,   766,   770,   774,
     778,   782,   786,   790,   794,   798,   802,   806,   810,   816,
     820,   824,   829,   831,   833,   838,   843,   848,   853,   858,
     863,   868,   875,   882,   889,   894,   901,   906,   908,   915,
     922,   929,   934,   939,   944,   948,   949,   953,   954,   959,
     960,   965,   966,   968,   969,   974,   975,   977,   979,   981,
     982,   983,   984,   985,   986,   987,  1009,  1010,  1011,  1012,
    1013,  1014,  1034,  1035,  1036,  1044,  1045,  1051,  1053,  1055,
    1057,  1059,  1061,  1065,  1066,  1069,  1073,  1076,  1083,  1094,
    1097,  1099,  1100,  1102,  1105,  1106,  1107,  1111,  1112,  1113,
    1114,  1115,  1127,  1132,  1133,  1136,  1137,  1138,  1145,  1147,
    1148,  1152,  1158,  1159,  1163,  1164,  1167,  1169,  1172,  1177,
    1180,  1181,  1184,  1185,  1191,  1193,  1196,  1201,  1207,  1214,
    1216,  1219,  1220,  1223,  1228,  1233,  1242,  1244,  1246,  1250,
    1254,  1255,  1265,  1266,  1274,  1276,  1280,  1282,  1286,  1288,
    1292,  1293
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     156,     0,    -1,   127,   172,    -1,   128,   160,    -1,   138,
     273,    -1,   147,   268,    -1,   129,   158,    -1,     4,    -1,
      -1,   159,     4,     6,   227,    -1,    -1,   161,   162,    -1,
     162,   163,   114,    -1,    -1,   109,   227,    -1,   109,   227,
     148,   227,    -1,     4,    -1,   110,    -1,   116,   165,    -1,
     115,    -1,   121,     4,     6,   227,    -1,   121,     4,   148,
     227,    -1,   121,     4,   227,    -1,   120,     4,    -1,   111,
       4,   148,   227,    -1,   111,     4,   227,    -1,   111,     4,
       6,   227,    -1,    38,     4,     6,   227,    -1,    38,     4,
     148,   227,    -1,   101,     4,     6,   227,    -1,   101,     4,
     148,   227,    -1,   112,   167,    -1,   113,   166,    -1,   117,
       4,    -1,   124,     4,   148,     4,    -1,   124,     4,   148,
       3,    -1,   123,   227,    -1,   125,     3,    -1,   130,   168,
      -1,   131,   169,    -1,    -1,    66,   157,   164,   162,    36,
      -1,   132,     4,    -1,    -1,   165,   148,     4,    -1,   165,
       4,    -1,    -1,     4,    -1,   166,   148,     4,    -1,     4,
      -1,   167,   148,     4,    -1,    -1,     4,    -1,   168,   148,
       4,    -1,    -1,   170,   171,    -1,     4,    -1,   171,     4,
      -1,   171,   148,     4,    -1,    -1,   173,   174,    -1,   174,
     175,    -1,    -1,   207,    -1,   182,    -1,   260,    -1,   218,
      -1,   219,    -1,   221,    -1,   223,    -1,   184,    -1,   275,
      -1,   149,    -1,    76,    37,     4,   150,    -1,    77,    37,
     157,   150,    -1,    99,    37,   157,   150,    -1,    61,    37,
       4,   150,    -1,    61,    37,     4,   148,     4,   148,     4,
     150,    -1,    63,    37,     4,   150,    -1,    62,    -1,    64,
      -1,    97,    37,   178,   150,    -1,    -1,    98,   176,    37,
     178,   150,    -1,    78,    37,   157,   150,    -1,    -1,    66,
     157,   177,   174,    36,    -1,    92,    37,   224,   150,    -1,
     131,    37,   169,   150,    -1,    48,    49,     4,    -1,    48,
      50,     4,    -1,    68,    37,     4,   148,     4,   150,    -1,
      69,    37,     4,   150,    -1,     4,    -1,   178,   148,     4,
      -1,   178,     4,    -1,     5,    -1,   178,   148,     5,    -1,
     178,     5,    -1,    -1,   108,    37,   179,   178,   150,    -1,
      -1,   178,   148,   108,    37,   180,   178,   150,    -1,    -1,
     178,   108,    37,   181,   178,   150,    -1,    46,    58,   183,
      59,    -1,   183,   235,    -1,   183,   184,    -1,    -1,    79,
      37,     4,   150,    -1,   205,   204,    -1,    -1,   118,   185,
      37,   227,   148,     4,   150,    -1,     4,    -1,    32,    -1,
      15,    -1,   186,    -1,   145,    37,   190,   150,   186,    -1,
      54,    37,   186,   150,    -1,    55,    37,   186,   150,    -1,
      56,    37,   186,   150,    -1,    54,    37,    55,    37,   186,
     150,   150,    -1,    54,    37,    54,    37,   186,   150,   150,
      -1,    55,    37,    54,    37,   186,   150,   150,    -1,    55,
      37,    55,    37,   186,   150,   150,    -1,    54,    37,   145,
      37,   190,   150,   186,   150,    -1,    57,    37,   186,   150,
      -1,     4,    -1,   188,    21,     4,    -1,   143,    37,   188,
     150,    -1,   190,   186,    -1,   186,    -1,   191,   206,   187,
      -1,   187,    -1,     4,    -1,   189,     4,    -1,   151,   191,
     152,    -1,   189,   151,   191,   152,    -1,   187,    37,   191,
     150,    -1,   189,   187,    37,   191,   150,    -1,   192,    -1,
      -1,   139,    37,   194,   192,   150,    -1,   205,   204,    -1,
      96,    -1,   149,    -1,   100,    -1,    54,    37,   100,   150,
      -1,   193,    -1,   200,    37,   225,   150,    -1,    94,    37,
     201,   150,    -1,    -1,   118,   196,    37,   227,   148,     4,
     150,   204,    -1,    -1,    66,   157,   197,   199,    36,    -1,
     198,   195,    -1,   195,    -1,    -1,   198,    -1,    41,    -1,
      42,    -1,    43,    -1,    44,    -1,    45,    -1,   225,    -1,
       6,   201,    -1,    -1,    14,    -1,    13,    -1,    12,    -1,
      11,    -1,    10,    -1,     9,    -1,     8,    -1,     7,    -1,
     149,    -1,   148,    -1,     4,     6,   225,    -1,     4,   203,
     225,    -1,   105,    37,     4,     6,   225,   150,    -1,   106,
      37,     4,     6,   225,   150,    -1,   107,    37,     4,     6,
     225,   150,    -1,   148,    -1,    -1,    67,    58,   208,    59,
      -1,   209,    -1,    -1,   209,   206,   210,    -1,   210,    -1,
      -1,     4,   211,   215,    16,   213,   206,   214,    -1,    -1,
      66,   157,   212,   208,    36,    -1,    93,     6,   225,    -1,
      95,     6,   225,    -1,    -1,    37,   216,   150,    -1,   217,
      -1,   216,   217,    -1,     4,    -1,   153,     4,    -1,    87,
      37,   157,   150,    -1,    88,    37,   220,   150,    -1,    88,
      37,   150,    -1,   220,   206,   157,    -1,   157,    -1,    89,
      37,   222,   150,    -1,   222,   206,   157,    -1,    -1,    90,
      -1,    91,    -1,    -1,     4,   224,    -1,     4,   148,   224,
      -1,    -1,   226,   227,    -1,    31,   227,    -1,    37,   227,
     150,    -1,    80,    37,   227,   150,    -1,   153,   227,    -1,
      30,   227,    -1,   154,   227,    -1,   227,    32,   227,    -1,
     227,    33,   227,    -1,   227,    34,   227,    -1,   227,    30,
     227,    -1,   227,    31,   227,    -1,   227,    29,   227,    -1,
     227,    28,   227,    -1,   227,    23,   227,    -1,   227,    22,
     227,    -1,   227,    27,   227,    -1,   227,    26,   227,    -1,
     227,    24,   227,    -1,   227,    25,   227,    -1,   227,    21,
     227,    -1,   227,    20,   227,    -1,   227,    19,   227,    -1,
     227,    15,   227,    16,   227,    -1,   227,    18,   227,    -1,
     227,    17,   227,    -1,    75,    37,     4,   150,    -1,     3,
      -1,    60,    -1,    82,    37,     4,   150,    -1,    81,    37,
       4,   150,    -1,    83,    37,     4,   150,    -1,    84,    37,
       4,   150,    -1,   146,    37,     4,   150,    -1,   112,    37,
     227,   150,    -1,    38,    37,   227,   150,    -1,    38,    37,
     227,   148,   227,   150,    -1,    51,    37,   227,   148,   227,
     150,    -1,    52,    37,   227,   148,   227,   150,    -1,    53,
      37,   227,   150,    -1,    65,    37,     4,   148,   227,   150,
      -1,    39,    37,   227,   150,    -1,     4,    -1,    85,    37,
     227,   148,   227,   150,    -1,    86,    37,   227,   148,   227,
     150,    -1,   118,    37,   227,   148,     4,   150,    -1,    93,
      37,     4,   150,    -1,    95,    37,     4,   150,    -1,   119,
      37,   227,   150,    -1,   102,    25,     4,    -1,    -1,   103,
      25,     4,    -1,    -1,   102,    37,   227,   150,    -1,    -1,
      38,    37,   227,   150,    -1,    -1,   144,    -1,    -1,   104,
      37,   227,   150,    -1,    -1,   140,    -1,   141,    -1,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,   236,   251,
     230,   231,   232,   233,   237,   234,    58,   238,   199,    59,
     239,   254,   228,   255,   202,   229,   240,   206,    -1,    -1,
      -1,    -1,    -1,    -1,    74,   241,   252,   253,   230,   233,
     242,    58,   243,   256,    59,   244,   254,   228,   255,   202,
     229,   245,   206,    -1,    -1,    -1,    98,   246,   251,   247,
      58,   183,    59,    -1,    -1,    66,   157,   248,   183,    36,
      -1,    70,    -1,    71,    -1,    72,    -1,    73,    -1,    74,
      -1,    37,   249,   150,    -1,    -1,    37,   150,    -1,   227,
     250,    16,    -1,   250,    16,    -1,    40,    37,   227,   150,
     250,    16,    -1,    40,    37,   227,   150,    39,    37,   227,
     150,   250,    16,    -1,   227,    16,    -1,    16,    -1,    -1,
      92,    -1,    25,     4,    -1,    -1,    -1,   255,    16,     4,
      -1,    -1,    -1,    -1,    -1,   256,     4,   257,    58,   199,
      59,   258,   255,   202,   259,   206,    -1,    47,    58,   261,
      59,    -1,    -1,   261,   262,    -1,    -1,    -1,     4,   263,
     265,   266,   264,   149,    -1,   227,    -1,    -1,     4,   267,
     266,    -1,   102,    37,   227,   150,   266,    -1,    -1,    37,
     227,   150,    -1,    -1,   269,   270,    -1,   271,    -1,   270,
     271,    -1,    58,   272,    59,   149,    -1,   281,   149,    -1,
      -1,   274,   277,    -1,    -1,   276,   137,    58,   277,    59,
      -1,   278,    -1,   277,   278,    -1,    58,   280,    59,   149,
      -1,   133,    58,   280,    59,   149,    -1,   133,    58,   280,
      59,   279,   149,    -1,   133,    -1,   279,   133,    -1,    -1,
     281,   149,    -1,   135,    16,   281,   149,    -1,   136,    16,
     281,   149,    -1,   135,    16,   281,   149,   136,    16,   281,
     149,    -1,   134,    -1,     4,    -1,   281,   149,   134,    -1,
     281,   149,     4,    -1,    -1,   281,   149,   131,     4,    58,
     282,   281,   284,    59,    -1,    -1,   131,     4,    58,   283,
     281,   284,    59,    -1,   135,    -1,   281,   149,   135,    -1,
     136,    -1,   281,   149,   136,    -1,   131,    -1,   281,   149,
     131,    -1,    -1,   149,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   170,   170,   171,   172,   173,   174,   178,   182,   182,
     192,   192,   205,   206,   210,   211,   212,   215,   218,   219,
     220,   222,   224,   226,   228,   230,   232,   234,   236,   238,
     240,   242,   243,   244,   246,   248,   250,   252,   254,   255,
     257,   256,   260,   262,   266,   267,   268,   272,   274,   278,
     280,   285,   286,   287,   292,   292,   297,   299,   301,   306,
     306,   312,   313,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   330,   332,   334,   337,   339,   341,
     343,   345,   347,   346,   350,   353,   352,   356,   360,   361,
     363,   365,   367,   372,   375,   378,   381,   384,   387,   391,
     390,   396,   395,   401,   400,   408,   412,   413,   414,   418,
     420,   421,   421,   429,   433,   437,   444,   451,   458,   465,
     472,   479,   486,   493,   500,   507,   514,   523,   541,   562,
     575,   584,   595,   604,   615,   624,   633,   637,   646,   650,
     658,   660,   659,   666,   667,   671,   672,   677,   682,   683,
     688,   692,   692,   696,   695,   702,   703,   706,   708,   712,
     714,   716,   718,   720,   725,   732,   734,   738,   740,   742,
     744,   746,   748,   750,   752,   757,   757,   762,   766,   774,
     778,   782,   790,   790,   794,   797,   797,   800,   801,   806,
     805,   811,   810,   817,   825,   833,   834,   838,   839,   843,
     845,   850,   855,   856,   861,   863,   869,   871,   873,   877,
     879,   885,   888,   897,   908,   908,   914,   916,   918,   920,
     922,   924,   927,   929,   931,   933,   935,   937,   939,   941,
     943,   945,   947,   949,   951,   953,   955,   957,   959,   961,
     963,   965,   967,   969,   972,   974,   976,   978,   980,   982,
     984,   986,   988,   990,   992,   994,  1003,  1005,  1007,  1009,
    1011,  1013,  1015,  1017,  1023,  1024,  1028,  1029,  1034,  1035,
    1039,  1040,  1044,  1045,  1049,  1050,  1054,  1055,  1056,  1057,
    1060,  1065,  1068,  1074,  1076,  1060,  1083,  1085,  1087,  1092,
    1094,  1082,  1104,  1106,  1104,  1112,  1111,  1118,  1119,  1120,
    1121,  1122,  1126,  1127,  1128,  1132,  1133,  1138,  1139,  1144,
    1145,  1150,  1151,  1156,  1158,  1163,  1166,  1179,  1183,  1188,
    1190,  1181,  1198,  1201,  1203,  1207,  1208,  1207,  1217,  1262,
    1265,  1278,  1287,  1290,  1297,  1297,  1309,  1310,  1314,  1318,
    1327,  1327,  1341,  1341,  1351,  1352,  1356,  1360,  1364,  1371,
    1375,  1383,  1386,  1390,  1394,  1398,  1405,  1409,  1413,  1417,
    1422,  1421,  1435,  1434,  1444,  1448,  1452,  1456,  1460,  1464,
    1470,  1472
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "NAME", "LNAME", "'='", "OREQ",
  "ANDEQ", "RSHIFTEQ", "LSHIFTEQ", "DIVEQ", "MULTEQ", "MINUSEQ", "PLUSEQ",
  "'?'", "':'", "OROR", "ANDAND", "'|'", "'^'", "'&'", "NE", "EQ", "'<'",
  "'>'", "GE", "LE", "RSHIFT", "LSHIFT", "'+'", "'-'", "'*'", "'/'", "'%'",
  "UNARY", "END", "'('", "ALIGN_K", "BLOCK", "BIND", "QUAD", "SQUAD",
  "LONG", "SHORT", "BYTE", "SECTIONS", "PHDRS", "INSERT_K", "AFTER",
  "BEFORE", "DATA_SEGMENT_ALIGN", "DATA_SEGMENT_RELRO_END",
  "DATA_SEGMENT_END", "SORT_BY_NAME", "SORT_BY_ALIGNMENT", "SORT_NONE",
  "SORT_BY_INIT_PRIORITY", "'{'", "'}'", "SIZEOF_HEADERS", "OUTPUT_FORMAT",
  "FORCE_COMMON_ALLOCATION", "OUTPUT_ARCH", "INHIBIT_COMMON_ALLOCATION",
  "SEGMENT_START", "INCLUDE", "MEMORY", "REGION_ALIAS", "LD_FEATURE",
  "NOLOAD", "DSECT", "COPY", "INFO", "OVERLAY", "DEFINED", "TARGET_K",
  "SEARCH_DIR", "MAP", "ENTRY", "NEXT", "SIZEOF", "ALIGNOF", "ADDR",
  "LOADADDR", "MAX_K", "MIN_K", "STARTUP", "HLL", "SYSLIB", "FLOAT",
  "NOFLOAT", "NOCROSSREFS", "ORIGIN", "FILL", "LENGTH",
  "CREATE_OBJECT_SYMBOLS", "INPUT", "GROUP", "OUTPUT", "CONSTRUCTORS",
  "ALIGNMOD", "AT", "OVERFLOW_INTO", "SUBALIGN", "HIDDEN", "PROVIDE",
  "PROVIDE_HIDDEN", "AS_NEEDED", "CHIP", "LIST", "SECT", "ABSOLUTE",
  "LOAD", "NEWLINE", "ENDWORD", "ORDER", "NAMEWORD", "ASSERT_K",
  "LOG2CEIL", "FORMAT", "PUBLIC", "DEFSYMEND", "BASE", "ALIAS", "TRUNCATE",
  "REL", "INPUT_SCRIPT", "INPUT_MRI_SCRIPT", "INPUT_DEFSYM", "CASE",
  "EXTERN", "START", "VERS_TAG", "VERS_IDENTIFIER", "GLOBAL", "LOCAL",
  "VERSIONK", "INPUT_VERSION_SCRIPT", "KEEP", "ONLY_IF_RO", "ONLY_IF_RW",
  "SPECIAL", "INPUT_SECTION_FLAGS", "ALIGN_WITH_INPUT", "EXCLUDE_FILE",
  "CONSTANT", "INPUT_DYNAMIC_LIST", "','", "';'", "')'", "'['", "']'",
  "'!'", "'~'", "$accept", "file", "filename", "defsym_expr", "$@1",
  "mri_script_file", "$@2", "mri_script_lines", "mri_script_command",
  "$@3", "ordernamelist", "mri_load_name_list", "mri_abs_name_list",
  "casesymlist", "extern_name_list", "$@4", "extern_name_list_body",
  "script_file", "$@5", "ifile_list", "ifile_p1", "$@6", "$@7",
  "input_list", "@8", "@9", "@10", "sections", "sec_or_group_p1",
  "statement_anywhere", "$@11", "wildcard_name", "wildcard_spec",
  "sect_flag_list", "sect_flags", "exclude_name_list", "file_NAME_list",
  "input_section_spec_no_keep", "input_section_spec", "$@12", "statement",
  "$@13", "$@14", "statement_list", "statement_list_opt", "length",
  "fill_exp", "fill_opt", "assign_op", "end", "assignment", "opt_comma",
  "memory", "memory_spec_list_opt", "memory_spec_list", "memory_spec",
  "$@15", "$@16", "origin_spec", "length_spec", "attributes_opt",
  "attributes_list", "attributes_string", "startup", "high_level_library",
  "high_level_library_NAME_list", "low_level_library",
  "low_level_library_NAME_list", "floating_point_support",
  "nocrossref_list", "mustbe_exp", "$@17", "exp", "memspec_at_opt",
  "memspec_ovfl_opt", "opt_at", "opt_align", "opt_align_with_input",
  "opt_subalign", "sect_constraint", "section", "$@18", "$@19", "$@20",
  "$@21", "$@22", "$@23", "$@24", "$@25", "$@26", "$@27", "$@28", "$@29",
  "$@30", "type", "atype", "opt_exp_with_type", "opt_exp_without_type",
  "opt_nocrossrefs", "memspec_opt", "phdr_opt", "overlay_section", "$@31",
  "$@32", "$@33", "phdrs", "phdr_list", "phdr", "$@34", "$@35",
  "phdr_type", "phdr_qualifiers", "phdr_val", "dynamic_list_file", "$@36",
  "dynamic_list_nodes", "dynamic_list_node", "dynamic_list_tag",
  "version_script_file", "$@37", "version", "$@38", "vers_nodes",
  "vers_node", "verdep", "vers_tag", "vers_defns", "@39", "@40",
  "opt_semicolon", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,    61,   261,   262,   263,
     264,   265,   266,   267,   268,    63,    58,   269,   270,   124,
      94,    38,   271,   272,    60,    62,   273,   274,   275,   276,
      43,    45,    42,    47,    37,   277,   278,    40,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   123,   125,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,    44,    59,
      41,    91,    93,    33,   126
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   155,   156,   156,   156,   156,   156,   157,   159,   158,
     161,   160,   162,   162,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     164,   163,   163,   163,   165,   165,   165,   166,   166,   167,
     167,   168,   168,   168,   170,   169,   171,   171,   171,   173,
     172,   174,   174,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   176,   175,   175,   177,   175,   175,   175,   175,
     175,   175,   175,   178,   178,   178,   178,   178,   178,   179,
     178,   180,   178,   181,   178,   182,   183,   183,   183,   184,
     184,   185,   184,   186,   186,   186,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   188,   188,   189,
     190,   190,   191,   191,   192,   192,   192,   192,   192,   192,
     193,   194,   193,   195,   195,   195,   195,   195,   195,   195,
     195,   196,   195,   197,   195,   198,   198,   199,   199,   200,
     200,   200,   200,   200,   201,   202,   202,   203,   203,   203,
     203,   203,   203,   203,   203,   204,   204,   205,   205,   205,
     205,   205,   206,   206,   207,   208,   208,   209,   209,   211,
     210,   212,   210,   213,   214,   215,   215,   216,   216,   217,
     217,   218,   219,   219,   220,   220,   221,   222,   222,   223,
     223,   224,   224,   224,   226,   225,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   228,   228,   229,   229,   230,   230,
     231,   231,   232,   232,   233,   233,   234,   234,   234,   234,
     236,   237,   238,   239,   240,   235,   241,   242,   243,   244,
     245,   235,   246,   247,   235,   248,   235,   249,   249,   249,
     249,   249,   250,   250,   250,   251,   251,   251,   251,   252,
     252,   253,   253,   254,   254,   255,   255,   256,   257,   258,
     259,   256,   260,   261,   261,   263,   264,   262,   265,   266,
     266,   266,   267,   267,   269,   268,   270,   270,   271,   272,
     274,   273,   276,   275,   277,   277,   278,   278,   278,   279,
     279,   280,   280,   280,   280,   280,   281,   281,   281,   281,
     282,   281,   283,   281,   281,   281,   281,   281,   281,   281,
     284,   284
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     2,     2,     2,     1,     0,     4,
       0,     2,     3,     0,     2,     4,     1,     1,     2,     1,
       4,     4,     3,     2,     4,     3,     4,     4,     4,     4,
       4,     2,     2,     2,     4,     4,     2,     2,     2,     2,
       0,     5,     2,     0,     3,     2,     0,     1,     3,     1,
       3,     0,     1,     3,     0,     2,     1,     2,     3,     0,
       2,     2,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     4,     4,     4,     8,     4,     1,
       1,     4,     0,     5,     4,     0,     5,     4,     4,     3,
       3,     6,     4,     1,     3,     2,     1,     3,     2,     0,
       5,     0,     7,     0,     6,     4,     2,     2,     0,     4,
       2,     0,     7,     1,     1,     1,     1,     5,     4,     4,
       4,     7,     7,     7,     7,     8,     4,     1,     3,     4,
       2,     1,     3,     1,     1,     2,     3,     4,     4,     5,
       1,     0,     5,     2,     1,     1,     1,     4,     1,     4,
       4,     0,     8,     0,     5,     2,     1,     0,     1,     1,
       1,     1,     1,     1,     1,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     6,
       6,     6,     1,     0,     4,     1,     0,     3,     1,     0,
       7,     0,     5,     3,     3,     0,     3,     1,     2,     1,
       2,     4,     4,     3,     3,     1,     4,     3,     0,     1,
       1,     0,     2,     3,     0,     2,     2,     3,     4,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     5,     3,
       3,     4,     1,     1,     4,     4,     4,     4,     4,     4,
       4,     6,     6,     6,     4,     6,     4,     1,     6,     6,
       6,     4,     4,     4,     3,     0,     3,     0,     4,     0,
       4,     0,     1,     0,     4,     0,     1,     1,     1,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,    19,     0,     0,     7,     0,     5,     1,     1,     1,
       1,     1,     3,     0,     2,     3,     2,     6,    10,     2,
       1,     0,     1,     2,     0,     0,     3,     0,     0,     0,
       0,    11,     4,     0,     2,     0,     0,     6,     1,     0,
       3,     5,     0,     3,     0,     2,     1,     2,     4,     2,
       0,     2,     0,     5,     1,     2,     4,     5,     6,     1,
       2,     0,     2,     4,     4,     8,     1,     1,     3,     3,
       0,     9,     0,     7,     1,     3,     1,     3,     1,     3,
       0,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,    59,    10,     8,   340,   334,     0,     2,    62,     3,
      13,     6,     0,     4,     0,     5,     0,     1,    60,    11,
       0,   351,     0,   341,   344,     0,   335,   336,     0,     0,
       0,     0,     0,    79,     0,    80,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,   210,     0,
       0,    82,     0,     0,     0,     0,   111,     0,    72,    61,
      64,    70,     0,    63,    66,    67,    68,    69,    65,    71,
       0,    16,     0,     0,     0,     0,    17,     0,     0,     0,
      19,    46,     0,     0,     0,     0,     0,     0,    51,    54,
       0,     0,     0,   357,   368,   356,   364,   366,     0,     0,
     351,   345,   364,   366,     0,     0,   337,   214,   174,   173,
     172,   171,   170,   169,   168,   167,   214,   108,   323,     0,
       0,     0,     0,     7,    85,   186,     0,     0,     0,     0,
       0,     0,     0,     0,   208,   211,     0,     0,     0,     0,
       0,     0,     0,    54,   176,   175,   110,     0,     0,    40,
       0,   242,   257,     0,     0,     0,     0,     0,     0,     0,
       0,   243,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    49,    31,    47,    32,    18,    33,    23,     0,    36,
       0,    37,    52,    38,    39,     0,    42,    12,     9,     0,
       0,     0,     0,   352,     0,     0,   339,   177,     0,   178,
       0,     0,    89,    90,     0,     0,    62,   189,     0,     0,
     183,   188,     0,     0,     0,     0,     0,     0,     0,   203,
     205,   183,   183,   211,     0,    93,    96,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
       0,     0,   220,   216,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   219,   221,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
       0,     0,    45,     0,     0,     0,    22,     0,     0,    56,
      55,   362,     0,     0,   346,   359,   369,   358,   365,   367,
       0,   338,   215,   280,   105,     0,   286,   292,   107,   106,
     325,   322,   324,     0,    76,    78,   342,   195,   191,   184,
     182,     0,     0,    92,    73,    74,    84,   109,   201,   202,
       0,   206,     0,   211,   212,    87,    99,    95,    98,     0,
       0,    81,     0,    75,   214,   214,   214,     0,    88,     0,
      27,    28,    43,    29,    30,   217,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   240,   239,   237,
     236,   235,   230,   229,   233,   234,   232,   231,   228,   227,
     225,   226,   222,   223,   224,    15,    26,    24,    50,    48,
      44,    20,    21,    35,    34,    53,    57,     0,     0,   353,
     354,     0,   349,   347,     0,   303,   295,     0,   303,     0,
       0,    86,     0,     0,   186,   187,     0,   204,   207,   213,
       0,   103,    94,    97,     0,    83,     0,     0,     0,     0,
     343,    41,     0,   250,   256,     0,     0,   254,     0,   241,
     218,   245,   244,   246,   247,     0,     0,   261,   262,   249,
       0,   263,   248,     0,    58,   370,   367,   360,   350,   348,
       0,     0,   303,     0,   269,   108,   310,     0,   311,   293,
     328,   329,     0,   199,     0,     0,   197,     0,     0,    91,
       0,     0,   101,   179,   180,   181,     0,     0,     0,     0,
       0,     0,     0,     0,   238,   371,     0,     0,     0,   297,
     298,   299,   300,   301,   304,     0,     0,     0,     0,   306,
       0,   271,     0,   309,   312,   269,     0,   332,     0,   326,
       0,   200,   196,   198,     0,   183,   192,   100,     0,     0,
     112,   251,   252,   253,   255,   258,   259,   260,   363,     0,
     370,   302,     0,   305,     0,     0,   273,   296,   275,   108,
       0,   329,     0,     0,    77,   214,     0,   104,     0,   355,
       0,   303,     0,     0,   272,   275,     0,   287,     0,     0,
     330,     0,   327,   193,     0,   190,   102,   361,     0,     0,
     268,     0,   281,     0,     0,   294,   333,   329,   214,     0,
     307,   270,   279,     0,   288,   331,   194,     0,   276,   277,
     278,     0,   274,   317,   303,   282,     0,     0,   157,   318,
     289,   308,   134,   115,   114,   159,   160,   161,   162,   163,
       0,     0,     0,     0,     0,     0,   144,   146,   151,     0,
       0,     0,   145,     0,   116,     0,     0,   140,   148,   156,
     158,     0,     0,     0,     0,   314,     0,     0,     0,     0,
     153,   214,     0,   141,     0,     0,   113,     0,   133,   183,
       0,   135,     0,     0,   155,   283,   214,   143,   157,     0,
     265,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   157,     0,   164,     0,     0,   127,     0,   131,     0,
       0,   136,     0,   183,   183,     0,   314,     0,     0,   313,
       0,   315,     0,     0,   147,     0,   118,     0,     0,   119,
     120,   126,     0,   150,     0,   113,     0,     0,   129,     0,
     130,   132,   138,   137,   183,   265,   149,   319,     0,   166,
       0,     0,     0,     0,     0,   154,     0,   142,   128,   117,
     139,   315,   315,   264,   214,     0,   267,     0,     0,     0,
       0,     0,     0,   166,   166,   165,   316,     0,   290,   122,
     121,     0,   123,   124,     0,   267,   320,     0,   183,   125,
     152,   284,   183,   266,   291,   183,   321,   285
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     6,   124,    11,    12,     9,    10,    19,    91,   249,
     185,   184,   182,   193,   194,   195,   310,     7,     8,    18,
      59,   137,   216,   238,   450,   559,   511,    60,   210,   328,
     142,   664,   665,   717,   666,   719,   689,   667,   668,   715,
     669,   682,   711,   670,   671,   672,   712,   776,   116,   146,
      62,   722,    63,   219,   220,   221,   337,   444,   555,   605,
     443,   505,   506,    64,    65,   231,    66,   232,    67,   234,
     713,   208,   254,   731,   788,   541,   576,   595,   597,   631,
     329,   435,   622,   638,   726,   805,   437,   614,   633,   675,
     798,   438,   546,   495,   535,   493,   494,   498,   545,   700,
     759,   636,   674,   772,   802,    68,   211,   332,   439,   583,
     501,   549,   581,    15,    16,    26,    27,   104,    13,    14,
      69,    70,    23,    24,   434,    98,    99,   528,   428,   526
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -652
static const yytype_int16 yypact[] =
{
     245,  -652,  -652,  -652,  -652,  -652,    41,  -652,  -652,  -652,
    -652,  -652,    63,  -652,   -18,  -652,    43,  -652,   863,  1696,
      44,   102,    64,   -18,  -652,   114,    43,  -652,   624,    89,
     137,   128,   101,  -652,   117,  -652,    77,   169,   148,   210,
     239,   244,   247,   251,   258,   262,   278,  -652,  -652,   291,
     300,  -652,   309,   314,   316,   324,  -652,   328,  -652,  -652,
    -652,  -652,   109,  -652,  -652,  -652,  -652,  -652,  -652,  -652,
     230,  -652,   371,    77,   372,   697,  -652,   376,   377,   381,
    -652,  -652,   382,   391,   393,   697,   394,   396,   398,  -652,
     399,   280,   697,  -652,   400,  -652,   395,   410,   347,   261,
     102,  -652,  -652,  -652,   348,   265,  -652,  -652,  -652,  -652,
    -652,  -652,  -652,  -652,  -652,  -652,  -652,  -652,  -652,   411,
     423,   424,   425,  -652,  -652,    27,   426,   427,   429,    77,
      77,   432,    77,    16,  -652,   433,    34,   401,    77,   435,
     437,   438,   406,  -652,  -652,  -652,  -652,   386,    31,  -652,
      46,  -652,  -652,   697,   697,   697,   412,   414,   415,   416,
     418,  -652,   419,   420,   421,   428,   430,   431,   439,   440,
     463,   464,   465,   466,   469,   471,   473,   697,   697,  1484,
     444,  -652,   312,  -652,   315,    19,  -652,  -652,   529,  1748,
     318,  -652,  -652,   321,  -652,   458,  -652,  -652,  1748,   453,
     114,   114,   363,   170,   455,   373,   170,  -652,   697,  -652,
     366,    57,  -652,  -652,    13,   320,  -652,  -652,    77,   456,
      40,  -652,   375,   370,   384,   392,   404,   405,   407,  -652,
    -652,    96,   103,    22,   408,  -652,  -652,   484,    92,    34,
     422,   525,   530,   534,   697,   443,   -18,   697,   697,  -652,
     697,   697,  -652,  -652,   873,   697,   697,   697,   697,   697,
     537,   557,   697,   560,   561,   565,   566,   697,   697,   567,
     569,   697,   697,   697,   570,  -652,  -652,   697,   697,   697,
     697,   697,   697,   697,   697,   697,   697,   697,   697,   697,
     697,   697,   697,   697,   697,   697,   697,   697,   697,  1748,
     571,   573,  -652,   574,   697,   697,  1748,   299,   581,  -652,
      23,  -652,   446,   447,  -652,  -652,   583,  -652,  -652,  -652,
     -22,  -652,  1748,   624,  -652,    77,  -652,  -652,  -652,  -652,
    -652,  -652,  -652,   584,  -652,  -652,   929,   506,  -652,  -652,
    -652,    27,   595,  -652,  -652,  -652,  -652,  -652,  -652,  -652,
      77,  -652,    77,   433,  -652,  -652,  -652,  -652,  -652,   563,
      49,  -652,   116,  -652,  -652,  -652,  -652,  1504,  -652,   -25,
    1748,  1748,  1721,  1748,  1748,  -652,  1062,  1082,  1524,  1544,
    1102,   454,   451,  1122,   457,   468,   470,   475,  1564,  1616,
     476,   477,  1142,  1636,  1162,   478,  1885,  1843,  1042,  1579,
    1310,  1175,   828,   828,   460,   460,   460,   460,   390,   390,
     275,   275,  -652,  -652,  -652,  1748,  1748,  1748,  -652,  -652,
    -652,  1748,  1748,  -652,  -652,  -652,  -652,   599,   114,   182,
     170,   547,  -652,  -652,    -8,     5,  -652,   613,     5,   697,
     492,  -652,     3,   590,    27,  -652,   495,  -652,  -652,  -652,
      34,  -652,  -652,  -652,   582,  -652,   496,   499,   503,   604,
    -652,  -652,   697,  -652,  -652,   697,   697,  -652,   697,  -652,
    -652,  -652,  -652,  -652,  -652,   697,   697,  -652,  -652,  -652,
     617,  -652,  -652,   697,  -652,   474,   626,  -652,  -652,  -652,
     259,   618,  1718,   638,   554,  -652,  -652,  1905,   575,  -652,
    1748,    24,   653,  -652,   654,     2,  -652,   577,   623,  -652,
     124,    34,  -652,  -652,  -652,  -652,   518,  1196,  1216,  1236,
    1256,  1276,  1296,   521,  1748,   170,   615,   114,   114,  -652,
    -652,  -652,  -652,  -652,  -652,   522,   697,   285,   660,  -652,
     642,   643,   380,  -652,  -652,   554,   622,   647,   648,  -652,
     536,  -652,  -652,  -652,   681,   541,  -652,  -652,   127,    34,
    -652,  -652,  -652,  -652,  -652,  -652,  -652,  -652,  -652,   542,
     474,  -652,  1330,  -652,   697,   655,   546,  -652,   598,  -652,
     697,    24,   697,   556,  -652,  -652,   612,  -652,   135,   170,
     650,   162,  1350,   697,  -652,   598,   673,  -652,  1781,  1370,
    -652,  1390,  -652,  -652,   705,  -652,  -652,  -652,   675,   698,
    -652,  1410,  -652,   697,   657,  -652,  -652,    24,  -652,   697,
    -652,  -652,   180,  1430,  -652,  -652,  -652,  1464,  -652,  -652,
    -652,   662,  -652,  -652,   676,  -652,    60,   702,   769,  -652,
    -652,  -652,   539,  -652,  -652,  -652,  -652,  -652,  -652,  -652,
     686,   687,   689,   692,    77,   693,  -652,  -652,  -652,   700,
     701,   703,  -652,   237,  -652,   704,    80,  -652,  -652,  -652,
     769,   680,   706,   109,   684,   719,   334,   364,    51,    51,
    -652,  -652,   708,  -652,   743,    51,  -652,   714,  -652,    -2,
     237,   715,   237,   716,  -652,  -652,  -652,  -652,   769,   750,
     656,   718,   723,   606,   724,   614,   728,   731,   619,   620,
     621,   769,   625,  -652,   697,    17,  -652,    -5,  -652,    14,
     224,  -652,   237,   120,     1,   237,   719,   635,   717,  -652,
     749,  -652,    51,    51,  -652,    51,  -652,    51,    51,  -652,
    -652,  -652,   751,  -652,  1656,   636,   639,   784,  -652,    51,
    -652,  -652,  -652,  -652,   123,   656,  -652,  -652,   787,    98,
     644,   645,    15,   646,   649,  -652,   789,  -652,  -652,  -652,
    -652,  -652,  -652,  -652,  -652,   793,   695,   652,   658,    51,
     667,   668,   669,    98,    98,  -652,  -652,   775,  -652,  -652,
    -652,   670,  -652,  -652,   109,   695,  -652,   799,   541,  -652,
    -652,  -652,   541,  -652,  -652,   541,  -652,  -652
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -652,  -652,   -70,  -652,  -652,  -652,  -652,   555,  -652,  -652,
    -652,  -652,  -652,  -652,   663,  -652,  -652,  -652,  -652,   589,
    -652,  -652,  -652,  -224,  -652,  -652,  -652,  -652,  -470,   -13,
    -652,   -16,  -615,  -652,  -652,    72,  -425,   106,  -652,  -652,
     152,  -652,  -652,  -652,  -629,  -652,    53,  -458,  -652,  -651,
    -384,  -219,  -652,   385,  -652,   487,  -652,  -652,  -652,  -652,
    -652,  -652,   325,  -652,  -652,  -652,  -652,  -652,  -652,  -209,
    -105,  -652,   -75,    76,    37,   288,  -652,  -652,   241,  -652,
    -652,  -652,  -652,  -652,  -652,  -652,  -652,  -652,  -652,  -652,
    -652,  -652,  -652,  -652,  -652,  -400,   402,  -652,  -652,   108,
    -424,  -652,  -652,  -652,  -652,  -652,  -652,  -652,  -652,  -652,
    -652,  -487,  -652,  -652,  -652,  -652,   811,  -652,  -652,  -652,
    -652,  -652,   592,   -19,  -652,   739,   -11,  -652,  -652,   271
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -343
static const yytype_int16 yytable[] =
{
     179,   341,   207,   149,   101,    61,   503,   503,   151,   152,
     189,   209,   350,   352,   105,   362,   747,   198,   686,   686,
     123,   745,   697,   302,   354,   542,   233,   426,   547,   643,
     643,   217,   643,    21,   460,   153,   154,   247,   235,   236,
      21,    17,   490,   156,   157,   491,   644,   644,   688,   644,
      92,   693,   250,   452,   453,   686,   158,   159,   160,   225,
     226,   330,   228,   230,   639,   161,   643,    20,   240,   728,
     162,   687,   651,   652,   653,   688,  -185,   688,   252,   253,
     163,   123,   742,   644,   691,   164,   165,   166,   167,   168,
     169,   170,   538,   218,   600,   643,   357,   358,   171,  -185,
     172,    25,   275,   276,   774,   299,    93,   751,    22,   598,
     688,   432,   644,   306,   775,    22,   331,   173,    93,   640,
     357,   358,   100,   174,   175,   488,   548,   433,   357,   358,
     625,   357,   358,   322,   687,   651,   652,   653,   121,   357,
     358,   489,   237,   800,   449,   748,   340,   117,   338,   340,
     721,   176,   552,   753,   122,   504,   504,   454,   177,   178,
     660,   333,   661,   334,   749,   779,   229,   303,   663,   367,
     353,   427,   370,   371,   315,   373,   374,   119,   120,   248,
     376,   377,   378,   379,   380,   126,   315,   383,   340,   312,
     313,   609,   388,   389,   251,   118,   392,   393,   394,   537,
     359,   608,   396,   397,   398,   399,   400,   401,   402,   403,
     404,   405,   406,   407,   408,   409,   410,   411,   412,   413,
     414,   415,   416,   417,   359,   661,   510,   125,   686,   421,
     422,   692,   359,    94,   637,   359,    95,    96,    97,   643,
     360,   686,   361,   359,   340,    94,   349,   127,    95,   102,
     103,   340,   643,   351,   673,   436,   644,   144,   145,   456,
     457,   458,   151,   152,   360,   723,   455,   724,   340,   644,
     752,   340,   360,   770,   557,   360,   128,   587,   701,   702,
     447,   129,   448,   360,   130,   606,   673,   558,   131,   153,
     154,   687,   651,   652,   653,   132,   155,   156,   157,   133,
     754,   316,   423,   424,   317,   318,   319,   293,   294,   295,
     158,   159,   160,   316,   673,   134,   317,   318,   486,   161,
     628,   629,   630,    61,   162,   795,   796,   673,   135,   529,
     530,   531,   532,   533,   163,   588,   586,   136,   686,   164,
     165,   166,   167,   168,   169,   170,   138,   783,   784,   643,
     101,   139,   171,   140,   172,   529,   530,   531,   532,   533,
     492,   141,   497,   492,   500,   143,   644,   147,   686,   704,
     323,   173,     1,     2,     3,   148,   150,   174,   175,   643,
     180,   181,   661,     4,   323,   183,   186,   517,   701,   702,
     518,   519,     5,   520,   197,   187,   644,   188,   190,   191,
     521,   522,   192,   196,   199,   176,   202,   205,   524,   534,
     203,   200,   177,   178,   206,   212,   577,   485,   706,   707,
     291,   292,   293,   294,   295,   324,   201,   213,   214,   215,
     222,   223,   325,   224,   703,   534,   227,   233,   239,   241,
     326,   242,   243,   244,   246,    43,   325,   151,   152,   255,
     297,   256,   257,   258,   326,   259,   260,   261,   262,    43,
     300,   572,   309,   301,   327,   263,   307,   264,   265,   308,
     335,    53,    54,    55,   153,   154,   266,   267,   327,   704,
     603,   155,   156,   157,    56,    53,    54,    55,   289,   290,
     291,   292,   293,   294,   295,   158,   159,   160,    56,   592,
     268,   269,   270,   271,   161,   599,   272,   601,   273,   162,
     274,   311,   314,   626,   320,   339,   569,   570,   611,   163,
     343,   356,   321,   342,   164,   165,   166,   167,   168,   169,
     170,   364,   151,   152,   344,   304,   365,   171,   623,   172,
     366,   381,   345,   442,   627,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   346,   347,   173,   348,   355,   153,
     154,   382,   174,   175,   384,   385,   155,   156,   157,   386,
     387,   390,   363,   391,   395,   418,  -113,   419,   420,   804,
     158,   159,   160,   806,   680,   425,   807,   431,   440,   161,
     176,   727,   298,   368,   162,   429,   430,   177,   178,   446,
     451,   469,   468,   484,   163,   487,   507,   471,   516,   164,
     165,   166,   167,   168,   169,   170,   151,   152,   472,   512,
     473,   523,   171,   525,   172,   474,   477,   478,   482,   496,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   744,
     502,   173,   527,   153,   154,   509,   513,   174,   175,   514,
     155,   156,   157,   515,   539,   536,   540,   550,   551,   556,
     705,   708,   709,   710,   158,   159,   160,   544,   560,   718,
     554,   567,   571,   161,   568,   176,   573,   305,   162,   574,
     579,   575,   177,   178,   580,   582,   584,   585,   163,   340,
     594,   589,   593,   164,   165,   166,   167,   168,   169,   170,
     151,   152,   596,   750,   705,   602,   171,   604,   172,   607,
     613,   618,   619,   537,   620,   624,   760,   761,   641,   718,
     635,   763,   764,   676,   677,   173,   678,   153,   154,   679,
     681,   174,   175,   769,   155,   156,   157,   683,   684,   695,
     685,   690,   698,   696,   699,   714,   750,   716,   158,   159,
     160,   720,  -113,   725,   729,   732,   734,   161,   730,   176,
     733,   735,   162,   791,   736,   737,   177,   178,   738,   739,
     740,   741,   163,   642,   758,   743,   757,   164,   165,   166,
     167,   168,   169,   170,   643,   756,  -134,   765,   768,   767,
     171,   773,   172,   782,   777,   778,   780,   786,   787,   781,
     797,   644,   789,   803,   372,   336,   245,   762,   790,   173,
     645,   646,   647,   648,   649,   174,   175,   792,   793,   794,
     799,   746,   694,   650,   651,   652,   653,   785,   445,   508,
     553,   771,   801,   578,   755,   654,   612,   106,   369,   204,
     499,   590,     0,   176,     0,     0,     0,     0,     0,     0,
     177,   178,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   655,     0,   656,     0,    28,     0,   657,
       0,     0,     0,     0,    53,    54,    55,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   658,   277,     0,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   659,    29,
      30,    31,   660,     0,   661,     0,     0,     0,   662,     0,
     663,     0,     0,     0,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    28,     0,     0,     0,     0,     0,    40,
      41,    42,    43,     0,     0,     0,     0,     0,     0,     0,
      44,    45,    46,    47,    48,    49,     0,     0,     0,     0,
      50,    51,    52,     0,     0,   441,     0,     0,    53,    54,
      55,     0,     0,     0,     0,    29,    30,    31,     0,     0,
       0,    56,     0,     0,     0,     0,     0,     0,     0,     0,
      32,    33,    34,    35,    57,    36,    37,    38,    39,     0,
    -342,     0,     0,     0,     0,    40,    41,    42,    43,     0,
       0,     0,    58,     0,     0,     0,    44,    45,    46,    47,
      48,    49,     0,   375,     0,     0,    50,    51,    52,     0,
       0,     0,     0,     0,    53,    54,    55,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    56,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      57,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   277,    58,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   277,     0,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   277,     0,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   277,     0,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   277,     0,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   277,     0,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     462,   277,   463,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   277,   464,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   277,   467,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   277,   470,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   277,   479,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   277,   481,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   277,   561,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   277,   562,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   277,   563,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   277,   564,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   277,   565,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   277,   566,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   277,
     591,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   277,
     610,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   277,
     616,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   277,
     617,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   277,
     621,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   277,
     632,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   634,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   277,   296,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   277,   459,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   277,   465,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,     0,   466,     0,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   475,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
       0,     0,     0,   277,    72,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,     0,     0,   537,     0,   461,     0,    72,
       0,     0,    73,   277,   476,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,     0,   480,   323,     0,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,     0,     0,   766,    75,    76,    77,    78,    79,
     -43,    80,    81,    82,     0,     0,    83,    84,     0,    85,
      86,    87,    74,     0,     0,     0,    88,    89,    90,     0,
      75,    76,    77,    78,    79,     0,    80,    81,    82,     0,
     615,    83,    84,     0,    85,    86,    87,   325,     0,     0,
       0,    88,    89,    90,     0,   326,     0,     0,     0,     0,
      43,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,     0,   327,
       0,     0,     0,     0,     0,     0,    53,    54,    55,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    56,
     277,   483,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     277,   543,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295
};

static const yytype_int16 yycheck[] =
{
      75,   220,   107,    73,    23,    18,     4,     4,     3,     4,
      85,   116,   231,   232,    25,   239,    21,    92,     4,     4,
       4,     4,   673,     4,   233,   495,     4,     4,     4,    15,
      15,     4,    15,    58,    59,    30,    31,     6,     4,     5,
      58,     0,    37,    38,    39,    40,    32,    32,   663,    32,
       6,   666,     6,     4,     5,     4,    51,    52,    53,   129,
     130,     4,   132,   133,     4,    60,    15,     4,   138,   698,
      65,    54,    55,    56,    57,   690,    36,   692,   153,   154,
      75,     4,   711,    32,     4,    80,    81,    82,    83,    84,
      85,    86,   492,    66,   581,    15,     4,     5,    93,    59,
      95,    58,   177,   178,     6,   180,     4,   722,   133,   579,
     725,   133,    32,   188,    16,   133,    59,   112,     4,    59,
       4,     5,    58,   118,   119,   133,   102,   149,     4,     5,
     617,     4,     5,   208,    54,    55,    56,    57,    37,     4,
       5,   149,   108,   794,   353,   150,   148,    58,   218,   148,
     152,   146,   150,   152,    37,   153,   153,   108,   153,   154,
     143,   148,   145,   150,   150,   150,   150,   148,   151,   244,
     148,   148,   247,   248,     4,   250,   251,    49,    50,   148,
     255,   256,   257,   258,   259,    37,     4,   262,   148,   200,
     201,   591,   267,   268,   148,    58,   271,   272,   273,    37,
     108,    39,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   108,   145,   450,    58,     4,   304,
     305,   151,   108,   131,   634,   108,   134,   135,   136,    15,
     148,     4,   150,   108,   148,   131,   150,    37,   134,   135,
     136,   148,    15,   150,   638,   325,    32,   148,   149,   364,
     365,   366,     3,     4,   148,   690,   150,   692,   148,    32,
     150,   148,   148,   150,   150,   148,    37,   150,    54,    55,
     350,    37,   352,   148,    37,   150,   670,   511,    37,    30,
      31,    54,    55,    56,    57,    37,    37,    38,    39,    37,
     725,   131,     3,     4,   134,   135,   136,    32,    33,    34,
      51,    52,    53,   131,   698,    37,   134,   135,   136,    60,
     140,   141,   142,   336,    65,   783,   784,   711,    37,    70,
      71,    72,    73,    74,    75,   559,   555,    37,     4,    80,
      81,    82,    83,    84,    85,    86,    37,   771,   772,    15,
     369,    37,    93,    37,    95,    70,    71,    72,    73,    74,
     435,    37,   437,   438,   439,    37,    32,   137,     4,   145,
       4,   112,   127,   128,   129,     4,     4,   118,   119,    15,
       4,     4,   145,   138,     4,     4,     4,   462,    54,    55,
     465,   466,   147,   468,   114,     4,    32,     4,     4,     3,
     475,   476,     4,     4,     4,   146,    59,    59,   483,   150,
     149,    16,   153,   154,   149,     4,    36,   428,    54,    55,
      30,    31,    32,    33,    34,    59,    16,     4,     4,     4,
       4,     4,    66,     4,   100,   150,     4,     4,    37,     4,
      74,     4,     4,    37,    58,    79,    66,     3,     4,    37,
       6,    37,    37,    37,    74,    37,    37,    37,    37,    79,
     148,   536,     4,   148,    98,    37,   148,    37,    37,   148,
     150,   105,   106,   107,    30,    31,    37,    37,    98,   145,
     585,    37,    38,    39,   118,   105,   106,   107,    28,    29,
      30,    31,    32,    33,    34,    51,    52,    53,   118,   574,
      37,    37,    37,    37,    60,   580,    37,   582,    37,    65,
      37,    58,   149,   618,    59,    59,   527,   528,   593,    75,
     150,    37,   149,   148,    80,    81,    82,    83,    84,    85,
      86,     6,     3,     4,   150,     6,     6,    93,   613,    95,
       6,     4,   150,    37,   619,     6,     7,     8,     9,    10,
      11,    12,    13,    14,   150,   150,   112,   150,   150,    30,
      31,     4,   118,   119,     4,     4,    37,    38,    39,     4,
       4,     4,   150,     4,     4,     4,    37,     4,     4,   798,
      51,    52,    53,   802,   654,     4,   805,     4,     4,    60,
     146,   696,   148,   150,    65,   149,   149,   153,   154,     4,
      37,   150,   148,     4,    75,    58,    16,   150,     4,    80,
      81,    82,    83,    84,    85,    86,     3,     4,   150,    37,
     150,     4,    93,   149,    95,   150,   150,   150,   150,    16,
       6,     7,     8,     9,    10,    11,    12,    13,    14,   714,
     148,   112,    16,    30,    31,   150,   150,   118,   119,   150,
      37,    38,    39,   150,    16,    37,   102,     4,     4,    36,
     676,   677,   678,   679,    51,    52,    53,    92,   150,   685,
      93,   150,   150,    60,    59,   146,    16,   148,    65,    37,
      58,    38,   153,   154,    37,    37,   150,     6,    75,   148,
     144,   149,    37,    80,    81,    82,    83,    84,    85,    86,
       3,     4,   104,   719,   720,   149,    93,    95,    95,    59,
      37,     6,    37,    37,    16,    58,   732,   733,    16,   735,
      58,   737,   738,    37,    37,   112,    37,    30,    31,    37,
      37,   118,   119,   749,    37,    38,    39,    37,    37,    59,
      37,    37,    58,    37,    25,    37,   762,     4,    51,    52,
      53,    37,    37,    37,     4,    37,   150,    60,   102,   146,
      37,    37,    65,   779,   150,    37,   153,   154,    37,   150,
     150,   150,    75,     4,    25,   150,    59,    80,    81,    82,
      83,    84,    85,    86,    15,   150,   150,    36,     4,   150,
      93,     4,    95,     4,   150,   150,   150,     4,   103,   150,
      25,    32,   150,     4,   249,   216,   143,   735,   150,   112,
      41,    42,    43,    44,    45,   118,   119,   150,   150,   150,
     150,   715,   670,    54,    55,    56,    57,   774,   341,   444,
     505,   755,   795,   545,   726,    66,   595,    26,   246,   100,
     438,   570,    -1,   146,    -1,    -1,    -1,    -1,    -1,    -1,
     153,   154,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    94,    -1,    96,    -1,     4,    -1,   100,
      -1,    -1,    -1,    -1,   105,   106,   107,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    15,    -1,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,   139,    46,
      47,    48,   143,    -1,   145,    -1,    -1,    -1,   149,    -1,
     151,    -1,    -1,    -1,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,     4,    -1,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    -1,    -1,    -1,    -1,
      97,    98,    99,    -1,    -1,    36,    -1,    -1,   105,   106,
     107,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,    -1,
      -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      61,    62,    63,    64,   131,    66,    67,    68,    69,    -1,
     137,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    -1,   150,    -1,    -1,    97,    98,    99,    -1,
      -1,    -1,    -1,    -1,   105,   106,   107,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,   149,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,    -1,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,    -1,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,    -1,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,    -1,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,    -1,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
     148,    15,   150,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    15,   150,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    15,   150,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    15,   150,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    15,   150,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    15,   150,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    15,   150,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    15,   150,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    15,   150,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    15,   150,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    15,   150,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    15,   150,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,
     150,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    15,
     150,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    15,
     150,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    15,
     150,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    15,
     150,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    15,
     150,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,   150,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    15,   148,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    15,   148,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    15,   148,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    38,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    -1,    37,    -1,    36,    -1,    38,
      -1,    -1,    66,    15,   148,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,   148,     4,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   109,   110,   111,   112,   113,
     114,   115,   116,   117,    -1,    -1,   120,   121,    -1,   123,
     124,   125,   101,    -1,    -1,    -1,   130,   131,   132,    -1,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,    -1,
      59,   120,   121,    -1,   123,   124,   125,    66,    -1,    -1,
      -1,   130,   131,   132,    -1,    74,    -1,    -1,    -1,    -1,
      79,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,   105,   106,   107,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   127,   128,   129,   138,   147,   156,   172,   173,   160,
     161,   158,   159,   273,   274,   268,   269,     0,   174,   162,
       4,    58,   133,   277,   278,    58,   270,   271,     4,    46,
      47,    48,    61,    62,    63,    64,    66,    67,    68,    69,
      76,    77,    78,    79,    87,    88,    89,    90,    91,    92,
      97,    98,    99,   105,   106,   107,   118,   131,   149,   175,
     182,   184,   205,   207,   218,   219,   221,   223,   260,   275,
     276,     4,    38,    66,   101,   109,   110,   111,   112,   113,
     115,   116,   117,   120,   121,   123,   124,   125,   130,   131,
     132,   163,     6,     4,   131,   134,   135,   136,   280,   281,
      58,   278,   135,   136,   272,   281,   271,     6,     7,     8,
       9,    10,    11,    12,    13,    14,   203,    58,    58,    49,
      50,    37,    37,     4,   157,    58,    37,    37,    37,    37,
      37,    37,    37,    37,    37,    37,    37,   176,    37,    37,
      37,    37,   185,    37,   148,   149,   204,   137,     4,   157,
       4,     3,     4,    30,    31,    37,    38,    39,    51,    52,
      53,    60,    65,    75,    80,    81,    82,    83,    84,    85,
      86,    93,    95,   112,   118,   119,   146,   153,   154,   227,
       4,     4,   167,     4,   166,   165,     4,     4,     4,   227,
       4,     3,     4,   168,   169,   170,     4,   114,   227,     4,
      16,    16,    59,   149,   280,    59,   149,   225,   226,   225,
     183,   261,     4,     4,     4,     4,   177,     4,    66,   208,
     209,   210,     4,     4,     4,   157,   157,     4,   157,   150,
     157,   220,   222,     4,   224,     4,     5,   108,   178,    37,
     157,     4,     4,     4,    37,   169,    58,     6,   148,   164,
       6,   148,   227,   227,   227,    37,    37,    37,    37,    37,
      37,    37,    37,    37,    37,    37,    37,    37,    37,    37,
      37,    37,    37,    37,    37,   227,   227,    15,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,   148,     6,   148,   227,
     148,   148,     4,   148,     6,   148,   227,   148,   148,     4,
     171,    58,   281,   281,   149,     4,   131,   134,   135,   136,
      59,   149,   227,     4,    59,    66,    74,    98,   184,   235,
       4,    59,   262,   148,   150,   150,   174,   211,   157,    59,
     148,   206,   148,   150,   150,   150,   150,   150,   150,   150,
     206,   150,   206,   148,   224,   150,    37,     4,     5,   108,
     148,   150,   178,   150,     6,     6,     6,   227,   150,   277,
     227,   227,   162,   227,   227,   150,   227,   227,   227,   227,
     227,     4,     4,   227,     4,     4,     4,     4,   227,   227,
       4,     4,   227,   227,   227,     4,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,     4,     4,
       4,   227,   227,     3,     4,     4,     4,   148,   283,   149,
     149,     4,   133,   149,   279,   236,   157,   241,   246,   263,
       4,    36,    37,   215,   212,   210,     4,   157,   157,   224,
     179,    37,     4,     5,   108,   150,   225,   225,   225,   148,
      59,    36,   148,   150,   150,   148,   148,   150,   148,   150,
     150,   150,   150,   150,   150,   148,   148,   150,   150,   150,
     148,   150,   150,    16,     4,   281,   136,    58,   133,   149,
      37,    40,   227,   250,   251,   248,    16,   227,   252,   251,
     227,   265,   148,     4,   153,   216,   217,    16,   208,   150,
     178,   181,    37,   150,   150,   150,     4,   227,   227,   227,
     227,   227,   227,     4,   227,   149,   284,    16,   282,    70,
      71,    72,    73,    74,   150,   249,    37,    37,   250,    16,
     102,   230,   183,    16,    92,   253,   247,     4,   102,   266,
       4,     4,   150,   217,    93,   213,    36,   150,   178,   180,
     150,   150,   150,   150,   150,   150,   150,   150,    59,   281,
     281,   150,   227,    16,    37,    38,   231,    36,   230,    58,
      37,   267,    37,   264,   150,     6,   206,   150,   178,   149,
     284,   150,   227,    37,   144,   232,   104,   233,   183,   227,
     266,   227,   149,   225,    95,   214,   150,    59,    39,   250,
     150,   227,   233,    37,   242,    59,   150,   150,     6,    37,
      16,   150,   237,   227,    58,   266,   225,   227,   140,   141,
     142,   234,   150,   243,   150,    58,   256,   250,   238,     4,
      59,    16,     4,    15,    32,    41,    42,    43,    44,    45,
      54,    55,    56,    57,    66,    94,    96,   100,   118,   139,
     143,   145,   149,   151,   186,   187,   189,   192,   193,   195,
     198,   199,   200,   205,   257,   244,    37,    37,    37,    37,
     157,    37,   196,    37,    37,    37,     4,    54,   187,   191,
      37,     4,   151,   187,   195,    59,    37,   204,    58,    25,
     254,    54,    55,   100,   145,   186,    54,    55,   186,   186,
     186,   197,   201,   225,    37,   194,     4,   188,   186,   190,
      37,   152,   206,   191,   191,    37,   239,   225,   199,     4,
     102,   228,    37,    37,   150,    37,   150,    37,    37,   150,
     150,   150,   199,   150,   227,     4,   192,    21,   150,   150,
     186,   187,   150,   152,   191,   254,   150,    59,    25,   255,
     186,   186,   190,   186,   186,    36,   148,   150,     4,   186,
     150,   228,   258,     4,     6,    16,   202,   150,   150,   150,
     150,   150,     4,   255,   255,   201,     4,   103,   229,   150,
     150,   186,   150,   150,   150,   202,   202,    25,   245,   150,
     204,   229,   259,     4,   206,   240,   206,   206
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
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



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 8:

/* Line 1464 of yacc.c  */
#line 182 "ldgram.y"
    { ldlex_defsym(); }
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 184 "ldgram.y"
    {
		  ldlex_popstate();
		  lang_add_assignment (exp_defsym ((yyvsp[(2) - (4)].name), (yyvsp[(4) - (4)].etree)));
		}
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 192 "ldgram.y"
    {
		  ldlex_mri_script ();
		  PUSH_ERROR (_("MRI style script"));
		}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 197 "ldgram.y"
    {
		  ldlex_popstate ();
		  mri_draw_tree ();
		  POP_ERROR ();
		}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 212 "ldgram.y"
    {
			einfo(_("%P%F: unrecognised keyword in MRI style script '%s'\n"),(yyvsp[(1) - (1)].name));
			}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 215 "ldgram.y"
    {
			config.map_filename = "-";
			}
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 221 "ldgram.y"
    { mri_public((yyvsp[(2) - (4)].name), (yyvsp[(4) - (4)].etree)); }
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 223 "ldgram.y"
    { mri_public((yyvsp[(2) - (4)].name), (yyvsp[(4) - (4)].etree)); }
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 225 "ldgram.y"
    { mri_public((yyvsp[(2) - (3)].name), (yyvsp[(3) - (3)].etree)); }
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 227 "ldgram.y"
    { mri_format((yyvsp[(2) - (2)].name)); }
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 229 "ldgram.y"
    { mri_output_section((yyvsp[(2) - (4)].name), (yyvsp[(4) - (4)].etree));}
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 231 "ldgram.y"
    { mri_output_section((yyvsp[(2) - (3)].name), (yyvsp[(3) - (3)].etree));}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 233 "ldgram.y"
    { mri_output_section((yyvsp[(2) - (4)].name), (yyvsp[(4) - (4)].etree));}
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 235 "ldgram.y"
    { mri_align((yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].etree)); }
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 237 "ldgram.y"
    { mri_align((yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].etree)); }
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 239 "ldgram.y"
    { mri_alignmod((yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].etree)); }
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 241 "ldgram.y"
    { mri_alignmod((yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].etree)); }
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 245 "ldgram.y"
    { mri_name((yyvsp[(2) - (2)].name)); }
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 247 "ldgram.y"
    { mri_alias((yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].name),0);}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 249 "ldgram.y"
    { mri_alias ((yyvsp[(2) - (4)].name), 0, (int) (yyvsp[(4) - (4)].bigint).integer); }
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 251 "ldgram.y"
    { mri_base((yyvsp[(2) - (2)].etree)); }
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 253 "ldgram.y"
    { mri_truncate ((unsigned int) (yyvsp[(2) - (2)].bigint).integer); }
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 257 "ldgram.y"
    { ldlex_script (); ldfile_open_command_file((yyvsp[(2) - (2)].name)); }
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 259 "ldgram.y"
    { ldlex_popstate (); }
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 261 "ldgram.y"
    { lang_add_entry ((yyvsp[(2) - (2)].name), FALSE); }
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 266 "ldgram.y"
    { mri_order((yyvsp[(3) - (3)].name)); }
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 267 "ldgram.y"
    { mri_order((yyvsp[(2) - (2)].name)); }
    break;

  case 47:

/* Line 1464 of yacc.c  */
#line 273 "ldgram.y"
    { mri_load((yyvsp[(1) - (1)].name)); }
    break;

  case 48:

/* Line 1464 of yacc.c  */
#line 274 "ldgram.y"
    { mri_load((yyvsp[(3) - (3)].name)); }
    break;

  case 49:

/* Line 1464 of yacc.c  */
#line 279 "ldgram.y"
    { mri_only_load((yyvsp[(1) - (1)].name)); }
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 281 "ldgram.y"
    { mri_only_load((yyvsp[(3) - (3)].name)); }
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 285 "ldgram.y"
    { (yyval.name) = NULL; }
    break;

  case 54:

/* Line 1464 of yacc.c  */
#line 292 "ldgram.y"
    { ldlex_expression (); }
    break;

  case 55:

/* Line 1464 of yacc.c  */
#line 294 "ldgram.y"
    { ldlex_popstate (); }
    break;

  case 56:

/* Line 1464 of yacc.c  */
#line 298 "ldgram.y"
    { ldlang_add_undef ((yyvsp[(1) - (1)].name), FALSE); }
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 300 "ldgram.y"
    { ldlang_add_undef ((yyvsp[(2) - (2)].name), FALSE); }
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 302 "ldgram.y"
    { ldlang_add_undef ((yyvsp[(3) - (3)].name), FALSE); }
    break;

  case 59:

/* Line 1464 of yacc.c  */
#line 306 "ldgram.y"
    { ldlex_both(); }
    break;

  case 60:

/* Line 1464 of yacc.c  */
#line 308 "ldgram.y"
    { ldlex_popstate(); }
    break;

  case 73:

/* Line 1464 of yacc.c  */
#line 329 "ldgram.y"
    { lang_add_target((yyvsp[(3) - (4)].name)); }
    break;

  case 74:

/* Line 1464 of yacc.c  */
#line 331 "ldgram.y"
    { ldfile_add_library_path ((yyvsp[(3) - (4)].name), FALSE); }
    break;

  case 75:

/* Line 1464 of yacc.c  */
#line 333 "ldgram.y"
    { lang_add_output((yyvsp[(3) - (4)].name), 1); }
    break;

  case 76:

/* Line 1464 of yacc.c  */
#line 335 "ldgram.y"
    { lang_add_output_format ((yyvsp[(3) - (4)].name), (char *) NULL,
					    (char *) NULL, 1); }
    break;

  case 77:

/* Line 1464 of yacc.c  */
#line 338 "ldgram.y"
    { lang_add_output_format ((yyvsp[(3) - (8)].name), (yyvsp[(5) - (8)].name), (yyvsp[(7) - (8)].name), 1); }
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 340 "ldgram.y"
    { ldfile_set_output_arch ((yyvsp[(3) - (4)].name), bfd_arch_unknown); }
    break;

  case 79:

/* Line 1464 of yacc.c  */
#line 342 "ldgram.y"
    { command_line.force_common_definition = TRUE ; }
    break;

  case 80:

/* Line 1464 of yacc.c  */
#line 344 "ldgram.y"
    { command_line.inhibit_common_definition = TRUE ; }
    break;

  case 82:

/* Line 1464 of yacc.c  */
#line 347 "ldgram.y"
    { lang_enter_group (); }
    break;

  case 83:

/* Line 1464 of yacc.c  */
#line 349 "ldgram.y"
    { lang_leave_group (); }
    break;

  case 84:

/* Line 1464 of yacc.c  */
#line 351 "ldgram.y"
    { lang_add_map((yyvsp[(3) - (4)].name)); }
    break;

  case 85:

/* Line 1464 of yacc.c  */
#line 353 "ldgram.y"
    { ldlex_script (); ldfile_open_command_file((yyvsp[(2) - (2)].name)); }
    break;

  case 86:

/* Line 1464 of yacc.c  */
#line 355 "ldgram.y"
    { ldlex_popstate (); }
    break;

  case 87:

/* Line 1464 of yacc.c  */
#line 357 "ldgram.y"
    {
		  lang_add_nocrossref ((yyvsp[(3) - (4)].nocrossref));
		}
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 362 "ldgram.y"
    { lang_add_insert ((yyvsp[(3) - (3)].name), 0); }
    break;

  case 90:

/* Line 1464 of yacc.c  */
#line 364 "ldgram.y"
    { lang_add_insert ((yyvsp[(3) - (3)].name), 1); }
    break;

  case 91:

/* Line 1464 of yacc.c  */
#line 366 "ldgram.y"
    { lang_memory_region_alias ((yyvsp[(3) - (6)].name), (yyvsp[(5) - (6)].name)); }
    break;

  case 92:

/* Line 1464 of yacc.c  */
#line 368 "ldgram.y"
    { lang_ld_feature ((yyvsp[(3) - (4)].name)); }
    break;

  case 93:

/* Line 1464 of yacc.c  */
#line 373 "ldgram.y"
    { lang_add_input_file((yyvsp[(1) - (1)].name),lang_input_file_is_search_file_enum,
				 (char *)NULL); }
    break;

  case 94:

/* Line 1464 of yacc.c  */
#line 376 "ldgram.y"
    { lang_add_input_file((yyvsp[(3) - (3)].name),lang_input_file_is_search_file_enum,
				 (char *)NULL); }
    break;

  case 95:

/* Line 1464 of yacc.c  */
#line 379 "ldgram.y"
    { lang_add_input_file((yyvsp[(2) - (2)].name),lang_input_file_is_search_file_enum,
				 (char *)NULL); }
    break;

  case 96:

/* Line 1464 of yacc.c  */
#line 382 "ldgram.y"
    { lang_add_input_file((yyvsp[(1) - (1)].name),lang_input_file_is_l_enum,
				 (char *)NULL); }
    break;

  case 97:

/* Line 1464 of yacc.c  */
#line 385 "ldgram.y"
    { lang_add_input_file((yyvsp[(3) - (3)].name),lang_input_file_is_l_enum,
				 (char *)NULL); }
    break;

  case 98:

/* Line 1464 of yacc.c  */
#line 388 "ldgram.y"
    { lang_add_input_file((yyvsp[(2) - (2)].name),lang_input_file_is_l_enum,
				 (char *)NULL); }
    break;

  case 99:

/* Line 1464 of yacc.c  */
#line 391 "ldgram.y"
    { (yyval.integer) = input_flags.add_DT_NEEDED_for_regular;
		    input_flags.add_DT_NEEDED_for_regular = TRUE; }
    break;

  case 100:

/* Line 1464 of yacc.c  */
#line 394 "ldgram.y"
    { input_flags.add_DT_NEEDED_for_regular = (yyvsp[(3) - (5)].integer); }
    break;

  case 101:

/* Line 1464 of yacc.c  */
#line 396 "ldgram.y"
    { (yyval.integer) = input_flags.add_DT_NEEDED_for_regular;
		    input_flags.add_DT_NEEDED_for_regular = TRUE; }
    break;

  case 102:

/* Line 1464 of yacc.c  */
#line 399 "ldgram.y"
    { input_flags.add_DT_NEEDED_for_regular = (yyvsp[(5) - (7)].integer); }
    break;

  case 103:

/* Line 1464 of yacc.c  */
#line 401 "ldgram.y"
    { (yyval.integer) = input_flags.add_DT_NEEDED_for_regular;
		    input_flags.add_DT_NEEDED_for_regular = TRUE; }
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 404 "ldgram.y"
    { input_flags.add_DT_NEEDED_for_regular = (yyvsp[(4) - (6)].integer); }
    break;

  case 109:

/* Line 1464 of yacc.c  */
#line 419 "ldgram.y"
    { lang_add_entry ((yyvsp[(3) - (4)].name), FALSE); }
    break;

  case 111:

/* Line 1464 of yacc.c  */
#line 421 "ldgram.y"
    {ldlex_expression ();}
    break;

  case 112:

/* Line 1464 of yacc.c  */
#line 422 "ldgram.y"
    { ldlex_popstate ();
		  lang_add_assignment (exp_assert ((yyvsp[(4) - (7)].etree), (yyvsp[(6) - (7)].name))); }
    break;

  case 113:

/* Line 1464 of yacc.c  */
#line 430 "ldgram.y"
    {
			  (yyval.cname) = (yyvsp[(1) - (1)].name);
			}
    break;

  case 114:

/* Line 1464 of yacc.c  */
#line 434 "ldgram.y"
    {
			  (yyval.cname) = "*";
			}
    break;

  case 115:

/* Line 1464 of yacc.c  */
#line 438 "ldgram.y"
    {
			  (yyval.cname) = "?";
			}
    break;

  case 116:

/* Line 1464 of yacc.c  */
#line 445 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(1) - (1)].cname);
			  (yyval.wildcard).sorted = none;
			  (yyval.wildcard).exclude_name_list = NULL;
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 117:

/* Line 1464 of yacc.c  */
#line 452 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(5) - (5)].cname);
			  (yyval.wildcard).sorted = none;
			  (yyval.wildcard).exclude_name_list = (yyvsp[(3) - (5)].name_list);
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 118:

/* Line 1464 of yacc.c  */
#line 459 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(3) - (4)].cname);
			  (yyval.wildcard).sorted = by_name;
			  (yyval.wildcard).exclude_name_list = NULL;
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 119:

/* Line 1464 of yacc.c  */
#line 466 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(3) - (4)].cname);
			  (yyval.wildcard).sorted = by_alignment;
			  (yyval.wildcard).exclude_name_list = NULL;
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 120:

/* Line 1464 of yacc.c  */
#line 473 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(3) - (4)].cname);
			  (yyval.wildcard).sorted = by_none;
			  (yyval.wildcard).exclude_name_list = NULL;
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 121:

/* Line 1464 of yacc.c  */
#line 480 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(5) - (7)].cname);
			  (yyval.wildcard).sorted = by_name_alignment;
			  (yyval.wildcard).exclude_name_list = NULL;
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 122:

/* Line 1464 of yacc.c  */
#line 487 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(5) - (7)].cname);
			  (yyval.wildcard).sorted = by_name;
			  (yyval.wildcard).exclude_name_list = NULL;
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 123:

/* Line 1464 of yacc.c  */
#line 494 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(5) - (7)].cname);
			  (yyval.wildcard).sorted = by_alignment_name;
			  (yyval.wildcard).exclude_name_list = NULL;
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 124:

/* Line 1464 of yacc.c  */
#line 501 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(5) - (7)].cname);
			  (yyval.wildcard).sorted = by_alignment;
			  (yyval.wildcard).exclude_name_list = NULL;
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 125:

/* Line 1464 of yacc.c  */
#line 508 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(7) - (8)].cname);
			  (yyval.wildcard).sorted = by_name;
			  (yyval.wildcard).exclude_name_list = (yyvsp[(5) - (8)].name_list);
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 126:

/* Line 1464 of yacc.c  */
#line 515 "ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(3) - (4)].cname);
			  (yyval.wildcard).sorted = by_init_priority;
			  (yyval.wildcard).exclude_name_list = NULL;
			  (yyval.wildcard).section_flag_list = NULL;
			}
    break;

  case 127:

/* Line 1464 of yacc.c  */
#line 524 "ldgram.y"
    {
			  struct flag_info_list *n;
			  n = ((struct flag_info_list *) xmalloc (sizeof *n));
			  if ((yyvsp[(1) - (1)].name)[0] == '!')
			    {
			      n->with = without_flags;
			      n->name = &(yyvsp[(1) - (1)].name)[1];
			    }
			  else
			    {
			      n->with = with_flags;
			      n->name = (yyvsp[(1) - (1)].name);
			    }
			  n->valid = FALSE;
			  n->next = NULL;
			  (yyval.flag_info_list) = n;
			}
    break;

  case 128:

/* Line 1464 of yacc.c  */
#line 542 "ldgram.y"
    {
			  struct flag_info_list *n;
			  n = ((struct flag_info_list *) xmalloc (sizeof *n));
			  if ((yyvsp[(3) - (3)].name)[0] == '!')
			    {
			      n->with = without_flags;
			      n->name = &(yyvsp[(3) - (3)].name)[1];
			    }
			  else
			    {
			      n->with = with_flags;
			      n->name = (yyvsp[(3) - (3)].name);
			    }
			  n->valid = FALSE;
			  n->next = (yyvsp[(1) - (3)].flag_info_list);
			  (yyval.flag_info_list) = n;
			}
    break;

  case 129:

/* Line 1464 of yacc.c  */
#line 563 "ldgram.y"
    {
			  struct flag_info *n;
			  n = ((struct flag_info *) xmalloc (sizeof *n));
			  n->flag_list = (yyvsp[(3) - (4)].flag_info_list);
			  n->flags_initialized = FALSE;
			  n->not_with_flags = 0;
			  n->only_with_flags = 0;
			  (yyval.flag_info) = n;
			}
    break;

  case 130:

/* Line 1464 of yacc.c  */
#line 576 "ldgram.y"
    {
			  struct name_list *tmp;
			  tmp = (struct name_list *) xmalloc (sizeof *tmp);
			  tmp->name = (yyvsp[(2) - (2)].cname);
			  tmp->next = (yyvsp[(1) - (2)].name_list);
			  (yyval.name_list) = tmp;
			}
    break;

  case 131:

/* Line 1464 of yacc.c  */
#line 585 "ldgram.y"
    {
			  struct name_list *tmp;
			  tmp = (struct name_list *) xmalloc (sizeof *tmp);
			  tmp->name = (yyvsp[(1) - (1)].cname);
			  tmp->next = NULL;
			  (yyval.name_list) = tmp;
			}
    break;

  case 132:

/* Line 1464 of yacc.c  */
#line 596 "ldgram.y"
    {
			  struct wildcard_list *tmp;
			  tmp = (struct wildcard_list *) xmalloc (sizeof *tmp);
			  tmp->next = (yyvsp[(1) - (3)].wildcard_list);
			  tmp->spec = (yyvsp[(3) - (3)].wildcard);
			  (yyval.wildcard_list) = tmp;
			}
    break;

  case 133:

/* Line 1464 of yacc.c  */
#line 605 "ldgram.y"
    {
			  struct wildcard_list *tmp;
			  tmp = (struct wildcard_list *) xmalloc (sizeof *tmp);
			  tmp->next = NULL;
			  tmp->spec = (yyvsp[(1) - (1)].wildcard);
			  (yyval.wildcard_list) = tmp;
			}
    break;

  case 134:

/* Line 1464 of yacc.c  */
#line 616 "ldgram.y"
    {
			  struct wildcard_spec tmp;
			  tmp.name = (yyvsp[(1) - (1)].name);
			  tmp.exclude_name_list = NULL;
			  tmp.sorted = none;
			  tmp.section_flag_list = NULL;
			  lang_add_wild (&tmp, NULL, ldgram_had_keep);
			}
    break;

  case 135:

/* Line 1464 of yacc.c  */
#line 625 "ldgram.y"
    {
			  struct wildcard_spec tmp;
			  tmp.name = (yyvsp[(2) - (2)].name);
			  tmp.exclude_name_list = NULL;
			  tmp.sorted = none;
			  tmp.section_flag_list = (yyvsp[(1) - (2)].flag_info);
			  lang_add_wild (&tmp, NULL, ldgram_had_keep);
			}
    break;

  case 136:

/* Line 1464 of yacc.c  */
#line 634 "ldgram.y"
    {
			  lang_add_wild (NULL, (yyvsp[(2) - (3)].wildcard_list), ldgram_had_keep);
			}
    break;

  case 137:

/* Line 1464 of yacc.c  */
#line 638 "ldgram.y"
    {
			  struct wildcard_spec tmp;
			  tmp.name = NULL;
			  tmp.exclude_name_list = NULL;
			  tmp.sorted = none;
			  tmp.section_flag_list = (yyvsp[(1) - (4)].flag_info);
			  lang_add_wild (&tmp, (yyvsp[(3) - (4)].wildcard_list), ldgram_had_keep);
			}
    break;

  case 138:

/* Line 1464 of yacc.c  */
#line 647 "ldgram.y"
    {
			  lang_add_wild (&(yyvsp[(1) - (4)].wildcard), (yyvsp[(3) - (4)].wildcard_list), ldgram_had_keep);
			}
    break;

  case 139:

/* Line 1464 of yacc.c  */
#line 651 "ldgram.y"
    {
			  (yyvsp[(2) - (5)].wildcard).section_flag_list = (yyvsp[(1) - (5)].flag_info);
			  lang_add_wild (&(yyvsp[(2) - (5)].wildcard), (yyvsp[(4) - (5)].wildcard_list), ldgram_had_keep);
			}
    break;

  case 141:

/* Line 1464 of yacc.c  */
#line 660 "ldgram.y"
    { ldgram_had_keep = TRUE; }
    break;

  case 142:

/* Line 1464 of yacc.c  */
#line 662 "ldgram.y"
    { ldgram_had_keep = FALSE; }
    break;

  case 144:

/* Line 1464 of yacc.c  */
#line 668 "ldgram.y"
    {
 		lang_add_attribute(lang_object_symbols_statement_enum);
	      	}
    break;

  case 146:

/* Line 1464 of yacc.c  */
#line 673 "ldgram.y"
    {

		  lang_add_attribute(lang_constructors_statement_enum);
		}
    break;

  case 147:

/* Line 1464 of yacc.c  */
#line 678 "ldgram.y"
    {
		  constructors_sorted = TRUE;
		  lang_add_attribute (lang_constructors_statement_enum);
		}
    break;

  case 149:

/* Line 1464 of yacc.c  */
#line 684 "ldgram.y"
    {
			  lang_add_data ((int) (yyvsp[(1) - (4)].integer), (yyvsp[(3) - (4)].etree));
			}
    break;

  case 150:

/* Line 1464 of yacc.c  */
#line 689 "ldgram.y"
    {
			  lang_add_fill ((yyvsp[(3) - (4)].fill));
			}
    break;

  case 151:

/* Line 1464 of yacc.c  */
#line 692 "ldgram.y"
    {ldlex_expression ();}
    break;

  case 152:

/* Line 1464 of yacc.c  */
#line 693 "ldgram.y"
    { ldlex_popstate ();
			  lang_add_assignment (exp_assert ((yyvsp[(4) - (8)].etree), (yyvsp[(6) - (8)].name))); }
    break;

  case 153:

/* Line 1464 of yacc.c  */
#line 696 "ldgram.y"
    { ldlex_script (); ldfile_open_command_file((yyvsp[(2) - (2)].name)); }
    break;

  case 154:

/* Line 1464 of yacc.c  */
#line 698 "ldgram.y"
    { ldlex_popstate (); }
    break;

  case 159:

/* Line 1464 of yacc.c  */
#line 713 "ldgram.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].token); }
    break;

  case 160:

/* Line 1464 of yacc.c  */
#line 715 "ldgram.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].token); }
    break;

  case 161:

/* Line 1464 of yacc.c  */
#line 717 "ldgram.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].token); }
    break;

  case 162:

/* Line 1464 of yacc.c  */
#line 719 "ldgram.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].token); }
    break;

  case 163:

/* Line 1464 of yacc.c  */
#line 721 "ldgram.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].token); }
    break;

  case 164:

/* Line 1464 of yacc.c  */
#line 726 "ldgram.y"
    {
		  (yyval.fill) = exp_get_fill ((yyvsp[(1) - (1)].etree), 0, "fill value");
		}
    break;

  case 165:

/* Line 1464 of yacc.c  */
#line 733 "ldgram.y"
    { (yyval.fill) = (yyvsp[(2) - (2)].fill); }
    break;

  case 166:

/* Line 1464 of yacc.c  */
#line 734 "ldgram.y"
    { (yyval.fill) = (fill_type *) 0; }
    break;

  case 167:

/* Line 1464 of yacc.c  */
#line 739 "ldgram.y"
    { (yyval.token) = '+'; }
    break;

  case 168:

/* Line 1464 of yacc.c  */
#line 741 "ldgram.y"
    { (yyval.token) = '-'; }
    break;

  case 169:

/* Line 1464 of yacc.c  */
#line 743 "ldgram.y"
    { (yyval.token) = '*'; }
    break;

  case 170:

/* Line 1464 of yacc.c  */
#line 745 "ldgram.y"
    { (yyval.token) = '/'; }
    break;

  case 171:

/* Line 1464 of yacc.c  */
#line 747 "ldgram.y"
    { (yyval.token) = LSHIFT; }
    break;

  case 172:

/* Line 1464 of yacc.c  */
#line 749 "ldgram.y"
    { (yyval.token) = RSHIFT; }
    break;

  case 173:

/* Line 1464 of yacc.c  */
#line 751 "ldgram.y"
    { (yyval.token) = '&'; }
    break;

  case 174:

/* Line 1464 of yacc.c  */
#line 753 "ldgram.y"
    { (yyval.token) = '|'; }
    break;

  case 177:

/* Line 1464 of yacc.c  */
#line 763 "ldgram.y"
    {
		  lang_add_assignment (exp_assign ((yyvsp[(1) - (3)].name), (yyvsp[(3) - (3)].etree), FALSE));
		}
    break;

  case 178:

/* Line 1464 of yacc.c  */
#line 767 "ldgram.y"
    {
		  lang_add_assignment (exp_assign ((yyvsp[(1) - (3)].name),
						   exp_binop ((yyvsp[(2) - (3)].token),
							      exp_nameop (NAME,
									  (yyvsp[(1) - (3)].name)),
							      (yyvsp[(3) - (3)].etree)), FALSE));
		}
    break;

  case 179:

/* Line 1464 of yacc.c  */
#line 775 "ldgram.y"
    {
		  lang_add_assignment (exp_assign ((yyvsp[(3) - (6)].name), (yyvsp[(5) - (6)].etree), TRUE));
		}
    break;

  case 180:

/* Line 1464 of yacc.c  */
#line 779 "ldgram.y"
    {
		  lang_add_assignment (exp_provide ((yyvsp[(3) - (6)].name), (yyvsp[(5) - (6)].etree), FALSE));
		}
    break;

  case 181:

/* Line 1464 of yacc.c  */
#line 783 "ldgram.y"
    {
		  lang_add_assignment (exp_provide ((yyvsp[(3) - (6)].name), (yyvsp[(5) - (6)].etree), TRUE));
		}
    break;

  case 189:

/* Line 1464 of yacc.c  */
#line 806 "ldgram.y"
    { region = lang_memory_region_lookup ((yyvsp[(1) - (1)].name), TRUE); }
    break;

  case 190:

/* Line 1464 of yacc.c  */
#line 809 "ldgram.y"
    {}
    break;

  case 191:

/* Line 1464 of yacc.c  */
#line 811 "ldgram.y"
    { ldlex_script (); ldfile_open_command_file((yyvsp[(2) - (2)].name)); }
    break;

  case 192:

/* Line 1464 of yacc.c  */
#line 813 "ldgram.y"
    { ldlex_popstate (); }
    break;

  case 193:

/* Line 1464 of yacc.c  */
#line 818 "ldgram.y"
    {
		  region->origin = exp_get_vma ((yyvsp[(3) - (3)].etree), 0, "origin");
		  region->current = region->origin;
		}
    break;

  case 194:

/* Line 1464 of yacc.c  */
#line 826 "ldgram.y"
    {
		  region->length = exp_get_vma ((yyvsp[(3) - (3)].etree), -1, "length");
		}
    break;

  case 195:

/* Line 1464 of yacc.c  */
#line 833 "ldgram.y"
    { /* dummy action to avoid bison 1.25 error message */ }
    break;

  case 199:

/* Line 1464 of yacc.c  */
#line 844 "ldgram.y"
    { lang_set_flags (region, (yyvsp[(1) - (1)].name), 0); }
    break;

  case 200:

/* Line 1464 of yacc.c  */
#line 846 "ldgram.y"
    { lang_set_flags (region, (yyvsp[(2) - (2)].name), 1); }
    break;

  case 201:

/* Line 1464 of yacc.c  */
#line 851 "ldgram.y"
    { lang_startup((yyvsp[(3) - (4)].name)); }
    break;

  case 203:

/* Line 1464 of yacc.c  */
#line 857 "ldgram.y"
    { ldemul_hll((char *)NULL); }
    break;

  case 204:

/* Line 1464 of yacc.c  */
#line 862 "ldgram.y"
    { ldemul_hll((yyvsp[(3) - (3)].name)); }
    break;

  case 205:

/* Line 1464 of yacc.c  */
#line 864 "ldgram.y"
    { ldemul_hll((yyvsp[(1) - (1)].name)); }
    break;

  case 207:

/* Line 1464 of yacc.c  */
#line 872 "ldgram.y"
    { ldemul_syslib((yyvsp[(3) - (3)].name)); }
    break;

  case 209:

/* Line 1464 of yacc.c  */
#line 878 "ldgram.y"
    { lang_float(TRUE); }
    break;

  case 210:

/* Line 1464 of yacc.c  */
#line 880 "ldgram.y"
    { lang_float(FALSE); }
    break;

  case 211:

/* Line 1464 of yacc.c  */
#line 885 "ldgram.y"
    {
		  (yyval.nocrossref) = NULL;
		}
    break;

  case 212:

/* Line 1464 of yacc.c  */
#line 889 "ldgram.y"
    {
		  struct lang_nocrossref *n;

		  n = (struct lang_nocrossref *) xmalloc (sizeof *n);
		  n->name = (yyvsp[(1) - (2)].name);
		  n->next = (yyvsp[(2) - (2)].nocrossref);
		  (yyval.nocrossref) = n;
		}
    break;

  case 213:

/* Line 1464 of yacc.c  */
#line 898 "ldgram.y"
    {
		  struct lang_nocrossref *n;

		  n = (struct lang_nocrossref *) xmalloc (sizeof *n);
		  n->name = (yyvsp[(1) - (3)].name);
		  n->next = (yyvsp[(3) - (3)].nocrossref);
		  (yyval.nocrossref) = n;
		}
    break;

  case 214:

/* Line 1464 of yacc.c  */
#line 908 "ldgram.y"
    { ldlex_expression (); }
    break;

  case 215:

/* Line 1464 of yacc.c  */
#line 910 "ldgram.y"
    { ldlex_popstate (); (yyval.etree)=(yyvsp[(2) - (2)].etree);}
    break;

  case 216:

/* Line 1464 of yacc.c  */
#line 915 "ldgram.y"
    { (yyval.etree) = exp_unop ('-', (yyvsp[(2) - (2)].etree)); }
    break;

  case 217:

/* Line 1464 of yacc.c  */
#line 917 "ldgram.y"
    { (yyval.etree) = (yyvsp[(2) - (3)].etree); }
    break;

  case 218:

/* Line 1464 of yacc.c  */
#line 919 "ldgram.y"
    { (yyval.etree) = exp_unop ((int) (yyvsp[(1) - (4)].integer),(yyvsp[(3) - (4)].etree)); }
    break;

  case 219:

/* Line 1464 of yacc.c  */
#line 921 "ldgram.y"
    { (yyval.etree) = exp_unop ('!', (yyvsp[(2) - (2)].etree)); }
    break;

  case 220:

/* Line 1464 of yacc.c  */
#line 923 "ldgram.y"
    { (yyval.etree) = (yyvsp[(2) - (2)].etree); }
    break;

  case 221:

/* Line 1464 of yacc.c  */
#line 925 "ldgram.y"
    { (yyval.etree) = exp_unop ('~', (yyvsp[(2) - (2)].etree));}
    break;

  case 222:

/* Line 1464 of yacc.c  */
#line 928 "ldgram.y"
    { (yyval.etree) = exp_binop ('*', (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 223:

/* Line 1464 of yacc.c  */
#line 930 "ldgram.y"
    { (yyval.etree) = exp_binop ('/', (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 224:

/* Line 1464 of yacc.c  */
#line 932 "ldgram.y"
    { (yyval.etree) = exp_binop ('%', (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 225:

/* Line 1464 of yacc.c  */
#line 934 "ldgram.y"
    { (yyval.etree) = exp_binop ('+', (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 226:

/* Line 1464 of yacc.c  */
#line 936 "ldgram.y"
    { (yyval.etree) = exp_binop ('-' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 227:

/* Line 1464 of yacc.c  */
#line 938 "ldgram.y"
    { (yyval.etree) = exp_binop (LSHIFT , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 228:

/* Line 1464 of yacc.c  */
#line 940 "ldgram.y"
    { (yyval.etree) = exp_binop (RSHIFT , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 229:

/* Line 1464 of yacc.c  */
#line 942 "ldgram.y"
    { (yyval.etree) = exp_binop (EQ , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 230:

/* Line 1464 of yacc.c  */
#line 944 "ldgram.y"
    { (yyval.etree) = exp_binop (NE , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 231:

/* Line 1464 of yacc.c  */
#line 946 "ldgram.y"
    { (yyval.etree) = exp_binop (LE , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 232:

/* Line 1464 of yacc.c  */
#line 948 "ldgram.y"
    { (yyval.etree) = exp_binop (GE , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 233:

/* Line 1464 of yacc.c  */
#line 950 "ldgram.y"
    { (yyval.etree) = exp_binop ('<' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 234:

/* Line 1464 of yacc.c  */
#line 952 "ldgram.y"
    { (yyval.etree) = exp_binop ('>' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 235:

/* Line 1464 of yacc.c  */
#line 954 "ldgram.y"
    { (yyval.etree) = exp_binop ('&' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 236:

/* Line 1464 of yacc.c  */
#line 956 "ldgram.y"
    { (yyval.etree) = exp_binop ('^' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 237:

/* Line 1464 of yacc.c  */
#line 958 "ldgram.y"
    { (yyval.etree) = exp_binop ('|' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 238:

/* Line 1464 of yacc.c  */
#line 960 "ldgram.y"
    { (yyval.etree) = exp_trinop ('?' , (yyvsp[(1) - (5)].etree), (yyvsp[(3) - (5)].etree), (yyvsp[(5) - (5)].etree)); }
    break;

  case 239:

/* Line 1464 of yacc.c  */
#line 962 "ldgram.y"
    { (yyval.etree) = exp_binop (ANDAND , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 240:

/* Line 1464 of yacc.c  */
#line 964 "ldgram.y"
    { (yyval.etree) = exp_binop (OROR , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 241:

/* Line 1464 of yacc.c  */
#line 966 "ldgram.y"
    { (yyval.etree) = exp_nameop (DEFINED, (yyvsp[(3) - (4)].name)); }
    break;

  case 242:

/* Line 1464 of yacc.c  */
#line 968 "ldgram.y"
    { (yyval.etree) = exp_bigintop ((yyvsp[(1) - (1)].bigint).integer, (yyvsp[(1) - (1)].bigint).str); }
    break;

  case 243:

/* Line 1464 of yacc.c  */
#line 970 "ldgram.y"
    { (yyval.etree) = exp_nameop (SIZEOF_HEADERS,0); }
    break;

  case 244:

/* Line 1464 of yacc.c  */
#line 973 "ldgram.y"
    { (yyval.etree) = exp_nameop (ALIGNOF,(yyvsp[(3) - (4)].name)); }
    break;

  case 245:

/* Line 1464 of yacc.c  */
#line 975 "ldgram.y"
    { (yyval.etree) = exp_nameop (SIZEOF,(yyvsp[(3) - (4)].name)); }
    break;

  case 246:

/* Line 1464 of yacc.c  */
#line 977 "ldgram.y"
    { (yyval.etree) = exp_nameop (ADDR,(yyvsp[(3) - (4)].name)); }
    break;

  case 247:

/* Line 1464 of yacc.c  */
#line 979 "ldgram.y"
    { (yyval.etree) = exp_nameop (LOADADDR,(yyvsp[(3) - (4)].name)); }
    break;

  case 248:

/* Line 1464 of yacc.c  */
#line 981 "ldgram.y"
    { (yyval.etree) = exp_nameop (CONSTANT,(yyvsp[(3) - (4)].name)); }
    break;

  case 249:

/* Line 1464 of yacc.c  */
#line 983 "ldgram.y"
    { (yyval.etree) = exp_unop (ABSOLUTE, (yyvsp[(3) - (4)].etree)); }
    break;

  case 250:

/* Line 1464 of yacc.c  */
#line 985 "ldgram.y"
    { (yyval.etree) = exp_unop (ALIGN_K,(yyvsp[(3) - (4)].etree)); }
    break;

  case 251:

/* Line 1464 of yacc.c  */
#line 987 "ldgram.y"
    { (yyval.etree) = exp_binop (ALIGN_K,(yyvsp[(3) - (6)].etree),(yyvsp[(5) - (6)].etree)); }
    break;

  case 252:

/* Line 1464 of yacc.c  */
#line 989 "ldgram.y"
    { (yyval.etree) = exp_binop (DATA_SEGMENT_ALIGN, (yyvsp[(3) - (6)].etree), (yyvsp[(5) - (6)].etree)); }
    break;

  case 253:

/* Line 1464 of yacc.c  */
#line 991 "ldgram.y"
    { (yyval.etree) = exp_binop (DATA_SEGMENT_RELRO_END, (yyvsp[(5) - (6)].etree), (yyvsp[(3) - (6)].etree)); }
    break;

  case 254:

/* Line 1464 of yacc.c  */
#line 993 "ldgram.y"
    { (yyval.etree) = exp_unop (DATA_SEGMENT_END, (yyvsp[(3) - (4)].etree)); }
    break;

  case 255:

/* Line 1464 of yacc.c  */
#line 995 "ldgram.y"
    { /* The operands to the expression node are
			     placed in the opposite order from the way
			     in which they appear in the script as
			     that allows us to reuse more code in
			     fold_binary.  */
			  (yyval.etree) = exp_binop (SEGMENT_START,
					  (yyvsp[(5) - (6)].etree),
					  exp_nameop (NAME, (yyvsp[(3) - (6)].name))); }
    break;

  case 256:

/* Line 1464 of yacc.c  */
#line 1004 "ldgram.y"
    { (yyval.etree) = exp_unop (ALIGN_K,(yyvsp[(3) - (4)].etree)); }
    break;

  case 257:

/* Line 1464 of yacc.c  */
#line 1006 "ldgram.y"
    { (yyval.etree) = exp_nameop (NAME,(yyvsp[(1) - (1)].name)); }
    break;

  case 258:

/* Line 1464 of yacc.c  */
#line 1008 "ldgram.y"
    { (yyval.etree) = exp_binop (MAX_K, (yyvsp[(3) - (6)].etree), (yyvsp[(5) - (6)].etree) ); }
    break;

  case 259:

/* Line 1464 of yacc.c  */
#line 1010 "ldgram.y"
    { (yyval.etree) = exp_binop (MIN_K, (yyvsp[(3) - (6)].etree), (yyvsp[(5) - (6)].etree) ); }
    break;

  case 260:

/* Line 1464 of yacc.c  */
#line 1012 "ldgram.y"
    { (yyval.etree) = exp_assert ((yyvsp[(3) - (6)].etree), (yyvsp[(5) - (6)].name)); }
    break;

  case 261:

/* Line 1464 of yacc.c  */
#line 1014 "ldgram.y"
    { (yyval.etree) = exp_nameop (ORIGIN, (yyvsp[(3) - (4)].name)); }
    break;

  case 262:

/* Line 1464 of yacc.c  */
#line 1016 "ldgram.y"
    { (yyval.etree) = exp_nameop (LENGTH, (yyvsp[(3) - (4)].name)); }
    break;

  case 263:

/* Line 1464 of yacc.c  */
#line 1018 "ldgram.y"
    { (yyval.etree) = exp_unop (LOG2CEIL, (yyvsp[(3) - (4)].etree)); }
    break;

  case 264:

/* Line 1464 of yacc.c  */
#line 1023 "ldgram.y"
    { (yyval.name) = (yyvsp[(3) - (3)].name); }
    break;

  case 265:

/* Line 1464 of yacc.c  */
#line 1024 "ldgram.y"
    { (yyval.name) = 0; }
    break;

  case 266:

/* Line 1464 of yacc.c  */
#line 1028 "ldgram.y"
    { (yyval.name) = (yyvsp[(3) - (3)].name); /* overflow method 1 */ }
    break;

  case 267:

/* Line 1464 of yacc.c  */
#line 1029 "ldgram.y"
    { (yyval.name) = 0; /* overflow method 2 */  }
    break;

  case 268:

/* Line 1464 of yacc.c  */
#line 1034 "ldgram.y"
    { (yyval.etree) = (yyvsp[(3) - (4)].etree); }
    break;

  case 269:

/* Line 1464 of yacc.c  */
#line 1035 "ldgram.y"
    { (yyval.etree) = 0; }
    break;

  case 270:

/* Line 1464 of yacc.c  */
#line 1039 "ldgram.y"
    { (yyval.etree) = (yyvsp[(3) - (4)].etree); }
    break;

  case 271:

/* Line 1464 of yacc.c  */
#line 1040 "ldgram.y"
    { (yyval.etree) = 0; }
    break;

  case 272:

/* Line 1464 of yacc.c  */
#line 1044 "ldgram.y"
    { (yyval.token) = ALIGN_WITH_INPUT; }
    break;

  case 273:

/* Line 1464 of yacc.c  */
#line 1045 "ldgram.y"
    { (yyval.token) = 0; }
    break;

  case 274:

/* Line 1464 of yacc.c  */
#line 1049 "ldgram.y"
    { (yyval.etree) = (yyvsp[(3) - (4)].etree); }
    break;

  case 275:

/* Line 1464 of yacc.c  */
#line 1050 "ldgram.y"
    { (yyval.etree) = 0; }
    break;

  case 276:

/* Line 1464 of yacc.c  */
#line 1054 "ldgram.y"
    { (yyval.token) = ONLY_IF_RO; }
    break;

  case 277:

/* Line 1464 of yacc.c  */
#line 1055 "ldgram.y"
    { (yyval.token) = ONLY_IF_RW; }
    break;

  case 278:

/* Line 1464 of yacc.c  */
#line 1056 "ldgram.y"
    { (yyval.token) = SPECIAL; }
    break;

  case 279:

/* Line 1464 of yacc.c  */
#line 1057 "ldgram.y"
    { (yyval.token) = 0; }
    break;

  case 280:

/* Line 1464 of yacc.c  */
#line 1060 "ldgram.y"
    { ldlex_expression(); }
    break;

  case 281:

/* Line 1464 of yacc.c  */
#line 1065 "ldgram.y"
    { ldlex_popstate (); ldlex_script (); }
    break;

  case 282:

/* Line 1464 of yacc.c  */
#line 1068 "ldgram.y"
    {
			  lang_enter_output_section_statement((yyvsp[(1) - (10)].name), (yyvsp[(3) - (10)].etree),
							      sectype,
							      (yyvsp[(5) - (10)].etree), (yyvsp[(7) - (10)].etree), (yyvsp[(4) - (10)].etree), (yyvsp[(9) - (10)].token), (yyvsp[(6) - (10)].token));
			}
    break;

  case 283:

/* Line 1464 of yacc.c  */
#line 1074 "ldgram.y"
    { ldlex_popstate (); ldlex_expression (); }
    break;

  case 284:

/* Line 1464 of yacc.c  */
#line 1076 "ldgram.y"
    {
		  ldlex_popstate ();
		  lang_leave_output_section_statement ((yyvsp[(18) - (19)].fill), (yyvsp[(15) - (19)].name), (yyvsp[(17) - (19)].section_phdr), (yyvsp[(16) - (19)].name), (yyvsp[(19) - (19)].name));
		}
    break;

  case 285:

/* Line 1464 of yacc.c  */
#line 1081 "ldgram.y"
    {}
    break;

  case 286:

/* Line 1464 of yacc.c  */
#line 1083 "ldgram.y"
    { ldlex_expression (); }
    break;

  case 287:

/* Line 1464 of yacc.c  */
#line 1085 "ldgram.y"
    { ldlex_popstate (); ldlex_script (); }
    break;

  case 288:

/* Line 1464 of yacc.c  */
#line 1087 "ldgram.y"
    {
			  lang_enter_overlay ((yyvsp[(3) - (8)].etree), (yyvsp[(6) - (8)].etree));
			}
    break;

  case 289:

/* Line 1464 of yacc.c  */
#line 1092 "ldgram.y"
    { ldlex_popstate (); ldlex_expression (); }
    break;

  case 290:

/* Line 1464 of yacc.c  */
#line 1094 "ldgram.y"
    {
			  ldlex_popstate ();
			  lang_leave_overlay ((yyvsp[(5) - (17)].etree), (int) (yyvsp[(4) - (17)].integer),
					      (yyvsp[(16) - (17)].fill), (yyvsp[(13) - (17)].name), (yyvsp[(15) - (17)].section_phdr), (yyvsp[(14) - (17)].name), (yyvsp[(17) - (17)].name));
			}
    break;

  case 292:

/* Line 1464 of yacc.c  */
#line 1104 "ldgram.y"
    { ldlex_expression (); }
    break;

  case 293:

/* Line 1464 of yacc.c  */
#line 1106 "ldgram.y"
    {
		  ldlex_popstate ();
		  lang_add_assignment (exp_assign (".", (yyvsp[(3) - (3)].etree), FALSE));
		}
    break;

  case 295:

/* Line 1464 of yacc.c  */
#line 1112 "ldgram.y"
    { ldlex_script (); ldfile_open_command_file((yyvsp[(2) - (2)].name)); }
    break;

  case 296:

/* Line 1464 of yacc.c  */
#line 1114 "ldgram.y"
    { ldlex_popstate (); }
    break;

  case 297:

/* Line 1464 of yacc.c  */
#line 1118 "ldgram.y"
    { sectype = noload_section; }
    break;

  case 298:

/* Line 1464 of yacc.c  */
#line 1119 "ldgram.y"
    { sectype = noalloc_section; }
    break;

  case 299:

/* Line 1464 of yacc.c  */
#line 1120 "ldgram.y"
    { sectype = noalloc_section; }
    break;

  case 300:

/* Line 1464 of yacc.c  */
#line 1121 "ldgram.y"
    { sectype = noalloc_section; }
    break;

  case 301:

/* Line 1464 of yacc.c  */
#line 1122 "ldgram.y"
    { sectype = noalloc_section; }
    break;

  case 303:

/* Line 1464 of yacc.c  */
#line 1127 "ldgram.y"
    { sectype = normal_section; }
    break;

  case 304:

/* Line 1464 of yacc.c  */
#line 1128 "ldgram.y"
    { sectype = normal_section; }
    break;

  case 305:

/* Line 1464 of yacc.c  */
#line 1132 "ldgram.y"
    { (yyval.etree) = (yyvsp[(1) - (3)].etree); }
    break;

  case 306:

/* Line 1464 of yacc.c  */
#line 1133 "ldgram.y"
    { (yyval.etree) = (etree_type *)NULL;  }
    break;

  case 307:

/* Line 1464 of yacc.c  */
#line 1138 "ldgram.y"
    { (yyval.etree) = (yyvsp[(3) - (6)].etree); }
    break;

  case 308:

/* Line 1464 of yacc.c  */
#line 1140 "ldgram.y"
    { (yyval.etree) = (yyvsp[(3) - (10)].etree); }
    break;

  case 309:

/* Line 1464 of yacc.c  */
#line 1144 "ldgram.y"
    { (yyval.etree) = (yyvsp[(1) - (2)].etree); }
    break;

  case 310:

/* Line 1464 of yacc.c  */
#line 1145 "ldgram.y"
    { (yyval.etree) = (etree_type *) NULL;  }
    break;

  case 311:

/* Line 1464 of yacc.c  */
#line 1150 "ldgram.y"
    { (yyval.integer) = 0; }
    break;

  case 312:

/* Line 1464 of yacc.c  */
#line 1152 "ldgram.y"
    { (yyval.integer) = 1; }
    break;

  case 313:

/* Line 1464 of yacc.c  */
#line 1157 "ldgram.y"
    { (yyval.name) = (yyvsp[(2) - (2)].name); }
    break;

  case 314:

/* Line 1464 of yacc.c  */
#line 1158 "ldgram.y"
    { (yyval.name) = DEFAULT_MEMORY_REGION; }
    break;

  case 315:

/* Line 1464 of yacc.c  */
#line 1163 "ldgram.y"
    {
		  (yyval.section_phdr) = NULL;
		}
    break;

  case 316:

/* Line 1464 of yacc.c  */
#line 1167 "ldgram.y"
    {
		  struct lang_output_section_phdr_list *n;

		  n = ((struct lang_output_section_phdr_list *)
		       xmalloc (sizeof *n));
		  n->name = (yyvsp[(3) - (3)].name);
		  n->used = FALSE;
		  n->next = (yyvsp[(1) - (3)].section_phdr);
		  (yyval.section_phdr) = n;
		}
    break;

  case 318:

/* Line 1464 of yacc.c  */
#line 1183 "ldgram.y"
    {
			  ldlex_script ();
			  lang_enter_overlay_section ((yyvsp[(2) - (2)].name));
			}
    break;

  case 319:

/* Line 1464 of yacc.c  */
#line 1188 "ldgram.y"
    { ldlex_popstate (); ldlex_expression (); }
    break;

  case 320:

/* Line 1464 of yacc.c  */
#line 1190 "ldgram.y"
    {
			  ldlex_popstate ();
			  lang_leave_overlay_section ((yyvsp[(9) - (9)].fill), (yyvsp[(8) - (9)].section_phdr));
			}
    break;

  case 325:

/* Line 1464 of yacc.c  */
#line 1207 "ldgram.y"
    { ldlex_expression (); }
    break;

  case 326:

/* Line 1464 of yacc.c  */
#line 1208 "ldgram.y"
    { ldlex_popstate (); }
    break;

  case 327:

/* Line 1464 of yacc.c  */
#line 1210 "ldgram.y"
    {
		  lang_new_phdr ((yyvsp[(1) - (6)].name), (yyvsp[(3) - (6)].etree), (yyvsp[(4) - (6)].phdr).filehdr, (yyvsp[(4) - (6)].phdr).phdrs, (yyvsp[(4) - (6)].phdr).at,
				 (yyvsp[(4) - (6)].phdr).flags);
		}
    break;

  case 328:

/* Line 1464 of yacc.c  */
#line 1218 "ldgram.y"
    {
		  (yyval.etree) = (yyvsp[(1) - (1)].etree);

		  if ((yyvsp[(1) - (1)].etree)->type.node_class == etree_name
		      && (yyvsp[(1) - (1)].etree)->type.node_code == NAME)
		    {
		      const char *s;
		      unsigned int i;
		      static const char * const phdr_types[] =
			{
			  "PT_NULL", "PT_LOAD", "PT_DYNAMIC",
			  "PT_INTERP", "PT_NOTE", "PT_SHLIB",
			  "PT_PHDR", "PT_TLS"
			};

		      s = (yyvsp[(1) - (1)].etree)->name.name;
		      for (i = 0;
			   i < sizeof phdr_types / sizeof phdr_types[0];
			   i++)
			if (strcmp (s, phdr_types[i]) == 0)
			  {
			    (yyval.etree) = exp_intop (i);
			    break;
			  }
		      if (i == sizeof phdr_types / sizeof phdr_types[0])
			{
			  if (strcmp (s, "PT_GNU_EH_FRAME") == 0)
			    (yyval.etree) = exp_intop (0x6474e550);
			  else if (strcmp (s, "PT_GNU_STACK") == 0)
			    (yyval.etree) = exp_intop (0x6474e551);
			  else
			    {
			      einfo (_("\
%X%P:%S: unknown phdr type `%s' (try integer literal)\n"),
				     NULL, s);
			      (yyval.etree) = exp_intop (0);
			    }
			}
		    }
		}
    break;

  case 329:

/* Line 1464 of yacc.c  */
#line 1262 "ldgram.y"
    {
		  memset (&(yyval.phdr), 0, sizeof (struct phdr_info));
		}
    break;

  case 330:

/* Line 1464 of yacc.c  */
#line 1266 "ldgram.y"
    {
		  (yyval.phdr) = (yyvsp[(3) - (3)].phdr);
		  if (strcmp ((yyvsp[(1) - (3)].name), "FILEHDR") == 0 && (yyvsp[(2) - (3)].etree) == NULL)
		    (yyval.phdr).filehdr = TRUE;
		  else if (strcmp ((yyvsp[(1) - (3)].name), "PHDRS") == 0 && (yyvsp[(2) - (3)].etree) == NULL)
		    (yyval.phdr).phdrs = TRUE;
		  else if (strcmp ((yyvsp[(1) - (3)].name), "FLAGS") == 0 && (yyvsp[(2) - (3)].etree) != NULL)
		    (yyval.phdr).flags = (yyvsp[(2) - (3)].etree);
		  else
		    einfo (_("%X%P:%S: PHDRS syntax error at `%s'\n"),
			   NULL, (yyvsp[(1) - (3)].name));
		}
    break;

  case 331:

/* Line 1464 of yacc.c  */
#line 1279 "ldgram.y"
    {
		  (yyval.phdr) = (yyvsp[(5) - (5)].phdr);
		  (yyval.phdr).at = (yyvsp[(3) - (5)].etree);
		}
    break;

  case 332:

/* Line 1464 of yacc.c  */
#line 1287 "ldgram.y"
    {
		  (yyval.etree) = NULL;
		}
    break;

  case 333:

/* Line 1464 of yacc.c  */
#line 1291 "ldgram.y"
    {
		  (yyval.etree) = (yyvsp[(2) - (3)].etree);
		}
    break;

  case 334:

/* Line 1464 of yacc.c  */
#line 1297 "ldgram.y"
    {
		  ldlex_version_file ();
		  PUSH_ERROR (_("dynamic list"));
		}
    break;

  case 335:

/* Line 1464 of yacc.c  */
#line 1302 "ldgram.y"
    {
		  ldlex_popstate ();
		  POP_ERROR ();
		}
    break;

  case 339:

/* Line 1464 of yacc.c  */
#line 1319 "ldgram.y"
    {
		  lang_append_dynamic_list ((yyvsp[(1) - (2)].versyms));
		}
    break;

  case 340:

/* Line 1464 of yacc.c  */
#line 1327 "ldgram.y"
    {
		  ldlex_version_file ();
		  PUSH_ERROR (_("VERSION script"));
		}
    break;

  case 341:

/* Line 1464 of yacc.c  */
#line 1332 "ldgram.y"
    {
		  ldlex_popstate ();
		  POP_ERROR ();
		}
    break;

  case 342:

/* Line 1464 of yacc.c  */
#line 1341 "ldgram.y"
    {
		  ldlex_version_script ();
		}
    break;

  case 343:

/* Line 1464 of yacc.c  */
#line 1345 "ldgram.y"
    {
		  ldlex_popstate ();
		}
    break;

  case 346:

/* Line 1464 of yacc.c  */
#line 1357 "ldgram.y"
    {
		  lang_register_vers_node (NULL, (yyvsp[(2) - (4)].versnode), NULL);
		}
    break;

  case 347:

/* Line 1464 of yacc.c  */
#line 1361 "ldgram.y"
    {
		  lang_register_vers_node ((yyvsp[(1) - (5)].name), (yyvsp[(3) - (5)].versnode), NULL);
		}
    break;

  case 348:

/* Line 1464 of yacc.c  */
#line 1365 "ldgram.y"
    {
		  lang_register_vers_node ((yyvsp[(1) - (6)].name), (yyvsp[(3) - (6)].versnode), (yyvsp[(5) - (6)].deflist));
		}
    break;

  case 349:

/* Line 1464 of yacc.c  */
#line 1372 "ldgram.y"
    {
		  (yyval.deflist) = lang_add_vers_depend (NULL, (yyvsp[(1) - (1)].name));
		}
    break;

  case 350:

/* Line 1464 of yacc.c  */
#line 1376 "ldgram.y"
    {
		  (yyval.deflist) = lang_add_vers_depend ((yyvsp[(1) - (2)].deflist), (yyvsp[(2) - (2)].name));
		}
    break;

  case 351:

/* Line 1464 of yacc.c  */
#line 1383 "ldgram.y"
    {
		  (yyval.versnode) = lang_new_vers_node (NULL, NULL);
		}
    break;

  case 352:

/* Line 1464 of yacc.c  */
#line 1387 "ldgram.y"
    {
		  (yyval.versnode) = lang_new_vers_node ((yyvsp[(1) - (2)].versyms), NULL);
		}
    break;

  case 353:

/* Line 1464 of yacc.c  */
#line 1391 "ldgram.y"
    {
		  (yyval.versnode) = lang_new_vers_node ((yyvsp[(3) - (4)].versyms), NULL);
		}
    break;

  case 354:

/* Line 1464 of yacc.c  */
#line 1395 "ldgram.y"
    {
		  (yyval.versnode) = lang_new_vers_node (NULL, (yyvsp[(3) - (4)].versyms));
		}
    break;

  case 355:

/* Line 1464 of yacc.c  */
#line 1399 "ldgram.y"
    {
		  (yyval.versnode) = lang_new_vers_node ((yyvsp[(3) - (8)].versyms), (yyvsp[(7) - (8)].versyms));
		}
    break;

  case 356:

/* Line 1464 of yacc.c  */
#line 1406 "ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern (NULL, (yyvsp[(1) - (1)].name), ldgram_vers_current_lang, FALSE);
		}
    break;

  case 357:

/* Line 1464 of yacc.c  */
#line 1410 "ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern (NULL, (yyvsp[(1) - (1)].name), ldgram_vers_current_lang, TRUE);
		}
    break;

  case 358:

/* Line 1464 of yacc.c  */
#line 1414 "ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern ((yyvsp[(1) - (3)].versyms), (yyvsp[(3) - (3)].name), ldgram_vers_current_lang, FALSE);
		}
    break;

  case 359:

/* Line 1464 of yacc.c  */
#line 1418 "ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern ((yyvsp[(1) - (3)].versyms), (yyvsp[(3) - (3)].name), ldgram_vers_current_lang, TRUE);
		}
    break;

  case 360:

/* Line 1464 of yacc.c  */
#line 1422 "ldgram.y"
    {
			  (yyval.name) = ldgram_vers_current_lang;
			  ldgram_vers_current_lang = (yyvsp[(4) - (5)].name);
			}
    break;

  case 361:

/* Line 1464 of yacc.c  */
#line 1427 "ldgram.y"
    {
			  struct bfd_elf_version_expr *pat;
			  for (pat = (yyvsp[(7) - (9)].versyms); pat->next != NULL; pat = pat->next);
			  pat->next = (yyvsp[(1) - (9)].versyms);
			  (yyval.versyms) = (yyvsp[(7) - (9)].versyms);
			  ldgram_vers_current_lang = (yyvsp[(6) - (9)].name);
			}
    break;

  case 362:

/* Line 1464 of yacc.c  */
#line 1435 "ldgram.y"
    {
			  (yyval.name) = ldgram_vers_current_lang;
			  ldgram_vers_current_lang = (yyvsp[(2) - (3)].name);
			}
    break;

  case 363:

/* Line 1464 of yacc.c  */
#line 1440 "ldgram.y"
    {
			  (yyval.versyms) = (yyvsp[(5) - (7)].versyms);
			  ldgram_vers_current_lang = (yyvsp[(4) - (7)].name);
			}
    break;

  case 364:

/* Line 1464 of yacc.c  */
#line 1445 "ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern (NULL, "global", ldgram_vers_current_lang, FALSE);
		}
    break;

  case 365:

/* Line 1464 of yacc.c  */
#line 1449 "ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern ((yyvsp[(1) - (3)].versyms), "global", ldgram_vers_current_lang, FALSE);
		}
    break;

  case 366:

/* Line 1464 of yacc.c  */
#line 1453 "ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern (NULL, "local", ldgram_vers_current_lang, FALSE);
		}
    break;

  case 367:

/* Line 1464 of yacc.c  */
#line 1457 "ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern ((yyvsp[(1) - (3)].versyms), "local", ldgram_vers_current_lang, FALSE);
		}
    break;

  case 368:

/* Line 1464 of yacc.c  */
#line 1461 "ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern (NULL, "extern", ldgram_vers_current_lang, FALSE);
		}
    break;

  case 369:

/* Line 1464 of yacc.c  */
#line 1465 "ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern ((yyvsp[(1) - (3)].versyms), "extern", ldgram_vers_current_lang, FALSE);
		}
    break;



/* Line 1464 of yacc.c  */
#line 5106 "ldgram.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1684 of yacc.c  */
#line 1475 "ldgram.y"

void
yyerror(arg)
     const char *arg;
{
  if (ldfile_assumed_script)
    einfo (_("%P:%s: file format not recognized; treating as linker script\n"),
	   ldlex_filename ());
  if (error_index > 0 && error_index < ERROR_NAME_MAX)
    einfo ("%P%F:%S: %s in %s\n", NULL, arg, error_names[error_index - 1]);
  else
    einfo ("%P%F:%S: %s\n", NULL, arg);
}

