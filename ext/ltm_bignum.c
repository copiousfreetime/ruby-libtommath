#include "ruby.h"
#include <tommath.h> 
#include <math.h>

/**********************************************************************
 *                             Prototypes                             *
 **********************************************************************/

/* internal functions, not part of the API */
static mp_int* value_to_mp_int(VALUE obj);
static VALUE is_2(VALUE obj);


/* Module and Class */
static VALUE mLT;
static VALUE mLT_M;
static VALUE cLT_M_Bignum;
static VALUE eLT_M_Error;

/* Ruby Object life-cycle methods */
static void ltm_bignum_free(mp_int *bn);
static VALUE ltm_bignum_alloc(VALUE klass);
static VALUE ltm_bignum_initialize(int argc, VALUE *argv, VALUE self);

/* Class instance methods */
static VALUE ltm_bignum_coerce(VALUE self, VALUE other);

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

/**********************************************************************
 *                     Internally used functions                      *
 **********************************************************************/

#if 1
/*
 *   This is only used in debugging
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
        result = MP_INT(NEW_LTM_BIGNUM_FROM(i));
    }
    return result;
}


/*
 * See if the given Value has the integer value 2
 */
static VALUE is_2(VALUE obj)
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
static mp_int* value_to_mp_int(VALUE obj)
{
    mp_int* bn;

    if (IS_LTM_BIGNUM(obj)) {
        Data_Get_Struct(obj,mp_int,bn);
    } else {
        rb_raise(rb_eTypeError,"not a Bignum");
    }
    return bn;
}

/**********************************************************************
 *                       Class Instance methods                       *
 **********************************************************************/

/*
 * Numeric coercion protocol.
 */
static VALUE ltm_bignum_coerce(VALUE self, VALUE other)
{
    VALUE result = rb_ary_new2(2);
    VALUE param;
    VALUE receiver;

    switch (TYPE(other)) {
        case T_FIXNUM:
        case T_BIGNUM:
        case T_FLOAT:
            param = NEW_LTM_BIGNUM_FROM(other);
            receiver = self;
            break;
            /* FIXME
        case T_FLOAT:
            param = other;
            receiver = ltm_bignum_to_float(MP_INT(self));
            break
            */
        default:
            rb_raise(rb_eTypeError, "can't coerce %s to LTMBignum",
                    rb_obj_classname(other));
            break; 
    }

    rb_ary_push(result,param);
    rb_ary_push(result,receiver);
    return result;
}

/*
 * call-seq:
 *  -Bignum
 *
 * returns a copy of the current Bignum with its sign inverted
 */
static VALUE ltm_bignum_uminus(VALUE self)
{
    mp_int *a     = MP_INT(self);
    VALUE b_value = rb_funcall(self,rb_intern("dup"),0);
    mp_int *b     = MP_INT(b_value);
    int mp_result;

    if (MP_OKAY != (mp_result = mp_neg(a,b))) {
        rb_raise(eLT_M_Error,"Unable to negate Bignum: %s", 
                mp_error_to_string(mp_result));
    }
    return b_value;
}


/*
 * call-seq:
 *  Bignum.abs
 *
 * returns the absolute value of bignum
 */
static VALUE ltm_bignum_abs(VALUE self)
{
    mp_int *a     = MP_INT(self);
    VALUE b_value = rb_funcall(self,rb_intern("dup"),0);
    mp_int *b     = MP_INT(b_value);
    int mp_result;

    if (MP_OKAY != (mp_result = mp_abs(a,b))) {
        rb_raise(eLT_M_Error,"Failure to take absolute value of Bignum: %s", 
                mp_error_to_string(mp_result));
    }
    return b_value;
}


/*
 * call-seq:
 *   bignum + bignum
 *
 * returns the sum of two bignums
 */
static VALUE ltm_bignum_add(VALUE self, VALUE other)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(other);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;

    if (MP_OKAY != (mp_result = mp_add(a,b,c))) {
        rb_raise(eLT_M_Error,"Failure to add two Bignums: %s", 
                mp_error_to_string(mp_result));
    }

    return result;
}


/*
 * call-seq:
 *   bignum - bignum
 *
 * returns the difference of two bignums
 */
static VALUE ltm_bignum_subtract(VALUE self, VALUE other)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(other);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;

    if (MP_OKAY != (mp_result = mp_sub(a,b,c))) {
        rb_raise(eLT_M_Error,"Failure to subtract two Bignums: %s", 
                mp_error_to_string(mp_result));
    }

    return result;
}


/*
 * call-seq:
 *  Bignum == obj  => true or false
 *
 * Returns true only if obj has the same value as Bignum.  This converts
 * non Bignum's to Bignums and does the comparison that way.
 */
static VALUE ltm_bignum_eq(VALUE self, VALUE other)
{
    mp_int *a = MP_INT(self);
    mp_int *b = NUM2MP_INT(other);  

    if (MP_EQ == mp_cmp(a,b)) {
        return Qtrue;
    } 
    return Qfalse;
}


