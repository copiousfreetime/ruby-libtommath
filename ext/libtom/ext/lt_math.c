#include "ruby.h"
#include <tommath.h> 
#include "ltm.h"
#include <math.h>

/**********************************************************************
 *                             Prototypes                             *
 **********************************************************************/

/* Module and Class */
VALUE mLT;
VALUE mLT_M;
VALUE eLT_M_Error;

/*
 * call-seq:
 *  LibTom::Math.pow2(n) -> bignum
 *  LibTom::Math.two_to_the(n) -> bignum
 *
 * Calculates 2**n and returns the value as a Bignum.
 */
static VALUE ltm_two_to_the(VALUE self, VALUE other)
{
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *a    = MP_INT(result);
    int power    = NUM2INT(other);
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_2expt(a,power))) {
        rb_raise(eLT_M_Error, "Failure calculating 2**%d : %s\n",
            power,mp_error_to_string(mp_result));
    }
    return result;
}




/**********************************************************************
 *                   Ruby extension initialization                    *
 **********************************************************************/

/*
 * Interface to the LibTomMath[http://libtom.org/?page=features&newsitems=5&whatfile=ltm]
 * free open source portable number theoretic multiple-precision integer
 * library
 */
void Init_math()
{

    /*
     * module LibTom
     */
    mLT = rb_define_module("LibTom");

    /*
     * module Math
     */
    mLT_M = rb_define_module_under(mLT,"Math");
    
    /* LibTom::Math:: <methods> */
    rb_define_module_function(mLT_M,"pow2",ltm_two_to_the,1);
    rb_define_module_function(mLT_M,"two_to_the",ltm_two_to_the,1);

    /*
     * class LibTom::Math::Bignum
     */
    cLT_M_Bignum = rb_define_class_under(mLT_M,"Bignum",rb_cNumeric);
    rb_define_alloc_func(cLT_M_Bignum,ltm_bignum_alloc);
    rb_define_singleton_method(cLT_M_Bignum,"random_of_size",ltm_bignum_random_of_size,1);
    rb_define_method(cLT_M_Bignum,"initialize",ltm_bignum_initialize,-1);
    rb_define_method(cLT_M_Bignum,"initialize_copy",ltm_bignum_initialize_copy,1);
    rb_define_method(cLT_M_Bignum,"to_s",ltm_bignum_to_s, -1); 

    /* comparison operators */
    rb_define_method(cLT_M_Bignum, "==",ltm_bignum_eq, 1);
    rb_define_method(cLT_M_Bignum, "eql?",ltm_bignum_eql, 1);
    rb_define_method(cLT_M_Bignum, "<=>", ltm_bignum_spaceship, 1);

    /* mathematical operators */
    rb_define_method(cLT_M_Bignum,"coerce", ltm_bignum_coerce, 1);
    rb_define_method(cLT_M_Bignum, "-@", ltm_bignum_uminus, 0);
    rb_define_method(cLT_M_Bignum, "abs", ltm_bignum_abs, 0);
    rb_define_method(cLT_M_Bignum, "+", ltm_bignum_add, 1);
    rb_define_alias(cLT_M_Bignum, "add", "+");
    rb_define_method(cLT_M_Bignum, "-", ltm_bignum_subtract, 1);
    rb_define_alias(cLT_M_Bignum, "subtract", "-");
    rb_define_method(cLT_M_Bignum, "*", ltm_bignum_multiply, 1);
    rb_define_alias(cLT_M_Bignum, "multiply", "*");
    rb_define_method(cLT_M_Bignum, "/", ltm_bignum_divide, 1);
    rb_define_alias(cLT_M_Bignum, "divide", "/");
    rb_define_alias(cLT_M_Bignum, "quo", "/");
    rb_define_alias(cLT_M_Bignum, "div", "/");
    rb_define_method(cLT_M_Bignum, "remainder",ltm_bignum_remainder, 1);
    rb_define_method(cLT_M_Bignum, "%", ltm_bignum_modulo, 1);
    rb_define_alias(cLT_M_Bignum, "mod","%");
    rb_define_alias(cLT_M_Bignum, "modulo","%");
    rb_define_method(cLT_M_Bignum, "divmod",ltm_bignum_divmod, 1);
    rb_define_method(cLT_M_Bignum, "**",ltm_bignum_pow, 1);

    /* utility methods */
    rb_define_method(cLT_M_Bignum, "size",ltm_bignum_size, 0);
    rb_define_method(cLT_M_Bignum, "to_f",ltm_bignum_to_f, 0);
    rb_define_method(cLT_M_Bignum, "even?",ltm_bignum_even, 0);
    rb_define_method(cLT_M_Bignum, "odd?",ltm_bignum_odd, 0);
    rb_define_method(cLT_M_Bignum, "nonzero?",ltm_bignum_nonzero,0);
    rb_define_method(cLT_M_Bignum, "zero?",ltm_bignum_zero,0);
    rb_define_method(cLT_M_Bignum, "zero!",ltm_bignum_zero_bang,0);
    rb_define_method(cLT_M_Bignum, "hash",ltm_bignum_hash, 0);
    rb_define_method(cLT_M_Bignum, "num_bits",ltm_bignum_num_bits,0);

    /* logical / bitwise  operators */
    rb_define_method(cLT_M_Bignum, "&",ltm_bignum_bit_and, 1);
    rb_define_method(cLT_M_Bignum, "|",ltm_bignum_bit_or, 1);
    rb_define_method(cLT_M_Bignum, "^",ltm_bignum_bit_xor, 1);
    rb_define_method(cLT_M_Bignum, "<<",ltm_bignum_lshift_bits, 1);
    rb_define_method(cLT_M_Bignum, ">>",ltm_bignum_rshift_bits, 1);

    /* Bit wise negation and accessing a single bit as a bit vector, not
     * explicitly supported by libtom math.  They may be able to be 
     * hacked around, but it may not be worth it.  Currently these throw
     * NotImplementedErrors
     * 
     */         
    rb_define_method(cLT_M_Bignum, "~",ltm_bignum_bit_negation, 0);
    rb_define_method(cLT_M_Bignum, "[]",ltm_bignum_bit_ref, 1);
                            
    /* Additional methods that are provide by libtommath */
    rb_define_method(cLT_M_Bignum,"right_shift_digits",ltm_bignum_right_shift_digits,1);
    rb_define_alias(cLT_M_Bignum,"divide_by_x_pow_n","right_shift_digits");
    rb_define_method(cLT_M_Bignum,"left_shift_digits",ltm_bignum_left_shift_digits,1);
    rb_define_alias(cLT_M_Bignum,"multiply_by_x_pow_n","left_shift_digits");
                                                
    rb_define_method(cLT_M_Bignum,"squared",ltm_bignum_squared,0);
    rb_define_method(cLT_M_Bignum,"mod_2n",ltm_bignum_modulo_2d,1);
    rb_define_alias(cLT_M_Bignum,"mod_pow2","mod_2n");  
    rb_define_alias(cLT_M_Bignum,"modulo_2n","mod_2n");     
    rb_define_alias(cLT_M_Bignum,"modulo_pow2","mod_2n");       
                                                                     
    rb_define_method(cLT_M_Bignum,"add_modulus",ltm_bignum_add_modulus,2);
    rb_define_method(cLT_M_Bignum,"subtract_modulus",ltm_bignum_subtract_modulus,2);
    rb_define_method(cLT_M_Bignum,"multiply_modulus",ltm_bignum_multiply_modulus,2);
    rb_define_method(cLT_M_Bignum,"square_modulus",ltm_bignum_square_modulus,1);
    rb_define_method(cLT_M_Bignum,"exponent_modulus",ltm_bignum_exponent_modulus,2);
    rb_define_method(cLT_M_Bignum,"inverse_modulus",ltm_bignum_inverse_modulus,1);
    rb_define_method(cLT_M_Bignum,"greatest_common_divisor",ltm_bignum_greatest_common_divisor,1);
    rb_define_alias(cLT_M_Bignum,"gcd","greatest_common_divisor");
    rb_define_method(cLT_M_Bignum,"extended_euclidian",ltm_bignum_extended_euclidian,1);
    rb_define_method(cLT_M_Bignum,"least_common_multiple",ltm_bignum_least_common_multiple,1);
    rb_define_alias(cLT_M_Bignum,"lcm","least_common_multiple");
    rb_define_method(cLT_M_Bignum,"nth_root",ltm_bignum_nth_root,1);
    rb_define_method(cLT_M_Bignum,"square_root",ltm_bignum_square_root,0);
    rb_define_method(cLT_M_Bignum,"is_square?",ltm_bignum_is_square,0);
    rb_define_method(cLT_M_Bignum,"jacobi",ltm_bignum_jacobi,1);

    /* Prime number methods */
    rb_define_method(cLT_M_Bignum,"passes_fermat_primality?",ltm_bignum_passes_fermat_primality,1);
    rb_define_method(cLT_M_Bignum,"passes_miller_rabin?",ltm_bignum_passes_miller_rabin,1);
    rb_define_method(cLT_M_Bignum,"is_divisible_by_some_primes?",ltm_bignum_divisible_by_some_primes,0);
    rb_define_method(cLT_M_Bignum,"is_prime?",ltm_bignum_is_prime,-1);
    rb_define_method(cLT_M_Bignum,"next_prime",ltm_bignum_next_prime,-1);

   
    /*
     * class Prime
     */
    cLT_M_Prime = rb_define_class_under(mLT_M,"Prime",rb_cObject);
    rb_define_singleton_method(cLT_M_Prime,"num_miller_rabin_trials_for",ltm_prime_num_miller_rabin_trials,1);
    rb_define_singleton_method(cLT_M_Prime,"random_of_size",ltm_prime_random_of_size,-1);
}
