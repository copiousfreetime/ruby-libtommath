require 'libtommath'
module LibTom
    module Math
        #
        # In the vein of other Numeric conversions like Integer(),
        # Rational(), Complex(), Integer(), provide a similar call.
        # 
        def Bignum(a) 
            LibTom::Math::Bignum.new(a) 
        end 
        module_function :Bignum
       
        #
        # = What is LibTom::Math::Bignum?
        #
        # An almost drop in replacement for the built in ruby Bignum.
        # Overall the LibTomMath library is faster, can handler larger
        # numbers and has more features than the built in ruby Bignum.
        #
        # LibTom::Math::Bignum provides full support for all
        # arithmetic operations in the same manner as Ruby's Bignum.
        # In addition, it provides many number theory operations.
        #
        # == So what does it provide that Ruby's built in Bignum doesn't.
        #
        # It provides number theory operations such as primality tests, 
        # random number generation, the extended euclidian algorithm and 
        # additional modulus operations.
        #
        # == Hmm, it must be missing something then.
        # 
        # Yes, LibTom::Math::Bignum does not behave in the exact same
        # way that Ruby's Bignum behaves. These are the only ways in which
        # LibTom::Math::Bignum differes from Ruby's Bignum
        #
        # * Does not implement [] to provide "bit vector" style access.
        # * Does not implement ~ to provide bitwise negation.
        # * Forces all Numeric instances to LibTom::Math::Bignum when
        #   doing operations.  A Bignum + Float => bignum.
        # 
        # == That's all nice and good, but how about some examples?
        #
        # These are just a few, explore the API.
        #
        # === Generate big random numbers
        #
        #   big = LibTom::Math::Bignum::random_of_size(100_000)
        #   big.num_bits                              # => 1000000
        #   big.to_s.size                             # => 30108 (digits)
        #
        # === Test if a big number is prime
        #
        #   big.is_prime? 
        #
        # === Get the Greatest Common Divisor of 2 numbers
        #   
        #   a = LibTom::Math::Prime::random_of_size(56) # => 42732001016016959
        #   b = LibTom::Math::Prime::random_of_size(56) # => 63810767126601779
        #   c = LibTom::Math::Prime::random_of_size(56) # => 58312287814792003
        #
        #   ab = a * b
        #   bc = b * c
        #
        #   g = ab.greatest_common_divisor(bc)          # => 63810767126601779  
        #   g == b                                      # => true
        #
        class Bignum
            def num_miller_rabin_trials
                LibTom::Math::num_miller_rabin_trials(self.num_bits) 
            end 
        end
    end
end 