/*
 * call-seq:
 *  Bignum.eql?(obj) => true or false
 *
 * Returns true only if obj is a Bignum and they are equivalent
 */
static VALUE ltm_bignum_eql(VALUE self, VALUE other)
{
    mp_int *a = MP_INT(self);
    mp_int *b;

    if (IS_LTM_BIGNUM(other)) {
        b = MP_INT(other);
        if (MP_EQ == mp_cmp(a,b)) {
            return Qtrue;
        } 
    }
    return Qfalse;
}


/**
 * call-seq:
 *  a.to_s(radix = 10) -> String
 *
 * convert the given Bignum to a string with base(radix). Radix can be a
 * value in the range 2..64
 */
static VALUE ltm_bignum_to_s(int argc, VALUE *argv, VALUE self)
{
    mp_int *a      = MP_INT(self);
    int radix      = 10;
    int mp_size    = 0;
    int mp_result;
    VALUE result;
    char *mp_str;

    /* converting to a particular base */
    if (argc > 0) {
        radix = NUM2INT(argv[0]);
    }

    /* documentation of libtom math says that radix is [2,64] */
    if ((radix < 2) || (radix > 64)) {
        rb_raise(rb_eArgError, "radix must be betwen 2 and 64 inclusive");
    }

    /* create a local string then convert it to an RSTRING */
    if (MP_OKAY != (mp_result = mp_radix_size(a,radix,&mp_size))) {
        rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
    }

    mp_str = ALLOC_N(char,mp_size);
    if (MP_OKAY != (mp_result = mp_toradix(a,mp_str,radix))) {
        rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
    }
    result = rb_str_new(mp_str,mp_size-1); 
    free(mp_str);
    return result;
}


/*
 * call-seq:
 *   bignum * bignum
 *
 * returns the multiplication of a * b
 */
static VALUE ltm_bignum_multiply(VALUE self, VALUE other)
{
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    mp_int *a;
    int mp_result;
    int self_is_2, other_is_2;

    /* first find out if one of the values is a 2 or not.  Then we can use the fast
     * multiplier
     */
    self_is_2 = IS_2(self);
    other_is_2 = IS_2(other);
   
    /* using fast multiplier */
    if (self_is_2 || other_is_2) {
        if (self_is_2 && IS_LTM_BIGNUM(other)) {
            a = MP_INT(other);
        } else {
            a = MP_INT(self);
        }
        if (MP_OKAY != (mp_result = mp_mul_2(a,c))) {
            rb_raise(eLT_M_Error,"Failure to multiply Bignums: %s", 
                mp_error_to_string(mp_result));
        }
    } else {
        /* both of the operands are is not 2, so use the normal multiplier */
        mp_int *b;

        a = MP_INT(self);
        b = NUM2MP_INT(other);

        if (MP_OKAY != (mp_result = mp_mul(a,b,c))) {
            rb_raise(eLT_M_Error,"Failure to multiply Bignums: %s", 
                    mp_error_to_string(mp_result));
        }
    }

    return result;
}


/*
 * call-seq:
 *   bignum / bignum
 *
 * returns the division of a / b
 */
static VALUE ltm_bignum_divide(VALUE self, VALUE other)
{
    mp_int *a    = MP_INT(self);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;
    
    /* first find out if b is 2 or not.  Then we can use the fast
     * divisor
     */
    if (IS_2(other)) {
        if (MP_OKAY != (mp_result = mp_div_2(a,c))) {
            rb_raise(eLT_M_Error,"Failure to divide Bignums: %s", 
                mp_error_to_string(mp_result));
        }
    } else {
        /* both of the operands are not 2, so use the normal divisor */
        mp_int *b;

        a = MP_INT(self);
        b = NUM2MP_INT(other);

        if (MP_OKAY != (mp_result = mp_div(a,b,c,NULL))) {
            if (MP_VAL == mp_result) {
                rb_raise(rb_eZeroDivError,"divide by 0");
            }
            rb_raise(eLT_M_Error,"Failure to divide Bignums: %s", 
                    mp_error_to_string(mp_result));
        }
    }

    return result;
}


/*
 * call-seq:
 *   a.remainder(b)
 *
 * returns the remainder of the division of a / b
 */
static VALUE ltm_bignum_remainder(VALUE self, VALUE other)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(other);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_div(a,b,NULL,c))) {
        if (MP_VAL == mp_result) {
            rb_raise(rb_eZeroDivError,"divide by 0");
        }
        rb_raise(eLT_M_Error,"Failure to divide Bignum: %s", 
                mp_error_to_string(mp_result));
    }

    return result;
}


/*
 * call-seq:
 *   bignum % bignum
 *
 * returns the result of a % b
 */
static VALUE ltm_bignum_modulo(VALUE self, VALUE other)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(other);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;

    if (MP_OKAY != (mp_result = mp_mod(a,b,c))) {
        if (MP_VAL == mp_result) {
            rb_raise(rb_eZeroDivError,"divide by 0");
        }
        rb_raise(eLT_M_Error,"Failure to modulo Bignum: %s", 
                mp_error_to_string(mp_result));
    }

    return result;
}

