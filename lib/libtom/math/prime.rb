module LibTom
    module Math
        class Prime

            include Enumerable

            def initializeNumericNumeric
                @number = Bignum.new(1)
            end

            def succ
                return @number = @number.next_prime
            end

            alias next succ
            def each
                loop do 
                    yield succ
                end
            end
        end
    end
end

