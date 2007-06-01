#ifndef __LTM__
#define __LTM__

#include "ruby.h"
#include <math.h>
#include <tommath.h>


/* Module and Class */
extern VALUE mLT; 
extern VALUE mLT_M;
extern VALUE cLT_M_Bignum;
extern VALUE eLT_M_Error;

/**********************************************************************
 *                             Prototypes                             *
 **********************************************************************/

/* internal functions, not part of the API */
extern mp_int* value_to_mp_int(VALUE);
extern mp_int* num_to_mp_int(VALUE);

extern VALUE is_2(VALUE);
extern VALUE ltm_bignum_alloc(VALUE);
int ltm_bignum_random_prime_callback(unsigned char*, int, void*);

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

#define LTM_DEBUG 0
/**********************************************************************
 *                     Internally used functions                      *
 **********************************************************************/


#endif