/*
 * call-seq:
 *   a.divmod(b) => [ a.div(b), a.mod(b) ]
 *
 * returns an array holding [ a.div(b), a.mod(b) ]
 */
static VALUE ltm_bignum_divmod(VALUE self, VALUE other)
{
    mp_int *a = MP_INT(self);
    mp_int *b = NUM2MP_INT(other);
    int mp_result;

    VALUE div = ALLOC_LTM_BIGNUM;
    VALUE mod = ALLOC_LTM_BIGNUM;

    mp_int *m_div= MP_INT(div);
    mp_int *m_mod = MP_INT(mod);


    /* calc div */
    if (MP_OKAY != (mp_result = mp_div(a,b,m_div,NULL))) {
        if (MP_VAL == mp_result) {
            rb_raise(rb_eZeroDivError,"divide by 0");
        }
        rb_raise(eLT_M_Error,"Failure to divmod Bignum: %s", 
                mp_error_to_string(mp_result));
    }


    /* calc mod */
    if (MP_OKAY != (mp_result = mp_mod(a,b,m_mod))) {
        if (MP_VAL == mp_result) {
            rb_raise(rb_eZeroDivError,"divide by 0");
        }
        rb_raise(eLT_M_Error,"Failure to divmod Bignum: %s", 
                mp_error_to_string(mp_result));
    }

    return rb_ary_new3(2,div,mod);
}

/*
 * call-seq:
 *  a.size -> number of bytes in machine representation
 *
 *  This always has an extra byte on the front, 0 byte for positive #'s
 *  1 byte for negative numbers.  This also does not pad to the nearest
 *  word size.
 */

static VALUE ltm_bignum_size(VALUE self)
{
    mp_int *a = MP_INT(self);
    int size = mp_signed_bin_size(a);
    return INT2FIX(size);
}


/*
 * call-seq
 *  a.to_f -> float
 *
 *  returns the floating point value of the bignum, if it is larger than
 *  can be represented, Infinity is returned.
 */
static VALUE ltm_bignum_to_f(VALUE self)
{
    mp_int *a   = MP_INT(self);
    double f    = HUGE_VAL; /* assume that self is > Float::MAX */

    VALUE tmp_float;
    VALUE tmp_string;
    int mp_result;
    int mp_size;
    char *mp_str;

    /* do an endrun around the issue, convert ourselves to a string and
     * then convert the string to a float
     */
    if (MP_OKAY != (mp_result = mp_radix_size(a,10,&mp_size))) {
        rb_raise(eLT_M_Error, "Unable to convert Bignum to Float: %s", 
                mp_error_to_string(mp_result));
    }

    mp_str = ALLOC_N(char,mp_size);

    if (MP_OKAY != (mp_result = mp_toradix(a,mp_str,10))) {
        rb_raise(eLT_M_Error, "Unable to convert Bignum to Float: %s", 
                mp_error_to_string(mp_result));
    }

    tmp_string = rb_str_new2(mp_str); 
    free(mp_str);

    /* convert the new ruby string into a float then convert that to a float  */
    tmp_float =  rb_funcall(tmp_string,rb_intern("to_f"),0);
    f = NUM2DBL(tmp_float);

    return rb_float_new(f);
}

/*
 * spaceship operator used by Comparable
 */
static VALUE ltm_bignum_spaceship(VALUE self, VALUE other)
{
    mp_int *a = MP_INT(self);
    mp_int *b = NUM2MP_INT(other);

    switch (mp_cmp(a,b)) {
        case MP_LT:
            return INT2FIX(-1);
            break;
        case MP_EQ:
            return INT2FIX(0);
            break;
        case MP_GT:
            return INT2FIX(1);
            break;
        default:
            rb_raise(cLT_M_Bignum,"libtommath API error, mp_cmp returned invalid");
            break;
    }

}


/*
 * call-seq:
 *  a.nonzero?
 *
 */
static VALUE ltm_bignum_nonzero(VALUE self)
{
    mp_int *a = MP_INT(self);

    return (mp_iszero(a) ? Qfalse : Qtrue );
}

/*
 * call-seq:
 *  a.zero?
 *
 */
static VALUE ltm_bignum_zero(VALUE self)
{
    mp_int *a = MP_INT(self);

    return (mp_iszero(a) ? Qtrue : Qfalse);
}

/*
 * call-seq:
 *  a.zero!
 *
 * destroy the contents of the Bignum and make it Zero
 */
static VALUE ltm_bignum_zero_bang(VALUE self)
{
    mp_int *a = MP_INT(self);
    
    mp_zero(a);

    return self;
}


/*
 * call-seq:
 *  a.even?
 *
 */
static VALUE ltm_bignum_even(VALUE self)
{
    mp_int *a = MP_INT(self);

    return (mp_iseven(a) ? Qtrue : Qfalse);
}


