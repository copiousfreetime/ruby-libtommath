#include "ruby.h"
#include <tommath.h> 
#include "ltm.h"
#include <math.h>

/**********************************************************************
 *                             Prototypes                             *
 **********************************************************************/

/* Class */
VALUE cLT_M_Prime;

/*
 * call-seq:
 *  num_miller_rabin_trials_for(n)
 *
 * Calculate the number of Miller-Rabin trials necessary to get a 2**-92
 * or lower probability of failure for a given _n_-bit size Bignum that
 * may be prime.
 */
VALUE ltm_prime_num_miller_rabin_trials(VALUE self, VALUE other)
{
    mp_int *a    = NUM2MP_INT(other);
    int num_bits = mp_count_bits(a);
    int mp_result;

    /* do not ask me why its rabin_miller here and miller_rabin in the
     * other method.
     */
    mp_result = mp_prime_rabin_miller_trials(num_bits);

    return INT2FIX(mp_result);
}


/*
 * call-seq:
 *  random_of_size( n, options = Hash.new ) -> bignum
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
VALUE ltm_prime_random_of_size(int argc, VALUE* argv, VALUE self)
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


