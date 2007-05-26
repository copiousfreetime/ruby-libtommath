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
    int mp_result = MP_OKAY;

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
    int mp_result = MP_OKAY;

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
    mp_int *a = MP_INT(self);
    mp_int *b;
    mp_int *c;

    VALUE result = ALLOC_LTM_BIGNUM;
    int mp_result = MP_OKAY;

    if (IS_LTM_BIGNUM(other)) {
        b = MP_INT(other);
    } else {
        b = MP_INT(NEW_LTM_BIGNUM_FROM(other));
    }

    c = MP_INT(result);

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
    mp_int *a = MP_INT(self);
    mp_int *b;
    mp_int *c;

    VALUE result = ALLOC_LTM_BIGNUM;
    int mp_result = MP_OKAY;

    if (IS_LTM_BIGNUM(other)) {
        b = MP_INT(other);
    } else {
        b = MP_INT(NEW_LTM_BIGNUM_FROM(other));
    }

    c = MP_INT(result);

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
    mp_int *a; 
    mp_int *b;  
    VALUE param;

    a = MP_INT(self);

    if (IS_LTM_BIGNUM(other)) {
        param = other;
    } else {
        param = NEW_LTM_BIGNUM_FROM(other);
    }

    b = MP_INT(param);

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
    mp_int *a;
    mp_int *b;

    a = MP_INT(self);
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
 *      to_s(radix = 10) -> String
 *
 * convert the given Bignum to a string with base(radix). Radix can be a
 * value in the range 2..64
 */
static VALUE ltm_bignum_to_s(int argc, VALUE *argv, VALUE self)
{
    mp_int *bn;
    VALUE result;
    int radix = 10;
    int mp_result, mp_size = 0;
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
    bn = MP_INT(self);
    if (MP_OKAY != (mp_result = mp_radix_size(bn,radix,&mp_size))) {
        rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
    }

    mp_str = ALLOC_N(char,mp_size);
    if (MP_OKAY != (mp_result = mp_toradix(bn,mp_str,radix))) {
        rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
    }
    result = rb_str_new2(mp_str); 
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
    mp_int *a;
    mp_int *c;

    VALUE result = ALLOC_LTM_BIGNUM;
    int mp_result = MP_OKAY;
    int self_is_2, other_is_2;

    c = MP_INT(result);

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

        if (IS_LTM_BIGNUM(other)) {
            b = MP_INT(other);
        } else {
            b = MP_INT(NEW_LTM_BIGNUM_FROM(other));
        }

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
    mp_int *a = MP_INT(self);
    mp_int *c;

    VALUE result = ALLOC_LTM_BIGNUM;
    int mp_result = MP_OKAY;
    
    c = MP_INT(result);

    /* first find out if one of the values is a 2 or not.  Then we can use the fast
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

        if (IS_LTM_BIGNUM(other)) {
            b = MP_INT(other);
        } else {
            b = MP_INT(NEW_LTM_BIGNUM_FROM(other));
        }

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
    mp_int *a = MP_INT(self);
    mp_int *b;
    mp_int *c;

    VALUE result = ALLOC_LTM_BIGNUM;
    int mp_result = MP_OKAY;
    

    if (IS_LTM_BIGNUM(other)) {
        b = MP_INT(other);
    } else {
        b = MP_INT(NEW_LTM_BIGNUM_FROM(other));
    }

    c = MP_INT(result);
    if (MP_OKAY != (mp_result = mp_div(a,b,NULL,c))) {
        if (MP_VAL == mp_result) {
            rb_raise(rb_eZeroDivError,"divide by 0");
        }
        rb_raise(eLT_M_Error,"Failure to divide Bignums: %s", 
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
    mp_int *a = MP_INT(self);
    mp_int *b;
    mp_int *c;

    VALUE result = ALLOC_LTM_BIGNUM;
    int mp_result = MP_OKAY;
    
    c = MP_INT(result);

    if (IS_LTM_BIGNUM(other)) {
        b = MP_INT(other);
    } else {
        b = MP_INT(NEW_LTM_BIGNUM_FROM(other));
    }

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
    mp_int *b;

    VALUE div = ALLOC_LTM_BIGNUM;
    VALUE mod = ALLOC_LTM_BIGNUM;

    mp_int *m_div= MP_INT(div);
    mp_int *m_mod = MP_INT(mod);

    int mp_result = MP_OKAY;

    if (IS_LTM_BIGNUM(other)) {
        b = MP_INT(other);
    } else {
        b = MP_INT(NEW_LTM_BIGNUM_FROM(other));
    }
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
    mp_int *a = MP_INT(self);

    VALUE tmp_float;
    VALUE tmp_string;
    double f = HUGE_VAL; /* assume that self is > Float::MAX */

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

    tmp_float =  rb_funcall(tmp_string,rb_intern("to_f"),0);
    f = NUM2DBL(tmp_float);
    return rb_float_new(f);
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
    /*rb_define_method(cLT_M_Bignum, "**",ltm_bignum_pow, 1);
     * */
 
    /* utility methods */
    rb_define_method(cLT_M_Bignum, "size",ltm_bignum_size, 0);
    rb_define_method(cLT_M_Bignum, "to_f",ltm_bignum_to_f, 0);
    /*
       rb_define_method(cLT_M_Bignum, "<=>",        ltm_bignum_cmp, 1);
       rb_define_method(cLT_M_Bignum, "hash",       ltm_bignum_hash, 0);
       rb_define_mothod(cLT_M_Bignum, "nonzero?",   ltm_bignum_nonzero,0);
    */

    /* logical / bitwise  operators */
    /*
      rb_define_method(cLT_M_Bignum, "&",          ltm_bignum_and, 1);
       rb_define_method(cLT_M_Bignum, "|",          ltm_bignum_or, 1);
       rb_define_method(cLT_M_Bignum, "^",          ltm_bignum_xor, 1);
       rb_define_method(cLT_M_Bignum, "~",          ltm_bignum_neg, 0);
       rb_define_method(cLT_M_Bignum, "<<",         ltm_bignum_lshift, 1);
       rb_define_method(cLT_M_Bignum, ">>",         ltm_bignum_rshift, 1);
       rb_define_method(cLT_M_Bignum, "[]",         ltm_bignum_aref, 1);
     */


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
     * greatest common divisor
     * least common multiple
     * jacobi symbol
     * modular inverse
     */
}
