/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
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
/* 
 * fp_func4 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Operations allowed to use: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max # of operations: 30
 *   Points: 4
 */
unsigned fp_func4(unsigned uf) {
	int s = !(uf>>31);// uf>0 -> 1 / uf<0 -> 0
	int exp = (uf&0x7fffffff)>>23;//exponetial of uf
	int init = 0x807FFFFF; // exp = 0 and other part is fill with 1's
				// intialize the exp
	if (exp==0xff) return uf; // inf or NaN
	if (exp==0){
		if(s==1) return uf<<1; // Denormalized-> just <<1
		else return (uf<<1)|(0x80000000);
	}
	else return (uf&init)|((exp+1)<<23); // Normalized -> exp=exp+1

}
/*
's' is sign bit(if uf>0 then s=1 and else then s=0). 'exp' is exp of uf. init is exp's part is fill with 0s and other part fill with 1s. init used for initialize the exp part to 0.
if exp==0xff, uf is inf or NaN so return uf. if exp is 0, uf is denormalized value so just uf<<1. if uf<0, add sign bit using 0x80000000. else, uf is normalized value so just exp=exp+1.
To substitute new exp to uf, use init.
*/

/* 
 * is_addition_no_problem - Will it be okay(no overflow) when we add x and y?
 *   If there is an overflow, return 0. Otherwise, return 1.
 *   Example: is_addition_no_problem(0x80000000,0x80000000) = 0,
 *            is_addition_no_problem(0x80000000,0x70000000) = 1, 
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 20
 *   Points: 3
 */
int is_addition_no_problem(int x, int y) {
	int eq=x^y;
	return (!!(eq>>31))+((!(eq>>31))&((x>>31)^(~(((x+y)>>31)))));


/*
eq evaluate that whether x and y are have same sign bit.
if x and y have different sign bit, there's no overflow.
However, if x and y have same sign bit, we can evaluate that there is overflow or not.
If x or y is positive, overflow occurs when x+y<0. Also, if x or y is negative, overflow occurs when
x+y>0.
Now, let me explain about code.
!!(eq>>31) is 1 when x and y are have different sign bit and 0 when x and y are have same sign bit.
!(eq>>31) is 0 when diff-sign bits and 1 when same-sign bits.
x>>31 is 0 when x is >0 and 0xffffffff when x is <0.
(x+y)>>31 is same with in case of x>>31 but evaluate x+y's sign.
Now, look 5 different cases.
Case 1 : x and y have different sign bits
	!!(eq>>31)=1, !(eq>>31)=0 so return 1.
Case 2 : x and y are positive and overflow not occurs.
	!!(eq>>31)=0, !(eq>>31)=1, (x>>31)=0, (x+y)>>31=0
	(x>>31)^(~((x+y)>>31))=0xffffffff, (!(eq>>31))&0xffffffff=0x01
	so return 1.
Case 3 : x and y are positive and overflow occurs.
	other values are same with case 2, but (x+y)>>31=0xffffffff
	so return 0.
Case 4 : x and y are negative and overflow not occurs.
	other values are same with case 2, but (x>>31)=0xffffffff, (x+y)>>31=0xffffffff
	so return 1.
Case 5 : x and y are negative and overflow ocurrs.
	other values are same with case 4, but (x+y)>>31=0
	so return 0.
*/

	
}
/* 
 * is_x_fits_in_16_bit - Returns 1 if x fits in 16-bit, 2's complement.
 *   Examples: is_x_fits_in_16_bit(34000) = 0, is_x_fits_in_16_bit(-32768) = 1
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 8
 *   Points: 1
 */
int is_x_fits_in_16_bit(int x) {
	int crit_x=x>>15;
	int std=~0;
  return !(crit_x&std)+(!(crit_x^std));
}


