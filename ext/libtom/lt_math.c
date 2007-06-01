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
 * random_prime callback.  Uses the ruby rand method to fill a buffer of
 * length N with bytes and return the buffer
 */
int ltm_bignum_random_prime_callback(unsigned char *buf, int len, void *dat)
{
    int i ; 
    VALUE max = INT2NUM(256);
    VALUE num;

    for (i = 0 ; i < len ; i++) {
        num = rb_funcall(rb_mKernel,rb_intern("rand"),1,max);
        buf[i] = 0xff & (NUM2INT(num));
    }

    return i;
}

/*
 * call-seq:
 *  LibTom::Math.pow2(n) -> bignum
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


/*
 * call-seq:
 *  LibTom::Math.rand_of_size(n) -> bignum
 *
 * Generate a pseudo-random Bignum of at least _n_ bits and return it.
 */
static VALUE ltm_rand_of_size(VALUE self, VALUE other)
{
    VALUE result    = ALLOC_LTM_BIGNUM;
    mp_int *a       = MP_INT(result);
    double num_bits = NUM2DBL(other);
    int num_digits  = (int)ceil(num_bits / MP_DIGIT_BIT);
    int mp_result;

    if (MP_OKAY != (mp_result = mp_rand(a,num_digits))) {
        rb_raise(eLT_M_Error, "Failure calculating rand with %d bits: %s\n",
            num_bits,mp_error_to_string(mp_result));
    }
    return result;
}


/*
 * call-seq:
 *  LibTom::Math.num_miller_rabin_trials(n)
 *
 * Calculate the number of Miller-Rabin trials necessary to get a 2**-92
 * or lower probability of failure for a given _n_-bit size Bignum.
 */
static VALUE ltm_num_miller_rabin_trials(VALUE self, VALUE other)
{
    int num_bits    = NUM2INT(other);
    int mp_result;

    /* do not ask me why its rabin_miller here and miller_rabin in the
     * other method.
     */
    mp_result = mp_prime_rabin_miller_trials(num_bits);

    return INT2FIX(mp_result);
}


/*
 * call-seq:
 *  LibTome::Math.random_prime( n, options = Hash.new ) -> bignum
 * 
 * Generates a random prime of at least _n_ bits in length.  The
 * _options_ can be:
 *
 * <b><tt>:trials</tt></b>::        The number of Miller-Rabin trails the new
 *                                  prime must pass.  The default is the same
 *                                  as that used in is_prime?.
 * <b><tt>:congruency</tt></b>::    +true+ or +false+.  Should the prime
 *                                  returned be congruent to 3 mod 4.
 *                                  The default is +false+.
 * <b><tt>:safe</tt></b>::          The number returned (_p_) shall also
 *                                  satisfy ((_p_ - 1)/2).is_prime?.
 *                                  This implies *:congruency* <tt>=> true</tt>.
 * <b><tt>:msb</tt></b>::           Set this to +true+ to forece the 2nd
 *                                  most significant bit of the resulting 
 *                                  prime to be 1.
 */
static VALUE ltm_random_prime(int argc, VALUE* argv, VALUE self)
{
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *a    = MP_INT(result);

    int num_bits;
    VALUE options;
    VALUE value;
    int trials = 0;
    int trials_option = 0;
    int flags = 0;
    int mp_result;

    if (argc <= 0) {
        rb_raise(rb_eArgError,"A number of bits for the random prime is required.");
    } else {
        num_bits = NUM2INT(argv[0]);
        if (argc > 1) {
            options = argv[1] ;

            /* check for the :trials, :congruency, :safe and :msb  options */
            if (rb_obj_is_kind_of(options,rb_cHash)) {
                /* :trials */
                value = rb_hash_aref(options,ID2SYM(rb_intern("trials")));
                if (Qnil != value) {
                    trials = NUM2INT(value);
                    trials_option = 1;
                }

                /* :congruency */
                value = rb_hash_aref(options,ID2SYM(rb_intern("congruency")));
                if ((Qnil != value) && (Qtrue == value)) {
                    flags = flags | LTM_PRIME_BBS;
                }

                /* :safe */
                value = rb_hash_aref(options,ID2SYM(rb_intern("safe")));
                if ((Qnil != value) && (Qtrue == value)) {
                    flags = flags | LTM_PRIME_BBS;
                    flags = flags | LTM_PRIME_SAFE;
                }

                /* :msb */
                value = rb_hash_aref(options,ID2SYM(rb_intern("msb")));
                if ((Qnil != value) && (Qtrue == value)) {
                    flags |= LTM_PRIME_2MSB_ON;
                }
            } /* options hash */
        } /* argc */
    } /* else */

    /* calculate a reasonable default for trials */
    if (!trials_option && (trials <= 0)) {
        trials = mp_prime_rabin_miller_trials(num_bits);
    }

    if (MP_OKAY != (mp_result = mp_prime_random_ex(a,trials,num_bits,flags,
                                                   ltm_bignum_random_prime_callback,NULL))) {
            rb_raise(eLT_M_Error, "Failure to find a %d bit random prime: %s", 
                num_bits,mp_error_to_string(mp_result));
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
    /* module definitions */
    mLT = rb_define_module("LibTom");
    mLT_M = rb_define_module_under(mLT,"Math");
    
    /* 2**d */
    rb_define_module_function(mLT_M,"pow2",ltm_two_to_the,1);
    rb_define_module_function(mLT_M,"two_to_the",ltm_two_to_the,1);
    rb_define_module_function(mLT_M,"rand_of_size",ltm_rand_of_size,1);
    rb_define_module_function(mLT_M,"num_miller_rabin_trials",ltm_num_miller_rabin_trials,1);
    rb_define_module_function(mLT_M,"random_prime",ltm_random_prime,-1);

}
