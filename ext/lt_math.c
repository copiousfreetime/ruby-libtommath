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

/**********************************************************************
 *                       Class Instance Methods                       *
 **********************************************************************/

/* call-seq:
 *  bignum.coerce(other) -> [bignum, bignum]
 *
 * Numeric coercion protocol.  Required to allow any 2 *Numeric* objects
 * to have arithmetic operations.
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
 *  -bignum -> bignum
 *
 * Unary minus. Returns a Bignum of the opposite sign.
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
 *  bignum.abs -> bignum
 *
 * Calculates the absolute value of _bignum_.
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
 *   bignum + numeric -> bignum
 *   bignum.add(numeric) -> bignum
 *
 * Calculates the arithmetic sum of _bignum_ and _numeric_, returning a Bignum.
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
 *   bignum - numeric -> bignum
 *   bignum.subtract(numeric) -> bignum
 *
 * Calculates the arithmetic difference between _bignum_ and _numeric_, returning a Bignum.
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
 *  bignum == numeric -> true, false
 *
 * Compares _bignum_ to _other_ and returns +true+ only if _bignum_ and
 * _numeric_ are numerically equivalent.  The comparison is done by converting 
 * _numeric_ to a Bignum and comparing.
 *
 *  Bignum.new(1234567890987654321) == 1234567890987654321      # => true
 *  Bignum.new(1234567890987654321) == 1234567890987654321.0    # => true
 *  Bignum.new(1234567890987654321) == 1234567890987654321.42   # => false
 *  Bignum.new(1234567890987654321) == "1234567890987654321"    # => false
 *
 */
static VALUE ltm_bignum_eq(VALUE self, VALUE other)
{
    mp_int *a = MP_INT(self);
    mp_int *b ;
    double d;
 
    /* we can only compare to other Numerics */
    if (Qfalse == rb_obj_is_kind_of(other,rb_cNumeric)) { 
        return Qfalse; 
    }

    /* test for fractional portion of a double.  If we have bits
     * after the decimal point, we will definitely not equal
     */
    if (TYPE(other) == T_FLOAT) {
        d = RFLOAT(other)->value;
        if (floor(d) != d) {
            return Qfalse;
        }
    }

    /* now we can compare */
    b = NUM2MP_INT(other);  
    return ((MP_EQ == mp_cmp(a,b)) ? Qtrue : Qfalse);
}


/*
 * call-seq:
 *  bignum.eql?(other) => true, false
 *
 * Compares _bignum_ to _other_ and returns +true+ only if _bignum_ and
 * _other_ are both Bignum instances and they are numerically equivalent.
 * 
 *  Bignum.new(1234567890987654321) == 1234567890987654321.0 # => false
 *
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
 *  bignum.to_s(radix = 10) -> string
 *
 * Convert _bignum_ to a string representation in base _radix_. Radix can be a
 * value in the range 2..64.
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
 *  bignum * numeric -> bignum 
 *  bignum.multiply(numeric) -> bignum
 *
 * Multiplies _bignum_ by _numeric_ and returns the resulting Bignum.
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
 *  bignum / numeric -> bignum
 *  bignum.quo(numeric) -> bignum
 *  bignum.div(numeric) -> bignum
 *  bignum.divide(numeric) -> bignum
 *
 * Divides _bignum_ by _numeric_ and returns the resulting Bignum.
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
 *   bignum.remainder(numeric) -> bignum
 *
 * Divides _bignum_ by _numeric_ and returns the remainder as a Bignum.
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
 *   bignum % numeric -> bignum
 *   bignum.mod(numeric) -> bignum
 *   bignum.modulo(numeric) -> bignum
 *
 * Takes _bignum_ modulo _numeric_ and returns the result as a Bignum.
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
 *   bignum.divmod(numeric) => [ quotient, modulus ]
 *
 * Divides _bignum_ by _numeric_ and returns an array containing the
 * +quotient+ and the +modulus+.
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
 *  bignum.size -> int
 *
 * Calculates the number of bytes in the machine representation of
 * _bignum_ and returns that as an *Integer*.  The result is not a word
 * boundary multiple and includes a byte to contain the sign.
 */