/*
 * call-seq:
 *  a.odd?
 *
 */
static VALUE ltm_bignum_odd(VALUE self)
{
    mp_int *a = MP_INT(self);

    return (mp_isodd(a) ? Qtrue : Qfalse);
}


/*
 * call-seq:
 *  a.hash
 *
 *  formulate a hash value based on the content of the Bignum
 */
static VALUE ltm_bignum_hash(VALUE self)
{
    mp_int *a = MP_INT(self);
    long key = 0L;
    int len = a->alloc; /* found this by reading the libtommath source */
    int i;

    /* pulled this hash algorithm from ruby's bignum.c */
    for (i = 0 ; i < len; i++) {
        key ^= (long)DIGIT(a,i);
    }
    return LONG2FIX(key);
}


/*
 * call-seq:
 *  a**b -> bignum
 */
static VALUE ltm_bignum_pow(VALUE self, VALUE other)
{
    mp_int *a    = MP_INT(self);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);

    unsigned long exponent = NUM2LONG(other);
    int mp_result;

    if (MP_OKAY != (mp_result = mp_expt_d(a,(mp_digit)exponent,c))) {
        rb_raise(eLT_M_Error, "Failure raising Bignum to power %lu : %s", 
                exponent, mp_error_to_string(mp_result));
    }

    return result;
}


/*
 * call-seq: 
 *  a & b -> bitwise and
 */
static VALUE ltm_bignum_bit_and(VALUE self, VALUE other)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(other);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;

    if (MP_OKAY != (mp_result = mp_and(a,b,c))) {
        rb_raise(eLT_M_Error, "Failure bitwise AND of Bignum: %s", 
                mp_error_to_string(mp_result));
    }

    return result;
}


/*
 * call-seq: 
 *  a | b -> bitwise or
 */
static VALUE ltm_bignum_bit_or(VALUE self, VALUE other)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(other);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;

    if (MP_OKAY != (mp_result = mp_or(a,b,c))) {
        rb_raise(eLT_M_Error, "Failure bitwise OR of Bignum: %s", 
                mp_error_to_string(mp_result));
    }

    return result;
}

/*
 * call-seq: 
 *  a ^ b -> bitwise xor
 */
static VALUE ltm_bignum_bit_xor(VALUE self, VALUE other)
{

    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(other);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;

    if (MP_OKAY != (mp_result = mp_xor(a,b,c))) {
        rb_raise(eLT_M_Error, "Failure bitwise XOR of Bignum: %s", 
                mp_error_to_string(mp_result));
    }

    return result;
}


/*
 * call-seq: 
 *  a << b -> shift a left by b bits
 */
static VALUE ltm_bignum_lshift_bits(VALUE self, VALUE other)
{
    mp_int *a    = MP_INT(self);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *b    = MP_INT(result);

    unsigned long shift_width = NUM2ULONG(other);
    int mp_result;

    /* copy a into c to start */;
    if (MP_OKAY != (mp_result = mp_copy(a,b))) {
            rb_raise(eLT_M_Error, "Failure left shift of %lu bits Bignum: %s", 
                shift_width,mp_error_to_string(mp_result));
    }

    /* do mp_mul_2d with shift_width as the parameter, this shifts the
     * bits shift_width times
     */
    if (MP_OKAY != (mp_result = mp_mul_2d(b,shift_width,b))) {
        rb_raise(eLT_M_Error, "Failure left shift of %lu bits Bignum: %s", 
            shift_width,mp_error_to_string(mp_result));
    }

    return result;
}


/*
 * call-seq: 
 *  a >> b -> shift a right by b bits
 */
static VALUE ltm_bignum_rshift_bits(VALUE self, VALUE other)
{
    mp_int *a    = MP_INT(self);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *b    = MP_INT(result);

    unsigned long shift_width = NUM2ULONG(other);
    int mp_result;

    /* copy a into b to start */;
    if (MP_OKAY != (mp_result = mp_copy(a,b))) {
            rb_raise(eLT_M_Error, "Failure right shift of %lu bits Bignum: %s", 
                shift_width,mp_error_to_string(mp_result));
    }

    /* do mp_div_2d with shift_width as the parameter */
    if (MP_OKAY != (mp_result = mp_div_2d(b,shift_width,b,NULL))) {
        rb_raise(eLT_M_Error, "Failure right shift of %lu bits Bignum: %s", 
            shift_width,mp_error_to_string(mp_result));
    }

    return result;
}

/*
 * call-seq:
 *  ~a
 *
 *  This is NOT IMPLEMENTED
 */
static VALUE ltm_bignum_bit_negation(VALUE self)
{
    rb_raise(rb_eNotImpError,"Bitwise negation is not implemented on LibTom::Math::Bignum");
    return self;
}

/*
 * call-seq:
 *  a[i] -> bit value
 *
 *  This is NOT IMPLEMENTED
 */
