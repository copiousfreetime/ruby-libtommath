#include "ltm.h"

/**********************************************************************
 *                     Internally used functions                      *
 **********************************************************************/

#if LTM_DEBUG
/*
 *   This is only used in debugging.  The memory allocated by this
 *   method must be free()'d by the caller.
 */
char * mp_int_to_s(mp_int *a)
{   
    int mp_size,mp_result;
    char *mp_str;
    if (MP_OKAY != (mp_result = mp_radix_size(a,10,&mp_size))) {
        rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
    }

    mp_str = ALLOC_N(char,mp_size);
    if (MP_OKAY != (mp_result = mp_toradix(a,mp_str,10))) {
        rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
    }

    return mp_str;
}
#endif


/*
 * extract mp_int from any number, includes conversion to a temporary
 * Bignum if necessary.
 */
mp_int* num_to_mp_int(VALUE i)
{
    mp_int* result;

    if (IS_LTM_BIGNUM(i)) {
        result = MP_INT(i);
    } else {
        switch (TYPE(i)) {
            case T_FIXNUM:
            case T_BIGNUM:
            case T_FLOAT:
                result = MP_INT(NEW_LTM_BIGNUM_FROM(i));
                break;
            default:
                rb_raise(rb_eTypeError, "unable to convert %s to a Bignum",
                    rb_obj_classname(i));
                break;
        }
    }
    return result;
}


/*
 * See if the given Value has the integer value 2
 */
VALUE is_2(VALUE obj)
{
    mp_int *a = NULL;
    int val = -1;
    int digit_count = 0;

    switch (TYPE(obj)) {
    case T_FIXNUM:
        val = FIX2INT(obj);
        break;
    case T_FLOAT:
        val = NUM2INT(obj);
        break;
    case T_BIGNUM:
        if ((RBIGNUM(obj)->len == 1)) {
            val = NUM2INT(obj);
        }
        break;
    case T_DATA:
        if (IS_LTM_BIGNUM(obj)) {
            a = MP_INT(obj);
            if (MP_OKAY == mp_radix_size(a,10,&digit_count)) {
                /* digit count is the number of digits + space for the 
                 * '\0' in the string.  Normally mp_radix_size is used
                 * to allocate space for a string to hold the number
                 */
                if (2 == digit_count) {
                    val = (int)mp_get_int(a);
                    if (SIGN(a) == MP_NEG) {
                        val = -val;
                    }
                }
            }
        }
        break;
    default:
        /* nothing to do here, if it is not numeric then it we return
         * false 
         * */
        break;
    }

    return (2 == val) ? Qtrue : Qfalse;
}


/*
 * Convert a VALUE into the internal struct we use for libtommmath
 */
mp_int* value_to_mp_int(VALUE obj)
{
    mp_int* bn;

    if (IS_LTM_BIGNUM(obj)) {
        Data_Get_Struct(obj,mp_int,bn);
    } else {
        rb_raise(rb_eTypeError,"not a Bignum");
    }
    return bn;
}

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

/**********************************************************************
 *                   Ruby Object life-cycle methods                   *
 **********************************************************************/

/*
 *  * garbage collector free method for mp_int structures
 *   */
static void ltm_bignum_free(mp_int *bn) 
{
        mp_clear(bn);
            free(bn);
                return ;
}

/*
 * Allocator for mp_int structures
 */
VALUE ltm_bignum_alloc(VALUE klass)
{
    mp_int *bn = ALLOC(mp_int);
    VALUE  obj = (VALUE)NULL;
    int mp_result = MP_OKAY;

    if (MP_OKAY == (mp_result = mp_init(bn))) {
        obj = Data_Wrap_Struct(klass,NULL,ltm_bignum_free,bn);
    } else {
        rb_raise(eLT_M_Error, "Failure to allocate Bignum: %s", mp_error_to_string(mp_result));
    }    
    return obj; 
}