static VALUE ltm_bignum_size(VALUE self)
{
    mp_int *a = MP_INT(self);
    int size = mp_signed_bin_size(a);
    return INT2FIX(size);
}


/*
 * call-seq:
 *  bignum.to_f -> float
 *
 * Returns the floating point value of _bignum_.  If the result would be
 * larger than <b>Float::MAX</b>, *Infinity* is returned.
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
 * call-seq:
 *  bignum <=> numeric -> -1,0,1
 *
 * Compares _bignum_ to _numeric_ and returns <tt>-1,0,1</tt> if
 * _bignum_ is <tt>less than</tt>, <tt>equal to</tt>, or <tt>greater
 * than</tt> _numeric_.  This is the basis for the *Comparable* module.
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
 *  bignum.nonzero? -> true, false
 *
 * Returns true if _bignum_ is not zero, otherwise returns false.
 * nonzero? retrusn the opposite of zero?
 *
 */
static VALUE ltm_bignum_nonzero(VALUE self)
{
    mp_int *a = MP_INT(self);

    return (mp_iszero(a) ? Qfalse : Qtrue );
}

/*
 * call-seq:
 *  bignum.zero? -> true, false
 *
 * Returns true if _bignum_ is zero, otherwise returns false.  zero?
 * returns the opposite of nonzero? 
 */
static VALUE ltm_bignum_zero(VALUE self)
{
    mp_int *a = MP_INT(self);

    return (mp_iszero(a) ? Qtrue : Qfalse);
}

/*
 * call-seq:
 *  bignum.zero! -> nil
 *
 * Destroys _bignum_ forcing it to the value +zero+.  After this call,
 * zero? will return true.
 */
static VALUE ltm_bignum_zero_bang(VALUE self)
{
    mp_int *a = MP_INT(self);
    
    mp_zero(a);

    return Qnil;
}


/*
 * call-seq:
 *  bignum.even? -> true, false
 *
 * Returns true if _bignum_ is an even number, otherwise false.
 */
static VALUE ltm_bignum_even(VALUE self)
{
    mp_int *a = MP_INT(self);

    return (mp_iseven(a) ? Qtrue : Qfalse);
}


/*
 * call-seq:
 *  bignum.odd? -> true, false
 *
 * Returns true if _bignum_ is an odd number, otherwise false.
 */
static VALUE ltm_bignum_odd(VALUE self)
{
    mp_int *a = MP_INT(self);

    return (mp_isodd(a) ? Qtrue : Qfalse);
}


/*
 * call-seq:
 *  bignum.hash -> fixnum
 *
 * Calculates a hash of _bignum_ and returns it.  This is normally
 * only used internally for Ruby.
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
 *  bignum**numeric -> bignum
 *
 * Raise _bignum_ to the _numeric_ power and return the resulting
 * Bignum.
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
 *  bignum & numeric -> bignum
 *
 * Bitwise *and* _bignum_ with _numeric_ and return the resulting
 * Bignum.  _numeric_ is converted to a Bignum in the process.
 * 
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
 *  bignum | numeric -> bignum
 *
 * Bitwise *or* _bignum_ with _numeric_ and return the resulting
 * Bignum.  _numeric_ is converted to a Bignum in the process.
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
 *  bignum ^ numeric -> bignum
 *
 * Bitwise *xor* _bignum_ with _numeric_ and return the resulting
 * Bignum.  _numeric_ is converted to a Bignum in the process.
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
 *  bignum << N -> bignum
 *
 * Left shift _bignum_ by _N_ bits and return the resulting Bignum.
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
 *  bignum >> N -> bignum
 *
 * Right shift _bignum_ by _N_ bits and return the resulting Bignum.
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
 *  ~bignum -> raises NotImplementedError
 *
 *  Bitwise negation is not implemented.
 */
static VALUE ltm_bignum_bit_negation(VALUE self)
{
    rb_raise(rb_eNotImpError,"Bitwise negation is not implemented on LibTom::Math::Bignum");
    return self;
}

