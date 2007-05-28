/* the infamous mp_int structure */
typedef struct  {
    int used, alloc, sign;
    mp_digit *dp;
} mp_int;

/* callback for mp_prime_random, should fill dst with random bytes and return how many read [upto len] */
typedef int ltm_prime_callback(unsigned char *dst, int len, void *dat);


/* ---> number theory <--- */

/* c = 1/a (mod b) */
int mp_invmod(mp_int *a, mp_int *b, mp_int *c);

/* c = (a, b) */
int mp_gcd(mp_int *a, mp_int *b, mp_int *c);

/* produces value such that U1*a + U2*b = U3 */
int mp_exteuclid(mp_int *a, mp_int *b, mp_int *U1, mp_int *U2, mp_int *U3);

/* c = [a, b] or (a*b)/(a, b) */
int mp_lcm(mp_int *a, mp_int *b, mp_int *c);

/* finds one of the b'th root of a, such that |c|**b <= |a|
 *
 * returns error if a < 0 and b is even
 */
int mp_n_root(mp_int *a, mp_digit b, mp_int *c);

/* special sqrt algo */
int mp_sqrt(mp_int *arg, mp_int *ret);

/* is number a square? */
int mp_is_square(mp_int *arg, int *ret);

/* computes the jacobi c = (a | n) (or Legendre if b is prime)  */
int mp_jacobi(mp_int *a, mp_int *n, int *c);

/* used to setup the Barrett reduction for a given modulus b */
int mp_reduce_setup(mp_int *a, mp_int *b);

/* Barrett Reduction, computes a (mod b) with a precomputed value c
 *
 * Assumes that 0 < a <= b*b, note if 0 > a > -(b*b) then you can merely
 * compute the reduction as -1 * mp_reduce(mp_abs(a)) [pseudo code].
 */
int mp_reduce(mp_int *a, mp_int *b, mp_int *c);

/* setups the montgomery reduction */
int mp_montgomery_setup(mp_int *a, mp_digit *mp);

/* computes a = B**n mod b without division or multiplication useful for
 * normalizing numbers in a Montgomery system.
 */
int mp_montgomery_calc_normalization(mp_int *a, mp_int *b);

/* computes x/R == x (mod N) via Montgomery Reduction */
int mp_montgomery_reduce(mp_int *a, mp_int *m, mp_digit mp);

/* returns 1 if a is a valid DR modulus */
int mp_dr_is_modulus(mp_int *a);

/* sets the value of "d" required for mp_dr_reduce */
void mp_dr_setup(mp_int *a, mp_digit *d);

/* reduces a modulo b using the Diminished Radix method */
int mp_dr_reduce(mp_int *a, mp_int *b, mp_digit mp);

/* returns true if a can be reduced with mp_reduce_2k */
int mp_reduce_is_2k(mp_int *a);

/* determines k value for 2k reduction */
int mp_reduce_2k_setup(mp_int *a, mp_digit *d);

/* reduces a modulo b where b is of the form 2**p - k [0 <= a] */
int mp_reduce_2k(mp_int *a, mp_int *n, mp_digit d);

/* returns true if a can be reduced with mp_reduce_2k_l */
int mp_reduce_is_2k_l(mp_int *a);

/* determines k value for 2k reduction */
int mp_reduce_2k_setup_l(mp_int *a, mp_int *d);

/* reduces a modulo b where b is of the form 2**p - k [0 <= a] */
int mp_reduce_2k_l(mp_int *a, mp_int *n, mp_int *d);

/* d = a**b (mod c) */
int mp_exptmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* performs one Fermat test of "a" using base "b".
 * Sets result to 0 if composite or 1 if probable prime
 */
int mp_prime_fermat(mp_int *a, mp_int *b, int *result);

/* performs one Miller-Rabin test of "a" using base "b".
 * Sets result to 0 if composite or 1 if probable prime
 */
int mp_prime_miller_rabin(mp_int *a, mp_int *b, int *result);

/* This gives [for a given bit size] the number of trials required
 * such that Miller-Rabin gives a prob of failure lower than 2^-96 
 */
int mp_prime_rabin_miller_trials(int size);

/* performs t rounds of Miller-Rabin on "a" using the first
 * t prime bases.  Also performs an initial sieve of trial
 * division.  Determines if "a" is prime with probability
 * of error no more than (1/4)**t.
 *
 * Sets result to 1 if probably prime, 0 otherwise
 */
int mp_prime_is_prime(mp_int *a, int t, int *result);

/* finds the next prime after the number "a" using "t" trials
 * of Miller-Rabin.
 *
 * bbs_style = 1 means the prime must be congruent to 3 mod 4
 */
int mp_prime_next_prime(mp_int *a, int t, int bbs_style);

/* makes a truly random prime of a given size (bytes),
 * call with bbs = 1 if you want it to be congruent to 3 mod 4 
 *
 * You have to supply a callback which fills in a buffer with random bytes.  "dat" is a parameter you can
 * have passed to the callback (e.g. a state or something).  This function doesn't use "dat" itself
 * so it can be NULL
 *
 * The prime generated will be larger than 2^(8*size).
 */
#define mp_prime_random(a, t, size, bbs, cb, dat) mp_prime_random_ex(a, t, ((size) * 8) + 1, (bbs==1)?LTM_PRIME_BBS:0, cb, dat)

/* makes a truly random prime of a given size (bits),
 *
 * Flags are as follows:
 * 
 *   LTM_PRIME_BBS      - make prime congruent to 3 mod 4
 *   LTM_PRIME_SAFE     - make sure (p-1)/2 is prime as well (implies LTM_PRIME_BBS)
 *   LTM_PRIME_2MSB_OFF - make the 2nd highest bit zero
 *   LTM_PRIME_2MSB_ON  - make the 2nd highest bit one
 *
 * You have to supply a callback which fills in a buffer with random bytes.  "dat" is a parameter you can
 * have passed to the callback (e.g. a state or something).  This function doesn't use "dat" itself
 * so it can be NULL
 *
 */
int mp_prime_random_ex(mp_int *a, int t, int size, int flags, ltm_prime_callback cb, void *dat);

