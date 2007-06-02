require 'libtom/ext/math'
module LibTom
    module Math
        #
        # In the vein of other Numeric conversions like Integer(),
        # Rational(), Complex(), Integer(), provide a similar call
        # structure.
        # 
        def Bignum(a) 
            LibTom::Math::Bignum.new(a) 
        end 
        module_function :Bignum
       
        #
        # Documentation and examples for Bignum should go here
        #
        class Bignum
            def num_miller_rabin_trials
                LibTom::Math::num_miller_rabin_trials(self.num_bits) 
            end 
        end
    end
end 
