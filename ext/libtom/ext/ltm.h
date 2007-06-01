#ifndef __LTM__
#define __LTM__

#include "ruby.h"
#include <math.h>
#include <tommath.h>


/* Module and Class */
extern VALUE mLT; 
extern VALUE mLT_M;
extern VALUE cLT_M_Bignum;
extern VALUE cLT_M_Prime;
extern VALUE eLT_M_Error;

/**********************************************************************
 *                             Prototypes                             *
 **********************************************************************/

/* internal functions, not part of the API */
extern mp_int* value_to_mp_int(VALUE);
extern mp_int* num_to_mp_int(VALUE);
extern int ltm_bignum_random_prime_callback(unsigned char*, int, void*);


/** Bignum **/
extern VALUE is_2(VALUE);
extern VALUE ltm_bignum_abs(VALUE self);
extern VALUE ltm_bignum_add_modulus(VALUE self, VALUE p1, VALUE p2);
extern VALUE ltm_bignum_add(VALUE self, VALUE other);
extern VALUE ltm_bignum_alloc(VALUE);;
extern VALUE ltm_bignum_bit_and(VALUE self, VALUE other);
extern VALUE ltm_bignum_bit_negation(VALUE self);
extern VALUE ltm_bignum_bit_or(VALUE self, VALUE other);
extern VALUE ltm_bignum_bit_ref(VALUE self);
extern VALUE ltm_bignum_bit_xor(VALUE self, VALUE other);
extern VALUE ltm_bignum_coerce(VALUE self, VALUE other);
extern VALUE ltm_bignum_divide(VALUE self, VALUE other);
extern VALUE ltm_bignum_divisible_by_some_primes(VALUE self);
extern VALUE ltm_bignum_divmod(VALUE self, VALUE other);
extern VALUE ltm_bignum_eql(VALUE self, VALUE other);
extern VALUE ltm_bignum_eq(VALUE self, VALUE other);
extern VALUE ltm_bignum_even(VALUE self);
extern VALUE ltm_bignum_exponent_modulus(VALUE self, VALUE p1, VALUE p2);
extern VALUE ltm_bignum_extended_euclidian(VALUE self, VALUE p1);
extern VALUE ltm_bignum_greatest_common_divisor(VALUE self, VALUE p1);
extern VALUE ltm_bignum_hash(VALUE self);
extern VALUE ltm_bignum_initialize_copy(VALUE copy, VALUE orig);
extern VALUE ltm_bignum_initialize(int argc, VALUE *argv, VALUE self);
extern VALUE ltm_bignum_inverse_modulus(VALUE self, VALUE p1);
extern VALUE ltm_bignum_is_prime(int argc, VALUE* argv, VALUE self);
extern VALUE ltm_bignum_is_square(VALUE self);
extern VALUE ltm_bignum_jacobi(VALUE self, VALUE p);
extern VALUE ltm_bignum_least_common_multiple(VALUE self, VALUE p1);
extern VALUE ltm_bignum_left_shift_digits(VALUE self, VALUE other);
extern VALUE ltm_bignum_lshift_bits(VALUE self, VALUE other);
extern VALUE ltm_bignum_modulo_2d(VALUE self, VALUE other);
extern VALUE ltm_bignum_modulo(VALUE self, VALUE other);
extern VALUE ltm_bignum_multiply_modulus(VALUE self, VALUE p1, VALUE p2);
extern VALUE ltm_bignum_multiply(VALUE self, VALUE other);
extern VALUE ltm_bignum_next_prime(int argc, VALUE* argv, VALUE self);
extern VALUE ltm_bignum_nonzero(VALUE self);
extern VALUE ltm_bignum_nth_root(VALUE self, VALUE p1);
extern VALUE ltm_bignum_num_bits(VALUE self);
extern VALUE ltm_bignum_odd(VALUE self);
extern VALUE ltm_bignum_passes_fermat_primality(VALUE self,VALUE p1);
extern VALUE ltm_bignum_passes_miller_rabin(VALUE self,VALUE p1);
extern VALUE ltm_bignum_pow(VALUE self, VALUE other);
extern VALUE ltm_bignum_remainder(VALUE self, VALUE other);
extern VALUE ltm_bignum_right_shift_digits(VALUE self, VALUE other);
extern VALUE ltm_bignum_rshift_bits(VALUE self, VALUE other);
extern VALUE ltm_bignum_size(VALUE self);
extern VALUE ltm_bignum_spaceship(VALUE self, VALUE other);
extern VALUE ltm_bignum_squared(VALUE self);
extern VALUE ltm_bignum_square_modulus(VALUE self, VALUE p1);
extern VALUE ltm_bignum_square_root(VALUE self);
extern VALUE ltm_bignum_subtract_modulus(VALUE self, VALUE p1, VALUE p2);
extern VALUE ltm_bignum_subtract(VALUE self, VALUE other);
extern VALUE ltm_bignum_to_f(VALUE self);
extern VALUE ltm_bignum_to_s(int argc, VALUE *argv, VALUE self);
extern VALUE ltm_bignum_uminus(VALUE self);
extern VALUE ltm_bignum_zero_bang(VALUE self);
extern VALUE ltm_bignum_zero(VALUE self);

/** Prime **/

/**********************************************************************
 *                           Useful MACROS                            *
 **********************************************************************/

#define MP_INT(obj) (value_to_mp_int(obj))
#define MP_CHAR_SIGN(mp) ((SIGN(mp)==MP_ZPOS)?('+'):('-'))
#define IS_LTM_BIGNUM(obj) (Qtrue == rb_obj_is_instance_of(obj,cLT_M_Bignum))
#define ALLOC_LTM_BIGNUM (ltm_bignum_alloc(cLT_M_Bignum))
#define NEW_LTM_BIGNUM_FROM(other) (rb_class_new_instance(1,&other,cLT_M_Bignum))
#define NUM2MP_INT(obj) (num_to_mp_int(obj))
#define IS_2(obj) (Qtrue == is_2(obj))

#endif

