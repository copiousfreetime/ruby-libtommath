#include "ruby.h"
#include <tommath.h>

/*** Prototypes ***/
static VALUE mLT;
static VALUE mLT_M;
static VALUE cLT_M_Bignum;
static VALUE eLT_M_Error;
static void ltm_bignum_free(mp_int *bn);
static VALUE ltm_bignum_alloc(VALUE klass);
static VALUE ltm_bignum_initialize(int argc, VALUE *argv, VALUE self);
static mp_int* value_to_mp_int(VALUE obj);
#define LTM_BIGNUM(obj) (value_to_mp_int(obj))
#define IS_LTM_BIGNUM(obj) (Qtrue == rb_obj_is_instance_of(obj,cLT_M_Bignum))

    
/*
 * Convert a VALUE into the internal struct we use for libtommmath
 */
static mp_int* value_to_mp_int(VALUE obj)
{
    mp_int* bn;

    if (IS_LTM_BIGNUM(obj)) {
        Data_Get_Struct(obj,mp_int,bn);
    } else {
        rb_raise(rb_eTypeError,"wrong type");
    }
    return bn;
}


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
            param = rb_class_new_instance(1,&other,cLT_M_Bignum);
            receiver = self;
            break;
            /* FIXME
        case T_FLOAT:
            param = other;
            receiver = ltm_bignum_to_float(LTM_BIGNUM(self));
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
 *  Bignum == obj  => true or false
 *
 * Returns true only if obj has the same value as Bignum.  This converts
 * non LTMBignum's to LTMBignums and does the comparison that way.
 */
static VALUE ltm_bignum_eq(VALUE self, VALUE other)
{
    mp_int *a; 
    mp_int *b;  
    VALUE param;

    a = LTM_BIGNUM(self);

    if (IS_LTM_BIGNUM(other)) {
        param = other;
    } else {
        param = rb_class_new_instance(1,&other,cLT_M_Bignum);
    }

    b = LTM_BIGNUM(param);

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

    a = LTM_BIGNUM(self);
    if (IS_LTM_BIGNUM(other)) {
        b = LTM_BIGNUM(other);
        if (MP_EQ == mp_cmp(a,b)) {
            return Qtrue;
        } 
    }
    return Qfalse;
}


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
    VALUE  obj = Data_Wrap_Struct(cLT_M_Bignum,NULL,ltm_bignum_free,bn);
    return obj;
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
    bn = LTM_BIGNUM(self);
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
    long from_val = 0L;
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

    Data_Get_Struct(self,mp_int,bn);
    if (MP_OKAY == (mp_result = mp_init(bn))) {
        switch (TYPE(arg)) {
        case T_FIXNUM:
        case T_FLOAT:
        /* if arg is Fixnum or a Real then we convert to a ulong and
         * then set the sign as appropriate
         */
            from_val = FIX2LONG(arg);
            if (MP_OKAY != (mp_result = mp_init_set_int(bn,(unsigned long)from_val))) {
                rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
            }
            if (from_val < 0) {
                mp_neg(bn,bn);
            }
            break;
        case T_BIGNUM:
            /** convert a bignum to a string and set arg = to the string
             * and fall through to the string conversion
             */
             arg = rb_funcall(arg,rb_intern("to_s"),0);
        case T_STRING:
            /* if arg is a string then assume that 
             * it is a number and convert it as such
             */
            if (MP_OKAY != (mp_result = mp_read_radix(bn,RSTRING(arg)->ptr,radix))) {
                rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
            }
            break;
        default:
            rb_raise(rb_eArgError, "Unable to create Bignum from %s", StringValuePtr(arg));
            break;
        }
        return self;
    } else {
        rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
    }
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
    
    m_orig = LTM_BIGNUM(orig);
    m_copy = LTM_BIGNUM(copy);

    /* copy the internals using the mp_init_copy method to copy a bignum */
    if (MP_OKAY != (mp_result = mp_init_copy(m_copy,m_orig))) {
        rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
    }

    return copy;
}

/*
 * Library initialization.  Called when require 'libtom/math' is parsed
 * in a ruby program.
 */
void Init_libtommath()
{
    /* module definitions */
    mLT = rb_define_module("LibTom");
    mLT_M = rb_define_module_under(mLT,"Math");

    /* exception definition */
    eLT_M_Error = rb_define_class_under(mLT_M,"LTMathError",rb_eStandardError);

    /* class Bignum definition, same as :;Bignum  the builtin class */
    cLT_M_Bignum = rb_define_class_under(mLT_M,"Bignum",rb_cObject);
    rb_define_alloc_func(cLT_M_Bignum,ltm_bignum_alloc);
    rb_define_method(cLT_M_Bignum,"initialize",ltm_bignum_initialize,-1);
    rb_define_method(cLT_M_Bignum,"initialize_copy",ltm_bignum_initialize_copy,1);
    rb_define_method(cLT_M_Bignum,"to_s",ltm_bignum_to_s, -1); 
    rb_define_method(cLT_M_Bignum,"coerce",ltm_bignum_coerce, 1);
    rb_define_method(cLT_M_Bignum, "==",ltm_bignum_eq, 1);
    rb_define_method(cLT_M_Bignum, "eql?",ltm_bignum_eql, 1);

    /** Ruby Built int BigNum operators **/
    /*
       rb_define_method(cLT_M_Bignum, "-@",         ltm_bignum_uminus, 0);
       rb_define_method(cLT_M_Bignum, "+",          ltm_bignum_plus, 1);
       rb_define_method(cLT_M_Bignum, "-",          ltm_bignum_minus, 1);
       rb_define_method(cLT_M_Bignum, "*",          ltm_bignum_mul, 1);
       rb_define_method(cLT_M_Bignum, "/",          ltm_bignum_div, 1);
       rb_define_method(cLT_M_Bignum, "%",          ltm_bignum_modulo, 1);
       rb_define_method(cLT_M_Bignum, "div",        ltm_bignum_div, 1);
       rb_define_method(cLT_M_Bignum, "divmod",     ltm_bignum_divmod, 1);
       rb_define_method(cLT_M_Bignum, "modulo",     ltm_bignum_modulo, 1);
       rb_define_method(cLT_M_Bignum, "remainder",  ltm_bignum_remainder, 1);
       rb_define_method(cLT_M_Bignum, "quo",        ltm_bignum_quo, 1);
       rb_define_method(cLT_M_Bignum, "**",         ltm_bignum_pow, 1);
       rb_define_method(cLT_M_Bignum, "&",          ltm_bignum_and, 1);
       rb_define_method(cLT_M_Bignum, "|",          ltm_bignum_or, 1);
       rb_define_method(cLT_M_Bignum, "^",          ltm_bignum_xor, 1);
       rb_define_method(cLT_M_Bignum, "~",          ltm_bignum_neg, 0);
       rb_define_method(cLT_M_Bignum, "<<",         ltm_bignum_lshift, 1);
       rb_define_method(cLT_M_Bignum, ">>",         ltm_bignum_rshift, 1);
       rb_define_method(cLT_M_Bignum, "[]",         ltm_bignum_aref, 1);
       rb_define_method(cLT_M_Bignum, "<=>",        ltm_bignum_cmp, 1);
       rb_define_method(cLT_M_Bignum, "hash",       ltm_bignum_hash, 0);
       rb_define_method(cLT_M_Bignum, "to_f",       ltm_bignum_to_f, 0);
       rb_define_method(cLT_M_Bignum, "abs",        ltm_bignum_abs, 0);
       rb_define_method(cLT_M_Bignum, "size",       ltm_bignum_size, 0);
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
