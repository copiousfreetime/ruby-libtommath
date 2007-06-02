module LibTom
    module Math

        #
        # Drop in replacement for the mathn Prime class. It adds in the
        # additional features of:
        #
        #   * picking a number to start with
        #   * Prime numbers are validated with Miller-Rabin trials
        #   * Allows for the iteration through only primes that are
        #     congruent to 3 mod 4.
        #
        # The 2nd option means that this Prime class may skip primes
        # that would iterated through by the mathn Prime class.
        # 
        class Prime

            include Enumerable
           
            attr_reader :starting_at
            attr_reader :current

            # 
            # Create a Prime number iterator.  Initialize it with a
            # number before the first prime to generate.  It defaults to
            # 1.
            #
            # The options are the same options that may be passed to
            # Bignum#next_prime.
            #
            def initialize(starting_at = 1, options = Hash.new)
                @starting_at = LibTom::Math::Bignum(starting_at)
                @options = options
                @current = nil
            end

            #
            # Generate the next prime greater than the 'current' number
            # in 
            def succ
                if @current then
                    @current = @current.next_prime(@options)
                else
                    @current = @starting_at.next_prime(@options)
                end
                return @current
            end

            alias next succ

            #
            # Iterator over prime numbers.  There is no upper bound.
            # This will infinitely loop.
            #
            def each
                loop do 
                    yield succ
                end
            end
        end
    end
end