static VALUE ltm_bignum_bit_ref(VALUE self)
{
    rb_raise(rb_eNotImpError,"Bit Reference access is not implemented on LibTom::Math::Bignum");
    return self;
}


/*
 * call-seq:
 *  a.num_bits
 *
 * return the number of bits used in representing the Bignum
 */
static VALUE ltm_bignum_num_bits(VALUE self)
{
    mp_int *a = MP_INT(self);
    return INT2FIX(mp_count_bits(a));
}


/*
 * call-seq:
 *  a.right_shift_digits(n)
 *
 * shift n digits to the right.  This divides a by x**b
 */
static VALUE ltm_bignum_right_shift_digits(VALUE self, VALUE other)
{
    mp_int *a       = MP_INT(self);
    VALUE result    = ALLOC_LTM_BIGNUM;
    mp_int *b       = MP_INT(result);
    int shift_width = NUM2INT(other);
    int mp_result;

    /* copy a into b to start */;
    if (MP_OKAY != (mp_result = mp_copy(a,b))) {
        rb_raise(eLT_M_Error, "Failure right shift of %d digits on Bignum: %s", 
            shift_width,mp_error_to_string(mp_result));
    }

    /* do some shifting, no return code on a right shift, it cannot
     * fail (at least according to the documentation) 
     */
    mp_rshd(b,shift_width);
    return result;
}


/*
 * call-seq:
 *  a.left_shift_digits(n)
 *
 * shift n digits to the left.  This multiplies a by x**b
 */
static VALUE ltm_bignum_left_shift_digits(VALUE self, VALUE other)
{
    mp_int *a       = MP_INT(self);
    VALUE result    = ALLOC_LTM_BIGNUM;
    mp_int *b       = MP_INT(result);
    int shift_width = NUM2INT(other);
    int mp_result;

    /* copy a into b to start */;
    if (MP_OKAY != (mp_result = mp_copy(a,b))) {
        rb_raise(eLT_M_Error, "Failure left shift of %d digits on Bignum: %s", 
            shift_width,mp_error_to_string(mp_result));
    }

    /* do some shifting */
    if (MP_OKAY != (mp_result = mp_lshd(b,shift_width))) {
        rb_raise(eLT_M_Error, "Failure left shift of %d digits on Bignum: %s", 
            shift_width,mp_error_to_string(mp_result));
    }

    return result;
}


/*
 * call-seq:
 *  LibTom::Math.pow2(n)
 *
 *  returns the value of 2**n as a Bignum
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
 *  a.squared
 *
 * returns a**2
 */
static VALUE ltm_bignum_squared(VALUE self)
{
    mp_int *a    = MP_INT(self);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *b    = MP_INT(result);
    int mp_result ;

    if (MP_OKAY != (mp_result = mp_sqr(a,b))) {
        rb_raise(eLT_M_Error, "Failure squaring Bignum : %s\n",
            mp_error_to_string(mp_result));
    }
    return result;
}


/*
 * call-seq:
 *  a.modulo_2d(d)
 *
 * returns a mod (2**d)
 */
static VALUE ltm_bignum_modulo_2d(VALUE self, VALUE other)
{
    mp_int *a       = MP_INT(self);
    int power_of_2  = NUM2INT(other); 
    VALUE result    = ALLOC_LTM_BIGNUM;
    mp_int *b       = MP_INT(result);
    int mp_result ;

    if (MP_OKAY != (mp_result = mp_mod_2d(a,power_of_2,b))) {
        rb_raise(eLT_M_Error, "Failure modulo_2d of Bignum: %s\n",
            mp_error_to_string(mp_result));
    }
    return result;
}


/*
 * call-seq:
 *  LibTom::Math.rand_of_size(n)
 *
 *  returns a pseudo random Bignum of at least N bits.
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
 *  a.add_modulus(b,c) -> Bignum
 *
 *  returns (a + b) mod c
 */
static VALUE ltm_bignum_add_modulus(VALUE self, VALUE p1, VALUE p2)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);
    mp_int *c    = NUM2MP_INT(p2);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *d    = MP_INT(result);
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_addmod(a,b,c,d))) {
        rb_raise(eLT_M_Error, "Failure calculating add_modulos: %s\n",
            mp_error_to_string(mp_result));
    }
    return result;
}

/*
 * call-seq:
 *  a.subtract_modulus(b,c) -> Bignum
 *
 *  returns (a - b) mod c
 */
static VALUE ltm_bignum_subtract_modulus(VALUE self, VALUE p1, VALUE p2)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);
    mp_int *c    = NUM2MP_INT(p2);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *d    = MP_INT(result);
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_submod(a,b,c,d))) {
        rb_raise(eLT_M_Error, "Failure calculating subtract_modulus: %s\n",
            mp_error_to_string(mp_result));
    }
    return result;
}


/*
 * call-seq:
 *  a.multiply_modulus(b,c) -> Bignum
 *
 *  returns (a * b) mod c
 */