/*
 * call-seq:
 *  bignum[i] -> raises NotImplementedError
 *
 * Using _bignum_ as a bit vector is not implemented.
 */
static VALUE ltm_bignum_bit_ref(VALUE self)
{
    rb_raise(rb_eNotImpError,"Bit Reference access is not implemented on LibTom::Math::Bignum");
    return self;
}


/*
 * call-seq:
 *  bignum.num_bits -> int
 *
 * Return the number of bits used to represent _bignum_.
 */
static VALUE ltm_bignum_num_bits(VALUE self)
{
    mp_int *a = MP_INT(self);
    return INT2FIX(mp_count_bits(a));
}


/*
 * call-seq:
 *  bignum.right_shift_digits(n) -> bignum
 *  bignum.divide_by_x_pow_n(n) -> bignum
 *
 * Divide _bignum_ by the polynomial <tt>x**n</tt>.
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
 *  bignum.left_shift_digits(n) -> bignum
 *  bignum.multiply_by_x_pow_n(n) -> bignum
 *
 * Multiply _bignum_ by the polynomial <tt>x**n</tt>.
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
 *  bignum.squared -> bignum
 *
 * Calculates the square of _bignum_ and returns that result as a Bignum.
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
 *  bignum.mod_2n(n) -> bignum
 *  bignum.mod_pow2(n) -> bignum
 *  bignum.modulo_2n(n) -> bignum
 *  bignum.modulo_pow2(n) -> bignum
 *
 * Calculates _bignum_ <tt>mod (2**n)</tt> and returns the result as a Bignum.
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
 *  bignum.add_modulus(_numeric1_,_numeric2_) -> bignum
 *
 * Returns (_bignum_ + _numeric1_) mod _numeric2_
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
 *  bignum.subtract_modulus(numeric1,numeric2) -> bignum
 *
 * Returns (_bignum_ - _numeric_) mod _numeric_
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
 *  bignum.multiply_modulus(numeric1,numeric2) -> bignum
 *
 * Returns (_bignum_ * _numeric1_) mod _numeric2_
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
 *  bignum.square_modulus(numeric) -> bignum
 *
 * Returns (_bignum_ * _bignum_) mod _numeric_ 
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
 *  bignum.inverse_modulus(numeric) -> bignum
 *
 * Calculates and returns the multiplicative inverse of _bignum_ modulo
 * _numeric_.
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
 *  bignum.exponent_modulus(numeric1,numeric2) -> bignum
 *
 * Returns (_bignum_ ** _numeric1_) mod _numeric2_
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
 *  bignum.extended_euclidian(numeric) -> [bignum, bignum, bignum]
 *
 * Finds <tt>[u1, u2, u3]</tt> using the Extended Euclidian Algorithm
 * such that:
 *
 *  u1*bignum + u2*numeric = u3
 *
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
 *  bignum.gcd(numeric) -> bignum
 *  bignum.greatest_common_divisor(numeric) -> bignum
 *
 * Finds the greatest common divisor between _bignum_ and _numeric_.
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
 *  bignum.lcm(numeric) -> bignum
 *  bignum.least_common_multiple(numeric) -> bignum
 *
 * Finds the least common multiple of _bignum_ and _numeric_.
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
 *  bignum.jacobi(numeric) -> int
 *
 * Computes the Jacob symbol for _bignum_ with respect to _numeric_.
 * The result returned has different meanings:
 *
 * <b><tt>-1</tt></b>::         _bignum_ is not a quadratic residue modulo _numeric_
 * <b><tt>0</tt></b>::          _bignum_ divides _numeric_
 * <b><tt>1</tt></b>::          _bignum_ is a quadratic residue module _numeric_
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
 *  bignum.nth_root(numeric) -> bignum
 *
 * Calculates and returns the _numeric_ root of _bignum_.  _numeric_
 * cannot be < 0 and even.  If no perfect integer root is found it
 * returns a Bignum _r_ such that <tt>|r|**numeric <= |bignum|</tt>.
 *
 * This method is not efficient for _numeric_ > 3 on numbers 
 * with more than 1000 bits.
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
 *  bignum.square_root -> bignum
 *
 * Calculates and returns the square root of _bignum_.  If no perfect
 * integer root is found it returns a Bignum _r_ such that 
 * <tt>|r|**2 <= |bignum|</tt>.
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
 *  bignum.is_square? -> true, false
 *
 * Determins if _bignum_ is a square, that is, it has an integer root,
 * and returns +true+ or +false+ as appropriate.
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


/*
 * call-seq:
 *  bignum.divisible_by_some_primes? -> true, false
 *
 * Attempts to divide _bignum_ by a "list of known prime numbers".  If
 * _bignum_ is divisible by 1 or more of them, this method returns
 * +true+.  If not, it returns +false+.
 *
 * Currently the "list of known prime numbers" is the first 256 primes.
 *
 */
