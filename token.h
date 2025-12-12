/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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

#line 97 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
