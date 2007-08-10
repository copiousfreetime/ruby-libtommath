#include "ltm.h"

/* Module and Class */
extern VALUE mLT;
extern VALUE mLT_M;
VALUE cLT_M_Bignum;
extern VALUE eLT_M_Error;
VALUE ltm_bignum_alloc(VALUE);

/**********************************************************************
 *                      Class Singleton Methods                       *
 **********************************************************************/
/*
 * call-seq:
 *  LibTom::Math::Bignum.random_of_size(n) -> bignum
 *
 * Generate a pseudo-random Bignum of at least _n_ bits and return it.
 */
VALUE ltm_bignum_random_of_size(VALUE self, VALUE other)
{
    VALUE result    = ALLOC_LTM_BIGNUM;
    mp_int *a       = MP_INT(result);
    double num_bits = NUM2DBL(other);

	/* Add an extra digit on here since on OSX it seems to make 1023 bit number out of this */
    int num_digits  = (int)ceil(num_bits / MP_DIGIT_BIT) + 1;
    int mp_result;

    if (MP_OKAY != (mp_result = mp_rand(a,num_digits))) {
        rb_raise(eLT_M_Error, "Failure calculating rand with %d bits: %s\n",
            num_bits,mp_error_to_string(mp_result));
    }   
    return result;
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
VALUE ltm_bignum_coerce(VALUE self, VALUE other)
{
    VALUE result = rb_ary_new2(2);
    VALUE tmp;
    VALUE param;
    VALUE receiver;

    switch (TYPE(other)) {
        case T_FIXNUM:
        case T_BIGNUM:
        case T_FLOAT:
            param = NEW_LTM_BIGNUM_FROM(other);
            receiver = self;
            break;
        default:
            /* maybe it is a Rational or something that descends from
             * Numeric.  If that is the case, try to convert it to a
             * bignu anyway
             */
            if (Qtrue == rb_obj_is_kind_of(other,rb_cNumeric)) {
                tmp = rb_funcall(other,rb_intern("to_i"),0);
                param = NEW_LTM_BIGNUM_FROM(tmp);
                receiver = self;
            } else {
                rb_raise(rb_eTypeError, "can't coerce %s to LibTom::Math::Bignum",
                        rb_obj_classname(other));
            }
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
VALUE ltm_bignum_uminus(VALUE self)
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
VALUE ltm_bignum_abs(VALUE self)
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
VALUE ltm_bignum_add(VALUE self, VALUE other)
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
VALUE ltm_bignum_subtract(VALUE self, VALUE other)
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
VALUE ltm_bignum_eq(VALUE self, VALUE other)
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
VALUE ltm_bignum_eql(VALUE self, VALUE other)
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
VALUE ltm_bignum_to_s(int argc, VALUE *argv, VALUE self)
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
VALUE ltm_bignum_multiply(VALUE self, VALUE other)
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
VALUE ltm_bignum_divide(VALUE self, VALUE other)
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
VALUE ltm_bignum_remainder(VALUE self, VALUE other)
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
VALUE ltm_bignum_modulo(VALUE self, VALUE other)
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
VALUE ltm_bignum_divmod(VALUE self, VALUE other)
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

VALUE ltm_bignum_size(VALUE self)
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
VALUE ltm_bignum_to_f(VALUE self)
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
VALUE ltm_bignum_spaceship(VALUE self, VALUE other)
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
VALUE ltm_bignum_nonzero(VALUE self)
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
VALUE ltm_bignum_zero(VALUE self)
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
VALUE ltm_bignum_zero_bang(VALUE self)
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
VALUE ltm_bignum_even(VALUE self)
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
VALUE ltm_bignum_odd(VALUE self)
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
VALUE ltm_bignum_hash(VALUE self)
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
VALUE ltm_bignum_pow(VALUE self, VALUE other)
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
VALUE ltm_bignum_bit_and(VALUE self, VALUE other)
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
VALUE ltm_bignum_bit_or(VALUE self, VALUE other)
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
VALUE ltm_bignum_bit_xor(VALUE self, VALUE other)
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
VALUE ltm_bignum_lshift_bits(VALUE self, VALUE other)
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
VALUE ltm_bignum_rshift_bits(VALUE self, VALUE other)
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
VALUE ltm_bignum_bit_negation(VALUE self)
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
VALUE ltm_bignum_bit_ref(VALUE self)
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
VALUE ltm_bignum_num_bits(VALUE self)
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
VALUE ltm_bignum_right_shift_digits(VALUE self, VALUE other)
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
VALUE ltm_bignum_left_shift_digits(VALUE self, VALUE other)
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
 *  bignum.squared -> bignum
 *
 * Calculates the square of _bignum_ and returns that result as a Bignum.
 */
VALUE ltm_bignum_squared(VALUE self)
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
VALUE ltm_bignum_modulo_2d(VALUE self, VALUE other)
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
 *  bignum.add_modulus(_numeric1_,_numeric2_) -> bignum
 *
 * Returns (_bignum_ + _numeric1_) mod _numeric2_
 */
VALUE ltm_bignum_add_modulus(VALUE self, VALUE p1, VALUE p2)
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
VALUE ltm_bignum_subtract_modulus(VALUE self, VALUE p1, VALUE p2)
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
VALUE ltm_bignum_multiply_modulus(VALUE self, VALUE p1, VALUE p2)
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
VALUE ltm_bignum_square_modulus(VALUE self, VALUE p1)
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

VALUE ltm_bignum_inverse_modulus(VALUE self, VALUE p1)
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
VALUE ltm_bignum_exponent_modulus(VALUE self, VALUE p1, VALUE p2)
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
VALUE ltm_bignum_extended_euclidian(VALUE self, VALUE p1)
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
VALUE ltm_bignum_greatest_common_divisor(VALUE self, VALUE p1)
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
VALUE ltm_bignum_least_common_multiple(VALUE self, VALUE p1)
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
VALUE ltm_bignum_jacobi(VALUE self, VALUE p)
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
VALUE ltm_bignum_nth_root(VALUE self, VALUE p1)
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

VALUE ltm_bignum_square_root(VALUE self)
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
VALUE ltm_bignum_is_square(VALUE self)
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
VALUE ltm_bignum_divisible_by_some_primes(VALUE self)
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
VALUE ltm_bignum_passes_fermat_primality(VALUE self,VALUE p1)
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
VALUE ltm_bignum_passes_miller_rabin(VALUE self,VALUE p1)
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
VALUE ltm_bignum_is_prime(int argc, VALUE* argv, VALUE self)
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
VALUE ltm_bignum_next_prime(int argc, VALUE* argv, VALUE self)
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
VALUE ltm_bignum_initialize(int argc, VALUE *argv, VALUE self)
{
    mp_int *bn;
    mp_int *orig;
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

    /* If we are not a Bignum then convert */
    if (!IS_LTM_BIGNUM(arg)) {

        /* first pass, everything is converted to a ::Bignum or an integer */
        switch (TYPE(arg)) {
        case T_FIXNUM:
        case T_STRING:
            /* fixnums and strings are okay, they fall through to the 2nd pass */
            arg2 = arg;
            break;
        case T_FLOAT:
            /* Just call .to_i on the float and then to_s if it was converted
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
            /* If it is a descendant of Numeric try and convert it */
            if (Qtrue == rb_obj_is_kind_of(arg,rb_cNumeric)) {
                arg2 = rb_funcall(arg,rb_intern("to_i"),0);
            } else {
                rb_raise(rb_eArgError, "Unable to create Bignum from %s", StringValuePtr(arg));
            }
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
    } else {
        /* we are initializing from a Bignum so copy */
        Data_Get_Struct(self,mp_int,bn);
        orig = MP_INT(arg);
        if (MP_OKAY != (mp_result = mp_init_copy(bn,orig))) {
            rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
        }
    }
    return self;
}


/* 
 * Internal method used internally in ruby for +dup+ and +clone+
 */
VALUE ltm_bignum_initialize_copy(VALUE copy, VALUE orig)
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