static VALUE ltm_bignum_multiply_modulus(VALUE self, VALUE p1, VALUE p2)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);
    mp_int *c    = NUM2MP_INT(p2);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *d    = MP_INT(result);
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_mulmod(a,b,c,d))) {
        rb_raise(eLT_M_Error, "Failure calculating multiply_modulus: %s\n",
            mp_error_to_string(mp_result));
    }
    return result;
}


/*
 * call-seq:
 *  a.square_modulus(b,c) -> Bignum
 *
 *  returns (a * a) mod c
 */
static VALUE ltm_bignum_square_modulus(VALUE self, VALUE p1)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_sqrmod(a,b,c))) {
        rb_raise(eLT_M_Error, "Failure calculating square_modulus: %s\n",
            mp_error_to_string(mp_result));
    }
    return result;
}


/*
 * call-seq:
 *  a.inverse_modulus(b,c) -> Bignum
 *
 *  returns (1/a) mod c
 */
static VALUE ltm_bignum_inverse_modulus(VALUE self, VALUE p1)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_invmod(a,b,c))) {
        rb_raise(eLT_M_Error, "Failure calculating inverse_modulus: %s\n",
            mp_error_to_string(mp_result));
    }
    return result;
}


/*
 * call-seq:
 *  a.exponent_modulus(b,c) -> Bignum
 *
 *  returns (a ** b) mod c
 */
static VALUE ltm_bignum_exponent_modulus(VALUE self, VALUE p1, VALUE p2)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);
    mp_int *c    = NUM2MP_INT(p2);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *d    = MP_INT(result);
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_exptmod(a,b,c,d))) {
        rb_raise(eLT_M_Error, "Failure calculating exponent_modulus: %s\n",
            mp_error_to_string(mp_result));
    }
    return result;
}


/*
 * call-seq:
 *  a.extended_euclidian(b) -> Bignum
 *
 *  returns an array of 3 Bignums [u1, u2, u3] where
 *
 *      u1*a + u2*b = u3
 */
static VALUE ltm_bignum_extended_euclidian(VALUE self, VALUE p1)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);

    VALUE u1     = ALLOC_LTM_BIGNUM;
    VALUE u2     = ALLOC_LTM_BIGNUM;
    VALUE u3     = ALLOC_LTM_BIGNUM;

    mp_int *up1  = MP_INT(u1);
    mp_int *up2  = MP_INT(u2);
    mp_int *up3  = MP_INT(u3);

    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_exteuclid(a,b,up1,up2,up3))) {
        rb_raise(eLT_M_Error, "Failure calculating extended euclidian: %s\n",
            mp_error_to_string(mp_result));
    }

    return rb_ary_new3(3,u1,u2,u3);
}


/*
 * call-seq:
 *  a.gcd(b) -> Bignum
 *
 *  returns The greatest common divisor of the two numbers
 */
static VALUE ltm_bignum_greatest_common_divisor(VALUE self, VALUE p1)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_gcd(a,b,c))) {
        rb_raise(eLT_M_Error, "Failure calculating greatest common divisor: %s\n",
            mp_error_to_string(mp_result));
    }
    return result;
}


/*
 * call-seq:
 *  a.lcm(b) -> Bignum
 *
 *  returns The least common multiple of the two numbers
 */
static VALUE ltm_bignum_least_common_multiple(VALUE self, VALUE p1)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_lcm(a,b,c))) {
        rb_raise(eLT_M_Error, "Failure calculating least common multiple: %s\n",
            mp_error_to_string(mp_result));
    }
    return result;
}


/*
 * call-seq:
 *  a.jacobi(p) -> Integer
 *
 *  returns The Jacob symbol for a with respect to p.
 *
 *      -1 : a is not a quadratic residue modulo p
 *       0 : a divides p
 *       1 : a is a quadratic residue modulo p
 */
static VALUE ltm_bignum_jacobi(VALUE self, VALUE p)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p);
    int j_result;
    int mp_result;
    
    if (MP_OKAY != (mp_result = mp_jacobi(a,b,&j_result))) {
        rb_raise(eLT_M_Error, "Failure calculating jacobi symbol: %s\n",
            mp_error_to_string(mp_result));
    }
    return INT2FIX(j_result);
}


/*
 * call-seq:
 *  a.nth_root(b) -> Bignum
 *
 *  returns The N'th root of a.  b cannot be < 0 and even.  If no
 *  perfect integer root is found it returns c such that |c|**b <= |a|
 *
 *  Not very efficient for b > 3 on numbers with more than 1000 bits.
 */
static VALUE ltm_bignum_nth_root(VALUE self, VALUE p1)
{
    mp_int *a    = MP_INT(self);
    int b        = NUM2INT(p1);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *c    = MP_INT(result);
    int mp_result;
  
    /* if b is 2 then call the quicker square root function */
    if (2 == b) {
        if (MP_OKAY != (mp_result = mp_sqrt(a,c))) {
            rb_raise(eLT_M_Error, "Failure calculating nth_root(%d): %s\n",
                b,mp_error_to_string(mp_result));
        }
    } else {
        if (MP_OKAY != (mp_result = mp_n_root(a,(mp_digit)b,c))) {
            rb_raise(eLT_M_Error, "Failure calculating nth_root(%d): %s\n",
                b,mp_error_to_string(mp_result));
        }
    }
    return result;
}