static VALUE ltm_bignum_divisible_by_some_primes(VALUE self)
{
    mp_int *a    = MP_INT(self);
    int is_prime;
    int mp_result;
  
    if (MP_OKAY != (mp_result = mp_prime_is_divisible(a,&is_prime))) {
        rb_raise(eLT_M_Error, "Failure calculating if num is divisible by some primes: %s\n",
            mp_error_to_string(mp_result));
    }

    return (is_prime == 0) ? Qfalse : Qtrue; 
}


/*
 * call-seq:
 *  bignum.passes_fermat_primality?(numeric) -> true, false
 *  
 * Does 1 run of the Fermat Primality Test.  It tests:
 *
 *      numeric**bignum mod bignum == numeric
 *
 * returning +true+ or +false+ based upon the result.  A return value of
 * +true+ does not mean that _bignum_ is prime.  It means that _bignum_
 * is "probably" prime.
 *  
 */
static VALUE ltm_bignum_passes_fermat_primality(VALUE self,VALUE p1)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);
    
    int passed;
    int mp_result;
  
    if (MP_OKAY != (mp_result = mp_prime_fermat(a,b,&passed))) {
        rb_raise(eLT_M_Error, "Failure running fermat primality test %s\n",
            mp_error_to_string(mp_result));
    }

    return (passed == 0) ? Qfalse : Qtrue; 
}


/*
 * call-seq:
 *  bignum.passes_miller_rabin?(numeric) -> true, false
 *  
 * Does 1 iteration of the Miller-Rabin Test for primality on _bignum_
 * using base _numeric_. It returns +true+ or +false+ based upon the
 * result.  Generally many calls to this method with different _numeric_
 * values are required to have some level of assurance that _bignum_ is
 * a prime number.
 *
 * The number of iterations of the Miller-Rabin test to perform on
 * _bignum_ may be calculated with num_miller_rabin_trials(_bignum_.num_bits).
 */
static VALUE ltm_bignum_passes_miller_rabin(VALUE self,VALUE p1)
{
    mp_int *a    = MP_INT(self);
    mp_int *b    = NUM2MP_INT(p1);
    
    int passed;
    int mp_result;
  
    if (MP_OKAY != (mp_result = mp_prime_fermat(a,b,&passed))) {
        rb_raise(eLT_M_Error, "Failure running Miller-Rabin test %s\n",
            mp_error_to_string(mp_result));
    }

    return (passed == 0) ? Qfalse : Qtrue; 
}


/*
 * call-seq:
 *  bignum.is_prime?( trials = default ) -> true, false
 *  
 * Tests to see if _bignum_ is prime by doing a trial division
 * (is_divisible_by_some_primes?) followed by _trials_ rounds of
 * Miller-Rabin tests.  By default, _trials_ is the value returned from
 * num_miller_rabin_trails(bignum.num_bits).
 */
