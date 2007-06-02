module LibTom
    module Math

        #
        # Drop in replacement for the mathn Prime class. It adds in the
        # additional features of:
        # 
        # * Being MUCH MUCH faster
        # * Picking an initial starting point
        # * Set the number of Miller-Rabin trials for primality tests
        # * Only iterate through primes that are congruent to 3 mod 4.
        #
        # The last feature means that this Prime class may skip primes
        # that would be iterated through by the mathn Prime class.
        #
        # == The first 100 primes
        #   
        #   prime_list = []
        #   p = Prime.new
        #   100.times { prime_list << p.succ }
        # 
        # or 
        #
        #   p = Prime.new
        #   prime_list = Array.new(100) { p.succ }
        #
        # == Show me this speed improvement
        #
        # At the lower numbers it is not much faster, but once it starts
        # getting past the first 500 primes it screams in comparison
        #
        #     require 'libtom/math'
        #     require 'mathn'
        #     require 'benchmark'
        #
        #     n = ARGV.shift.to_i
        #
        #     mathn_prime = Prime.new
        #     tom_prime   = LibTom::Math::Prime.new
        #
        #     Benchmark.bm(25) do |x| 
        #       x.report("Mathn  #{n} primes") { n.times { mathn_prime.succ } }
        #       x.report("LibTom #{n} primes") { n.times { tom_prime.succ   } }
        #     end
        #
        # output
        #                                  user     system      total         real
        #   Mathn  10000 primes      105.280000  12.620000 117.900000 (125.627094)
        #   LibTom 10000 primes       21.020000   0.040000  21.060000 ( 21.082540)
        #
        # == Okay, now I want a REALLY BIG PRIME
        # 
        # Generate a random prime with 4096 bits that is congruent with
        # 3 mod 4 and given p, make sure that ((p-1)/2) is also prime.
        # This takes a while.
        #
        #   big = Prime::random_of_size(4096, { :congruency => true, :safe => true } )
        #   big.class                           # => LibTom::Math::Bignum
        #   big.num_bits                        # => 4096
        #   big.to_s.size                       # => 1233 (digits long)
        #
        # == Start with a 'small' prime and give me the next 10
        #
        #   small = Prime::random_of_size(512, { :congruency => true, :safe => true } )
        #   small.class                           # => LibTom::Math::Bignum
        #   small.num_bits                        # => 256
        #   small.to_s.size                       # => 78 (digits long)
        #   
        #   p = Prime.new(small)
        #   prime_list = Array.new(10) { p.succ } 
        #   prime_list.last - prime_list.first    # => 2786
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

