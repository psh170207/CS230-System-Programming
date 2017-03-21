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
unsigned fp_func4(unsigned);
unsigned test_fp_func4(unsigned);
int is_addition_no_problem(int, int);
int test_is_addition_no_problem(int, int);
int is_x_fits_in_16_bit(int);
int test_is_x_fits_in_16_bit(int);
unsigned fp_func1(unsigned);
unsigned test_fp_func1(unsigned);
unsigned fp_func3(unsigned);
unsigned test_fp_func3(unsigned);
int fp_func2(unsigned);
int test_fp_func2(unsigned);
int extract_nth_byte(int, int);
int test_extract_nth_byte(int, int);
int substitute_byte(int, int, int);
int test_substitute_byte(int, int, int);
int floor_log_2(int);
int test_floor_log_2(int);
int absolute_of_x(int);
int test_absolute_of_x(int);
int is_le(int, int);
int test_is_le(int, int);
int divide_by_power_of_2(int, int);
int test_divide_by_power_of_2(int, int);
int bang(int);
int test_bang(int);
int is_subtraction_no_problem(int, int);
int test_is_subtraction_no_problem(int, int);
int aeb();
int test_aeb();
