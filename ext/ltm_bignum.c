#include "ruby.h"
#include <tommath.h>

/*** Prototypes ***/
static VALUE mLT;
static VALUE mLT_M;
static VALUE cLT_M_Bignum;
static VALUE eLT_M_Error;
static void ltm_bignum_free(mp_int *bn);
static VALUE ltm_bignum_alloc(VALUE klass);
static VALUE ltm_bignum_initialize(int argc,VALUE *argv,VALUE self);


static void ltm_bignum_free(mp_int *bn)
{
    return mp_clear(bn);
}

static VALUE ltm_bignum_alloc(VALUE klass)
{
    mp_int *bn = ALLOC_N(mp_int,1);
    VALUE  obj = Data_Wrap_Struct(cLT_M_Bignum,NULL,ltm_bignum_free,bn);

    return obj;
}

/**
 * call-seq:
 *      Bignum.new -> bignum
 *
 * Creates a new Bignum
 */
static VALUE ltm_bignum_initialize(int argc, VALUE *argv, VALUE self)
{
    mp_int *bn;
    int mp_result;
   
    Data_Get_Struct(self,mp_int,bn);
    if (MP_OKAY != (mp_result = mp_init(bn))) {
        rb_raise(eLT_M_Error, "%s", mp_error_to_string(mp_result));
    } else {
        return self;
    }
}

void Init_libtommath()
{
    /* module definitions */
    mLT = rb_define_module("LibTom");
    mLT_M = rb_define_module_under(mLT,"Math");

    /* exception definition */
    eLT_M_Error = rb_define_class_under(mLT_M,"LTMathError",rb_eStandardError);

    /* class Bignum definition */
    cLT_M_Bignum = rb_define_class_under(mLT_M,"Bignum",rb_cObject);
    rb_define_alloc_func(cLT_M_Bignum,ltm_bignum_alloc);
    rb_define_method(cLT_M_Bignum,"initialize",  ltm_bignum_initialize,-1);

    /*
    rb_define_method(cLT_M_Bignum, "to_s",       ltm_bignum_to_s, -1); 
    rb_define_method(cLT_M_Bignum, "coerce",     ltm_bignum_coerce, 1);
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
    rb_define_method(cLT_M_Bignum, "==",         ltm_bignum_eq, 1);
    rb_define_method(cLT_M_Bignum, "eql?",       ltm_bignum_eql, 1);
    rb_define_method(cLT_M_Bignum, "hash",       ltm_bignum_hash, 0);
    rb_define_method(cLT_M_Bignum, "to_f",       ltm_bignum_to_f, 0);
    rb_define_method(cLT_M_Bignum, "abs",        ltm_bignum_abs, 0);
    rb_define_method(cLT_M_Bignum, "size",       ltm_bignum_size, 0);
    */
}