/*
 * call-seq:
 *  a.square_root -> Bignum
 *
 *  returns b such that |b|**2 <= |a|
 */
static VALUE ltm_bignum_square_root(VALUE self)
{
    mp_int *a    = MP_INT(self);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *b    = MP_INT(result);
    int mp_result;
  
    if (MP_OKAY != (mp_result = mp_sqrt(a,b))) {
        rb_raise(eLT_M_Error, "Failure calculating square root : %s\n",
            mp_error_to_string(mp_result));
    }

    return result;
}


/*
 * call-seq:
 *  a.is_square? -> Bignum
 *
 *  returns true if a is a square and false otherwise
 */
static VALUE ltm_bignum_is_square(VALUE self)
{
    mp_int *a    = MP_INT(self);
    int is_square;
    int mp_result;
  
    if (MP_OKAY != (mp_result = mp_is_square(a,&is_square))) {
        rb_raise(eLT_M_Error, "Failure calculating is_square: %s\n",
            mp_error_to_string(mp_result));
    }

    return (is_square == 0) ? Qfalse : Qtrue; 
}


/**********************************************************************
 *                   Ruby Object life-cycle methods                   *
 **********************************************************************/

/*
 * garbage collector free method for mp_int structures
 */
static void ltm_bignum_free(mp_int *bn)
{
    mp_clear(bn);
    free(bn);
    return ;
}


/*
 * Allocator for mp_int structures
 */
static VALUE ltm_bignum_alloc(VALUE klass)
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

/**
 * call-seq:
 *      Bignum.new(initial,radix) -> bignum
 *
 * Create a new Bignum
 *
 *  initial:: As a string it is the string representation of the Bignum
 *               and radix is the base
 *            As a number of some type it is converted to a Bignum
 *
 */
static VALUE ltm_bignum_initialize(int argc, VALUE *argv, VALUE self)
{
    mp_int *bn;
    VALUE arg;
    VALUE arg2;
    VALUE arg_tmp;
    unsigned long from_val = 0L;
    long signed_val = 0;
    int radix = 10;
    int mp_result = 0;

    /* require at least one argument */
    switch (argc) {
    case 0:
        rb_raise(rb_eArgError,"at least one argument is required");
        break;
    case 1:
        arg = argv[0];
        break;
    default:
        arg = argv[0];
        radix = NUM2INT(argv[1]);
        break;
    }

    /* first pass, everything is converted to a ::bignum or an integer
     * */
    switch (TYPE(arg)) {
    case T_FIXNUM:
    case T_STRING:
        /* fixnums and strings are okay, they fall through to the 2nd pass */
        arg2 = arg;
        break;
    case T_FLOAT:
        /* Just call .to_i on the float and then to_s if it was convertd
         * to a ::Bignum
         */
        arg_tmp = rb_funcall(arg,rb_intern("to_i"),0);
        if (TYPE(arg_tmp) == T_BIGNUM) {
            arg2 = rb_funcall(arg_tmp,rb_intern("to_s"),0);
        } else {
            arg2 = arg_tmp;
        }
        break;

    case T_BIGNUM:
        /* bignums get converted to strings which we will read in on the
         * 2nd pass
         */
        arg2 = rb_funcall(arg,rb_intern("to_s"),0);
        break;
    default:
        rb_raise(rb_eArgError, "Unable to create Bignum from %s", StringValuePtr(arg));
        break;

    }

    /* second pass, everything is either a T_STRING or a T_FIXNUM */
    Data_Get_Struct(self,mp_int,bn);
    switch (TYPE(arg2)) {
    case T_FIXNUM:
        /* if arg2 is Fixnum then we convert to a ulong and then set the sign
         * as appropriate
         */
        signed_val = NUM2LONG(arg2);
        from_val   = (signed_val < 0) ? (-signed_val) : (signed_val);
        if (MP_OKAY != (mp_result = mp_init_set_int(bn,(unsigned long)from_val))) {
            rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
        }
        if (signed_val < 0) {
            mp_neg(bn,bn);
        }
        break;
    case T_STRING:
        /* if arg is a string then assume that it is a number and
         * convert it as such
         */
        if (MP_OKAY != (mp_result = mp_read_radix(bn,RSTRING(arg2)->ptr,radix))) {
            rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
        }
        break;
    default:
        rb_raise(rb_eArgError, "Unable to create Bignum from %s", StringValuePtr(arg2));
        break;
    }
    return self;
}


/**
 * Copy initializer used by dup and clone
 */