/*
crit_x means criteria of x to evaluates the 17 most siginificance bits.
std means standard for judge that whether the 17 most significance bits are used or not.
if x>0, possible range of number is 0x0~0x7fff and if x<0, possible range of number is 0xffff8000
~0xffffffff. So if x>0, crit_x&std is 0 when x fits in 16 bits and non-zero when x doesn't fits in 16 bits. Also if x<0, crit_x^std is 0 when x fits and non-zero when x doesn't fits.
And they doesn't effects another sign's evaluation expressions.
*/


/* 
 * fp_func1 - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Operations allowed to use: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max # of operations: 30
 *   Points: 4
 */
unsigned fp_func1(unsigned uf) {
  	int s = uf>>31;//uf>0 -> 0 / uf<0 -> 0xffffffff
	int exp = (uf&0x7fffffff)>>23;// exp of uf
	//int frac = uf&0x7fffff;//fraction of uf
	int odd = uf&0x01;//odd->1, even->0
	if (exp==0xff) return uf;
	else if(exp<=1){
		if (s==0) return (uf>>1)+(((uf>>1)&0x01)&odd);//round to even.
		else return (((uf&0x7fffffff)>>1)|0x80000000)+(odd&((((uf&0x7fffffff)>>1)|0x80000000)&0x01));//delete sign bit and >>1 and add sign bit.round to even.
	}
	else return ((uf&0x807fffff)|((exp+~0)<<23));//There's no garbage bits.
}

/*
To check sign and exp of uf, declare s and exp. And declare 'odd' to check uf is odd or even. If uf is even, there' no rounding. However, uf is odd, we need round-to-even.To round to even, if uf is odd and uf*0.5 is odd, we need +1. Otherwise, we don't need +1.If exp = 0 or 1,use arithmetic right shift because it is boundary of Denormailized and Normailized. Then, we change fraction of uf. So we need round to even. Otherwise, we don't need to round to even.If exp=0xff, uf is NaN or infinity, so return uf. If 1<exp<0xff, just exp=exp-1.
*/


/* 
 * fp_func3 - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Operations allowed to use: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max # of operations: 10
 *   Points: 2
 */
unsigned fp_func3(unsigned uf) {
	int num = 0x7fffffff;
	int exp = (uf&num)>>23;//exp of uf
	int frac = uf<<9;//fraction of uf
	if (exp==0xff && frac!=0) return uf;
	else return uf&num;
}
/*
Find absolute value of uf, if exp is not NaN, just set sign bit to 0.
if exp is 0xff and frac is not 0, uf is NaN so just return uf. else, return uf&0x7fffffff.
*/


/* 
 * fp_func2 - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Operations allowed to use: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max # of operations: 30
 *   Points: 4
 */
int fp_func2(unsigned uf) {
	int sb = uf&0x80000000;//sign bit
	int E = ((uf&0x7f800000)>>23)-127; //E=exp-bias
	int frac = uf&0x7fffff;
	int M = frac|0x800000; // add leading 1
	int G = 0;
	int R = 0;
	int S = 0;
	int ind=22-E;
	int ind1=23-E;
	int ans = M>>ind1;
	int ans1 = M<<(-ind1);
	int ret=0;

	if (E>=31){ // out of range
		if (sb!=0 && frac==0) ret=0x80000000;//-2^31
		else ret=0x80000000u;
	}
	else if (E<0) ret=0;//uf<1
	else if (E>=23) ret=ans1;
	else{ //0<=E<23/ M>>(23-E)
		G = (M &(1<<ind1));
		R = (M &(1<<ind));
		S = (M &(~((-1)<<ind)));
		if(R&&(G||S)) ret=ans+1;
		else ret=ans;
	}
	if (sb==0) return ret;
	else return -ret;
}

/*
sb is sign bit, E=exp-bias, frac is fraction of uf, M is fraction with leading 1.
if E>=31 : out of range (because of Tmax=2^31-1, Tmin=-2^31). But, if sb is not 0 and frac is 0, return -2^31.
if E<0 : 0<uf<1 with round-to-even, (int)uf is 0.
if E>=23, return just M<<(E-23).
if 0<=E<23, return M>>(23-E) but use GRS rounding.
*/

