require 'libtom/math/bignum'
require 'libtom/math/prime'

#
# = LibTom
# LibTom[http://libtom.org] Projects are public domain open source
# libraries written in portable C. The libraries supports a variety of
# cryptographic and algebraic primitives designed to enable developers
# and students to pursue the field of cryptography much more
# efficiently. Currently the projects consist of three prominent
# libraries (LibTomCrypt, LibTomMath and TomsFastMath) which form the
# bulk of the source contributions.
#
module LibTom

    #
    # == LibTom::Math
    #
    # Interface to the LibTomMath[http://libtom.org/?page=features&newsitems=5&whatfile=ltm]
    # free open source portable number theoretic multiple-precision integer library.
    #
    # == What does it do?
    # The library provides a vast array of highly optimized routines from various branches of number theory.
    # 
    # * Simple Algebraic
    #   * Addition
    #   * Subtraction
    #   * Multiplication
    #   * Squaring
    #   * Division 
    # * Digit Manipulation
    #   * Shift left/right whole digits (mult by 2b by moving digits)
    #   * Fast multiplication/division by 2 and 2k for k>1
    #   * Binary AND, OR and XOR gates 
    # * Modular Reductions
    #   * Barrett Reduction (fast for any p)
    #   * Montgomery Reduction (faster for any odd p)
    #   * DR Reduction (faster for any restricted p see manual)
    #   * 2k Reduction (fast reduction modulo 2p - k for k < MP_MASK and for k > MP_MASK)
    #   * The exptmod logic can use any of the five reduction algorithms when appropriate with a single function call. 
    # * Number Theoretic
    #   * Greatest Common Divisor
    #   * Least Common Multiple
    #   * Jacobi Symbol Computation (falls back to Legendre for prime moduli)
    #   * Multiplicative Inverse
    #   * Extended Euclidean Algorithm
    #   * Modular Exponentiation
    #   * Fermat and Miller-Rabin Primality Tests, utility function such as is_prime and next_prime 
    # * Miscellaneous
    #   * Root finding over Z
    #   * Pseudo-random integers
    #   * Signed and Unsigned comparisons 
    # * Optimizations
    #   * Fast Comba based Multiplier, Squaring and Montgomery routines.
    #   * Montgomery, Diminished Radix and Barrett based modular exponentiation.
    #   * Karatsuba and Toom-Cook multiplication algorithms.
    #   * Many pointer aliasing optimiztions throughout the entire library. 
    #      
    module Math
    end
end