static VALUE ltm_bignum_is_prime(int argc, VALUE* argv, VALUE self)
{
    mp_int *a    = MP_INT(self);
    int t;       
    int num_bits;
    int passed;
    int mp_result;

    if (argc == 0) {
        num_bits = mp_count_bits(a);
        t = mp_prime_rabin_miller_trials(num_bits);
    } else {
        t = FIX2INT(argv[0]);
    }

    /* make sure that t is within a good range.  This is also done in
     * the mp_prime_is_prime method, but that error message isn't
     * helpful
     */
    if (t > PRIME_SIZE) {
        rb_raise(rb_eArgError,"Number of Miller-Rabin trials must be > 0 and < %d\n",PRIME_SIZE);
    }

    if (MP_OKAY != (mp_result = mp_prime_is_prime(a,t,&passed))) {
        rb_raise(eLT_M_Error, "Failure testing for primality : %s\n",
            mp_error_to_string(mp_result));
    }
  
    return (passed == 0) ? Qfalse : Qtrue; 
}


/*
 * call-seq:
 *  bignum.next_prime(options => Hash.new) -> bignum
 *
 * Calculates and returns the next prime number greater than _bignum_.
 * The _options_ can be:
 *
 * <b><tt>:trials</tt></b>::        The number of Miller-Rabin trails the new
 *                                  prime must pass.  The default is the same
 *                                  as that used in is_prime?.
 *
 *
 * <b><tt>:congruency</tt></b>::    +true+ or +false+.  Should the prime
 *                                  returned be congruent to 3 mod 4.
 *                                  The default is +false+.
 */
static VALUE ltm_bignum_next_prime(int argc, VALUE* argv, VALUE self)
{
    mp_int *a    = MP_INT(self);
    VALUE result = ALLOC_LTM_BIGNUM;
    mp_int *b    = MP_INT(result);

    VALUE options;
    int num_bits;
    int trials     = -1;
    int trials_option = 0;
    int congruency = 0;
    int mp_result;
    VALUE value;

    /* parse the options */
    if (argc > 0) {
        options = argv[0] ;
        /* check for the :trials and :congruency options */
        if (rb_obj_is_kind_of(options,rb_cHash)) {
            value = rb_hash_aref(options,ID2SYM(rb_intern("trials")));
            if (Qnil != value) {
                trials = NUM2INT(value);
                trials_option = 1;
            }

            value = rb_hash_aref(options,ID2SYM(rb_intern("congruency")));
            if (Qnil != value) {
                congruency = (Qtrue == value) ? 1 : 0;
            }
        }
    }
    
    if (!trials_option && (trials <= 0)) {
        num_bits = mp_count_bits(a);
        trials = mp_prime_rabin_miller_trials(num_bits);
    }

    /* make sure that trials is within a good range.  This is also done in
     * the mp_prime_is_prime method, but that error message isn't
     * helpful
     */
    if (trials_option && (trials < 0 || trials > PRIME_SIZE)) {
        rb_raise(rb_eArgError,"Number of Miller-Rabin trials must be > 0 and < %d\n",PRIME_SIZE);
    }

    if (congruency != 0 && congruency != 1) {
        rb_raise(rb_eArgError,"Congruency must be true or false\n");
    }

    /* options checked, now find a prime, first we have to copy over a
     * since next_prime operates in place
     */
    if (MP_OKAY != (mp_result = mp_copy(a,b))) {
            rb_raise(eLT_M_Error, "Failure to find next prime: %s", 
                mp_error_to_string(mp_result));
    }

    if (MP_OKAY != (mp_result = mp_prime_next_prime(b,trials,congruency))) {
            rb_raise(eLT_M_Error, "Failure to find next prime: %s", 
                mp_error_to_string(mp_result));
    }

    return result;
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
 *  Bignum.new(initial,radix = 10) -> bignum
 *
 * Create a new Bignum.  
 *
 * * When _initial_ is a *String* then _radix_ is the base 
 *   from which to convert.  
 * * When _initial_ is a *Numeric* then _radix_ is ignored.
 *
 *  Bignum.new(42.42)           # => 42
 *  Bignum.new(42.0)            # => 42
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

    /* first pass, everything is converted to a ::Bignum or an integer
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


/* 
 * Internal method used internally in ruby for +dup+ and +clone+
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

/*
 * Interface to the LibTomMath[http://libtom.org/?page=features&newsitems=5&whatfile=ltm]
 * free open source portable number theoretic multiple-precision integer
 * library
 */
void Init_Bignum()
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

}