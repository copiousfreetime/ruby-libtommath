require 'libtom/ext/math'
module LibTom
    module Math
        class Bignum
            def num_miller_rabin_trials
                LibTom::Math::num_miller_rabin_trials(self.num_bits)
            end
        end
    end
end