/* 
 * extract_nth_byte - Extract the nth byte from a word x and return it.
 *   The index of the byte at LSB is 0.
 *   The index of the byte at MSB is 3.
 *   Examples: extract_nth_byte(0xFF34AB78,1) = 0xAB
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 6
 *   Points: 2
 */
int extract_nth_byte(int x, int n) {
	int ind=n<<3;
  return ((x&(0xff<<ind))>>ind)&0xff;
}


/*
ind means index of bits. To extract nth byte, make extracting transposer that is 0xff<<ind.
x&(0xff<<ind) gets the nth byte on nth byte of transposer.
Now, to get nth byte on least significance bytes,use right shift with ind.
And to delete garbage bits use '&'operator with 0xff.
*/


/* 
 * substitute_byte(x,n,c) - In a word x, replace the nth byte with a new byte c.
 *   The index of the byte at LSB is 0.
 *   The index of the byte at MSB is 3.
 *   Examples: substitute_byte(0xffffffff,1,0xab) = 0xffffabff
 *   Assume that 0 <= n <= 3 and 0 <= c <= 255.
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 10
 *   Points: 3
 */
int substitute_byte(int x, int n, int c) {
	int ind=n<<3;
	int del=0xff<<ind;
  return (x&(~del))|(c<<ind);
}


/*
ind means index of nth byte's bits. To replace nth byte's value, first we delete nth byte's value
(x&(~del)).
And replace nth byte's value to c. use '|'operation with c<<ind.
*/


/*
 * floor_log_2 - return floor(log base 2 of x), where x > 0
 *   Example: floor_log_2(16) = 4
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 90
 *   Points: 4
 */
int floor_log_2(int x) {
  int n16 = ~0<<16;//0xffff0000
  int n8 = 0XFF<<8;//0x0000ff00
  int n4 = 0XF0;//0x000000f0
  int n2 = 0XC;//0x0000000c
  int n1 = 0x2;//0x00000002
  int exist_in_16 = !!(n16&x);//is 1 exists in first 16bits?
  int x8 = x>>(exist_in_16<<4);//yes -> x>>8 / no -> x
  int exist_in_8 = !!(n8&x8);//is 1 exists in first 8bits?
  int x4 = x8>>(exist_in_8<<3);//yes -> x>>4 / no -> x
  int exist_in_4 = !!(n4&x4);//is 1 exists in first 4bits?
  int x2 = x4>>(exist_in_4<<2);//yes -> x>>2 / no -> x
  int exist_in_2 = !!(n2&x2);//is 1 exists in first 2bits?
  int x1 = x2>>(exist_in_2<<1);//yes -> x>>1 / no -> x
  int exist_in_1 = !!(n1&x1);//is 1 exists in first 1bit?
  return (exist_in_16<<4)+(exist_in_8<<3)+(exist_in_4<<2)+(exist_in_2<<1)+exist_in_1;
}
/*
n16 is 0xffff0000, n8 is 0x0000ff00, n4 is 0x000000f0, n2 is 0x0000000c, n1 is 0x00000002.
exist_in_16 checks whether exists 1 in first 16bits of x. if there is 1 in first 16 bits then x8=x>>8, else then x8=x.
exist_in_8 checks whether exists 1 in first 8bits of x8. if ther is 1 in first 8 bits then x4=x8>>4, else then x4=x8.
repeat this steps until exist_in_1 then exist_in_16*16+exist_in_8*8+exist_in_4*4+exists_in_2*2+exists_in_1 is represent log2(x).
*/

/* 
 * absolute_of_x - Return abs(x)
 *   Example: absolute_of_x(-1) = 1.
 *   You may assume -TMax <= x <= TMax
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 10
 *   Points: 4
 */