static VALUE ltm_bignum_initialize_copy(VALUE copy, VALUE orig)
{
    mp_int *m_orig;
    mp_int *m_copy;
    int mp_result;

    if (copy == orig) {
        return copy;
    }
    
    m_orig = MP_INT(orig);
    m_copy = MP_INT(copy);

    /* copy the internals using the mp_init_copy method to copy a bignum */
    if (MP_OKAY != (mp_result = mp_init_copy(m_copy,m_orig))) {
        rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
    }

    return copy;
}

/**********************************************************************
 *                   Ruby extension initialization                    *
 **********************************************************************/
void Init_libtommath()
{
    /* module definitions */
    mLT = rb_define_module("LibTom");
    mLT_M = rb_define_module_under(mLT,"Math");
    
    /* 2**d */
    rb_define_module_function(mLT_M,"pow2",ltm_two_to_the,1);
    rb_define_module_function(mLT_M,"two_to_the",ltm_two_to_the,1);
    rb_define_module_function(mLT_M,"rand_of_size",ltm_rand_of_size,1);

    /* exception definition */
    eLT_M_Error = rb_define_class_under(mLT_M,"LTMathError",rb_eStandardError);

    /* class Bignum definition, same as :;Bignum  the builtin class */
    cLT_M_Bignum = rb_define_class_under(mLT_M,"Bignum",rb_cNumeric);
    rb_define_alloc_func(cLT_M_Bignum,ltm_bignum_alloc);
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
    rb_define_method(cLT_M_Bignum, "-", ltm_bignum_subtract, 1);
    rb_define_method(cLT_M_Bignum, "*", ltm_bignum_multiply, 1);
    rb_define_method(cLT_M_Bignum, "/", ltm_bignum_divide, 1);
    rb_define_method(cLT_M_Bignum, "quo", ltm_bignum_divide, 1);
    rb_define_method(cLT_M_Bignum, "div", ltm_bignum_divide, 1);
    rb_define_method(cLT_M_Bignum, "remainder",ltm_bignum_remainder, 1);
    rb_define_method(cLT_M_Bignum, "%", ltm_bignum_modulo, 1);
    rb_define_method(cLT_M_Bignum, "modulo",ltm_bignum_modulo, 1);
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
    rb_define_method(cLT_M_Bignum,"divide_by_x_pow_b",ltm_bignum_right_shift_digits,1);
    rb_define_method(cLT_M_Bignum,"left_shift_digits",ltm_bignum_left_shift_digits,1);
    rb_define_method(cLT_M_Bignum,"multiply_by_x_pow_b",ltm_bignum_right_shift_digits,1);
    rb_define_method(cLT_M_Bignum,"squared",ltm_bignum_squared,0);
    rb_define_method(cLT_M_Bignum,"modulo_pow2",ltm_bignum_modulo_2d,1);
    rb_define_method(cLT_M_Bignum,"modulo_2d",ltm_bignum_modulo_2d,1);
    rb_define_method(cLT_M_Bignum,"mod_pow2",ltm_bignum_modulo_2d,1);
    rb_define_method(cLT_M_Bignum,"mod_2d",ltm_bignum_modulo_2d,1);

    rb_define_method(cLT_M_Bignum,"add_modulus",ltm_bignum_add_modulus,2);
    rb_define_method(cLT_M_Bignum,"subtract_modulus",ltm_bignum_subtract_modulus,2);
    rb_define_method(cLT_M_Bignum,"multiply_modulus",ltm_bignum_multiply_modulus,2);
    rb_define_method(cLT_M_Bignum,"square_modulus",ltm_bignum_square_modulus,1);
    rb_define_method(cLT_M_Bignum,"exponent_modulus",ltm_bignum_exponent_modulus,2);
    rb_define_method(cLT_M_Bignum,"inverse_modulus",ltm_bignum_inverse_modulus,1);
    rb_define_method(cLT_M_Bignum,"greatest_common_divisor",ltm_bignum_greatest_common_divisor,1);
    rb_define_method(cLT_M_Bignum,"gcd",ltm_bignum_greatest_common_divisor,1);
    rb_define_method(cLT_M_Bignum,"extended_euclidian",ltm_bignum_extended_euclidian,1);
    rb_define_method(cLT_M_Bignum,"least_common_multiple",ltm_bignum_least_common_multiple,1);
    rb_define_method(cLT_M_Bignum,"lcm",ltm_bignum_least_common_multiple,1);
    rb_define_method(cLT_M_Bignum,"nth_root",ltm_bignum_nth_root,1);
    rb_define_method(cLT_M_Bignum,"square_root",ltm_bignum_square_root,0);
    rb_define_method(cLT_M_Bignum,"is_square?",ltm_bignum_is_square,0);
    rb_define_method(cLT_M_Bignum,"jacobi",ltm_bignum_jacobi,1);


    /* additional methods that are provided by libtommath */
    /* Prime number methods */
    /* prime is divisible
     * fermat test
     * miller-rabin test
     * is_prime?
     * next_prime
     * random_prime
     * generate_prime (mp_prime_random_ex)
     */

    /*
     * extended euclidian
     */
}
