#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define TMin INT_MIN
#define TMax INT_MAX

#include "btest.h"
#include "bits.h"

test_rec test_set[] = {
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 8.0.0.  Version 8.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2014, plus Amendment 1 (published
   2015-05-15).  */
/* We do not support C11 <threads.h>.  */
 {"fp_func4", (funct_t) fp_func4, (funct_t) test_fp_func4, 1,
    "$", 30, 4,
     {{1, 1},{1,1},{1,1}}},
 {"is_addition_no_problem", (funct_t) is_addition_no_problem, (funct_t) test_is_addition_no_problem, 2,
    "! ~ & ^ | + << >>", 20, 3,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"is_x_fits_in_16_bit", (funct_t) is_x_fits_in_16_bit, (funct_t) test_is_x_fits_in_16_bit, 1,
    "! ~ & ^ | + << >>", 8, 1,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"fp_func1", (funct_t) fp_func1, (funct_t) test_fp_func1, 1,
    "$", 30, 4,
     {{1, 1},{1,1},{1,1}}},
 {"fp_func3", (funct_t) fp_func3, (funct_t) test_fp_func3, 1,
    "$", 10, 2,
     {{1, 1},{1,1},{1,1}}},
 {"fp_func2", (funct_t) fp_func2, (funct_t) test_fp_func2, 1,
    "$", 30, 4,
     {{1, 1},{1,1},{1,1}}},
 {"extract_nth_byte", (funct_t) extract_nth_byte, (funct_t) test_extract_nth_byte, 2,
    "! ~ & ^ | + << >>", 6, 2,
  {{TMin, TMax},{0,3},{TMin,TMax}}},
 {"substitute_byte", (funct_t) substitute_byte, (funct_t) test_substitute_byte, 3,
    "! ~ & ^ | + << >>", 10, 3,
  {{TMin, TMax},{0,3},{0,255}}},
 {"floor_log_2", (funct_t) floor_log_2, (funct_t) test_floor_log_2, 1, "! ~ & ^ | + << >>", 90, 4,
  {{1, TMax},{TMin,TMax},{TMin,TMax}}},
 {"absolute_of_x", (funct_t) absolute_of_x, (funct_t) test_absolute_of_x, 1, "! ~ & ^ | + << >>", 10, 4,
  {{-TMax, TMax},{TMin,TMax},{TMin,TMax}}},
 {"is_le", (funct_t) is_le, (funct_t) test_is_le, 2,
    "! ~ & ^ | + << >>", 24, 3,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"divide_by_power_of_2", (funct_t) divide_by_power_of_2, (funct_t) test_divide_by_power_of_2, 2,
    "! ~ & ^ | + << >>", 15, 2,
  {{TMin, TMax},{0,30},{TMin,TMax}}},
 {"bang", (funct_t) bang, (funct_t) test_bang, 1,
    "~ & ^ | + << >>", 12, 4,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"is_subtraction_no_problem", (funct_t) is_subtraction_no_problem, (funct_t) test_is_subtraction_no_problem, 2,
    "! ~ & ^ | + << >>", 20, 3,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"aeb", (funct_t) aeb, (funct_t) test_aeb, 1,
    "! ~ & ^ | + << >>", 12, 2,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
  {"", NULL, NULL, 0, "", 0, 0,
   {{0, 0},{0,0},{0,0}}}
};