int absolute_of_x(int x) {
	int sign_x = x>>31; // x>0 -> 0, x<0 -> 0xffffffff
	return ((~sign_x)&x)+(sign_x&(~x+1));
}
/*
sign_x is represent sign bit of x(if x>0 then 0, else then 0xffffffff).
if sign_x is 0 then return x, else then -x(=~x+1).
so, ((~sign_x)&x)+(sign_x&(~x+1)) is represent absolute of x.
*/

/* 
 * is_le - Return 1 when x <= y, else return 0 
 *   Example: is_le(4,5) = 1.
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 24
 *   Points: 3
 */
int is_le(int x, int y) {
	int sign_x = !!(x>>31);//x>0 -> 0, x<0 -> 1
	int eq=!!((x^y)>>31);//diff->1, same->0
	int d=y+~x+1;//y-x
  return (eq&sign_x)+((!eq)&(!(d>>31)));
}


/*
if x and y have different sign bit and x>0 return 0 but x<0 return 1.
And if x and y have same sign bit then y-x>=0 return 1 but y-x<0 return 0.
*/


/* 
 * divide_by_power_of_2 - Return the same value as x/(2^n), for 0 <= n <= 30
 *                        Please round to 0.
 *   Examples: divide_by_power_of_2(15,1) = 7, divide_by_power_of_2(-33,4) = -2
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 15
 *   Points: 2
 */
int divide_by_power_of_2(int x, int n) {
    int sign_x = x>>31; // x>0 -> 0, x<0 -> 0xffffffff
	return (sign_x&((x+(1<<n)+~0)>>n))+((~sign_x)&(x>>n));

}

/*
If x>0, x>>n
If x<0, (x+2^n-1)>>n
*/


/* 
 * bang - return !x, but you should not use ! in the function.
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Operations allowed to use: ~ & ^ | + << >>
 *   Max # of operations: 12
 *   Points: 4 
 */
int bang(int x){	
  return ((x|(~x+1))>>31)+1;
}
/*
we need to check there is 1 or not.
if x is 0 then x|-x is -1, else then x|-x is 0.
so, ((x|-x)>>31)+1 represents !x.
*/


/* 
 * is_subtraction_no_problem - Can x-y be done without any problem(overflow)?
 *   If there is an overflow, return 0. Otherwise, return 1.
 *   Example: is_subtraction_no_problem(0x80000000,0x80000000) = 1,
 *            is_subtraction_no_problem(0x80000000,0x70000000) = 0, 
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 20
 *   Points: 3
 */
int is_subtraction_no_problem(int x, int y) {
	int sign_x = !(x>>31); // x>=0 -> 1, x<0 -> 0
	int eq = !((x^y)>>31); // diff -> 0, same -> 1
	int d = x+~y+1; // x-y
	int sign_d = !(d>>31); // x-y>=0 -> 1, x-y<0 -> 0
  return eq+((!eq)&((sign_x&sign_d)+((!sign_x)&(!sign_d))));
}

/*
If x and y have same sign bit, there's no subtraction overflow occurs.
If x and y have different sign bit, it depends on sign of x and sign of x-y.
If x>0 and y<0, when x-y<0 overflow occurs.
If x<0 and y>0, when x-y>0 overflow occurs.
Consider 4 different cases, we can make solution.
*/

/* 
 * aeb - If even-numbered bit in a word is set at least one, return 1.
 *   Examples aeb(0xA) = 0, aeb(0xE) = 1
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 12
 *   Points: 2
 */
int aeb(int x) {
	int det1 = 0x55<<24;//dectector 0x55000000
	int det2 = 0x55<<16;//0x00550000
	int det3 = 0x55<<8;//0x00005500
	int det = 0x55+det3+det2+det1;
  return !!(x&det);
}
/*
To check that there's even-numbered bits in a word, use '&' with 0x55555555 to x.
*/
