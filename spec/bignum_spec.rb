require 'libtom/math'

describe LibTom::Math::Bignum, "instantiations" do 
    it "should instantiate from a string" do 
        b = LibTom::Math::Bignum.new("12345"*10)
        b.class.should == LibTom::Math::Bignum
    end

    it "should instantiate from a Fixnum" do
        b = LibTom::Math::Bignum.new(12345)
        b.class.should == LibTom::Math::Bignum
    end

    it "should instantiate from a Real" do
        b = LibTom::Math::Bignum.new(12345.6) 
        b.class.should == LibTom::Math::Bignum
    end

    it "should instantiate from a ::Bignum" do 
        b = LibTom::Math::Bignum.new(1_234_567_890_098_765_432) 
        b.class.should == LibTom::Math::Bignum
    end

    it "should not instantiate from a Hash" do
        lambda do 
            b = LibTom::Math::Bignum.new({}) 
        end.should raise_error(TypeError)
    end

    it "should not instantiate without an argument" do
        lambda do
            b = LibTom::Math::Bignum.new
        end.should raise_error(ArgumentError)
    end

    it "should clone correctly" do
        b = LibTom::Math::Bignum.new(987654321123456789)
        c = b.dup
        c.object_id.should_not == b.object_id
    end
    it "should dup correctly"  do
        b = LibTom::Math::Bignum.new(987654321123456789)
        c = b.clone
        c.object_id.should_not == b.object_id
    end

end

describe LibTom::Math::Bignum, "conversions" do
    before(:each) do
        @bn = LibTom::Math::Bignum.new(12345654321)
    end

    bases = {
        2 => "1011011111110110111011110000110001",
        8 => "133766736061",
        16 => "2dfdbbc31".upcase,
        26 => "1dp1pc6d".upcase,
    }
    bases.each_pair do |base, value|
        it "should convert correctly to a string in base #{base}" do
            @bn.to_s(base).should == value
        end
    end

    it "should say 'RUBYRULES' in base 36" do
        b = LibTom::Math::Bignum.new("RUBYRULES",36)
        b.to_s(36).should == "RUBYRULES"
    end

    it "should say 'rubyrules' in base 64" do
        b = LibTom::Math::Bignum.new("rubyrules",64)
        b.to_s(64).should == "rubyrules"
    end

    it "should have Numeric as an ancestor" do
        @bn.class.ancestors.should include(Numeric)
    end

    it "should have Comparable as an ancestor" do
        @bn.class.ancestors.should include(Comparable)
    end

    it "should convert an Integer to Bignum correctly" do
        c = LibTom::Math::Bignum.new(42)
        c.should == 42
    end
    
    it "should convert a ::Bignum to Bignum correctly" do
        c = LibTom::Math::Bignum.new(9876543210987654321)
        c.should == 9876543210987654321
    end

    it "should convert a Float to Bignum correctly" do
        c = LibTom::Math::Bignum.new(42.42)
        c.should == 42
    end
end

describe LibTom::Math::Bignum, "object / class comparisons" do
    before(:each) do
        @big = LibTom::Math::Bignum.new(1234567890987654321)
        @small = 42
    end

    it "should eql? self" do
        @big.should.eql?(@big.dup)
    end

    it "should == self" do
        @big.should == @big.dup
    end

    it "should not eql? ::Bignum(1234567890987654321)" do
        @big.should_not.eql?(1234567890987654321)
    end

    it "should == ::Bignum(1234567890987654321)" do
        @big.should == 1234567890987654321
    end
    
    it "should not == ::Bignum(1234567890987654322)" do
        @big.should_not == 1234567890987654322
    end

    it "should == Integer(42)" do
        @small.should == 42
    end

    it "should not == Integer(63)" do
        @small.should_not == 63
    end

    it "should not eql? Integer(42)" do
        @small.should_not.eql?(42)
    end

    it "should == Float(42.0)" do
        @small.should == 42.0
    end

    it "should not eql? Float(42.0)" do
        @small.should_not.eql?(42.0)
    end

    it "should not == 42.42" do
        @small.should_not == 42.42
    end

    it "should not == '42'" do 
        @small.should_not == '42'
    end
end

describe LibTom::Math::Bignum, "arithmetic operations" do
    @other  = LibTom::Math::Bignum.new(1234567890987654321)
    before(:each) do
        @a = LibTom::Math::Bignum.new(1234567890987654321)
        @b = LibTom::Math::Bignum.new(9876543210123456789)
    end

    it "should negate correctly" do 
        c = -@a
        c.should == -1234567890987654321
    end

    it "should have a correct absolute value" do
        c = -@a
        c.abs.should == @a
    end

    answers = {
        # test has param1, param2, [result of param1 op param2, result of param2 op param1]
        # klass => Array of tests
        "+" => { Float => [ [@other, 42.42, [1234567890987654363,1234567890987654363]]],
                 Integer => [ [@other, 42,  [1234567890987654363,1234567890987654363]]],
                 Bignum => [ [@other,9876543210123456789, [11111111101111111110,11111111101111111110 ]]],
                 LibTom::Math::Bignum => [ [@other, LibTom::Math::Bignum.new(42424242424242424242),
                                            [43658810315230078563,43658810315230078563]]]
              },
        "-" => { Float => [ [@other,21.52,[1234567890987654300,-1234567890987654300]]],
                 Integer => [[@other,54321, [1234567890987600000,-1234567890987600000]]],
                 Bignum => [[@other,42424242424242424242, [-41189674533254769921,41189674533254769921]]],
                 LibTom::Math::Bignum => [ [@other, LibTom::Math::Bignum.new(42424242424242424242),
                                            [-41189674533254769921,41189674533254769921]]]
                } ,

        "*" => { Float => [ [@other, 42.42, [51851851421481481482, 51851851421481481482]],
                            [@other, -42.42, [-51851851421481481482, -51851851421481481482]],
                            [@other, 2.0, [2469135781975308642, 2469135781975308642]],
                            [@other, -2.0, [-2469135781975308642, -2469135781975308642]]],
                 Integer =>[[@other,42, [51851851421481481482,51851851421481481482]],
                            [@other,-42, [-51851851421481481482,-51851851421481481482]],
                            [@other,2, [2469135781975308642, 2469135781975308642]],
                            [@other,-2, [-2469135781975308642, -2469135781975308642]]],
                 Bignum => [[@other,42424242424242424242, [52375607496445940890385334834126449682,
                                                          52375607496445940890385334834126449682]],
                            [@other,-42424242424242424242, [-52375607496445940890385334834126449682,
                                                          -52375607496445940890385334834126449682]]],
                              
                 LibTom::Math::Bignum => [[@other,
                                           LibTom::Math::Bignum.new(42424242424242424242),
                                           [ 52375607496445940890385334834126449682,
                                               52375607496445940890385334834126449682]],
                                          [@other,
                                           LibTom::Math::Bignum.new(-42424242424242424242),
                                           [ -52375607496445940890385334834126449682,
                                             -52375607496445940890385334834126449682]],
                                          [@other,
                                           LibTom::Math::Bignum.new(2),
                                           [2469135781975308642,2469135781975308642]],
                                          [@other,
                                           LibTom::Math::Bignum.new(-2),
                                           [-2469135781975308642,-2469135781975308642]]
                                         ]
               },
        "/" => { Float => [ [@other, 42.42, [29394473594944150, 0]],
                            [@other, -42.42, [-29394473594944150, 0]],
                            [@other, 2.0, [617283945493827160, 0]],
                            [@other, -2.0, [-617283945493827160, 0]]],
                 Integer =>[[@other,42, [29394473594944150,0]],
                            [@other,-42, [-29394473594944150,0]],
                            [@other,2, [617283945493827160,0]],
                            [@other,-2, [-617283945493827160,0]],
                           ],
                 Bignum => [[@other,42424242424242424242, [0, 34]],
                            [@other,-42424242424242424242, [0, -34]]],
                 LibTom::Math::Bignum => [[@other, LibTom::Math::Bignum.new(42424242424242424242), [ 0, 34 ]],
                                          [@other, LibTom::Math::Bignum.new(-42424242424242424242), [ 0, -34 ]],
                                          [@other, LibTom::Math::Bignum.new(2),[617283945493827160,0]],
                                          [@other, LibTom::Math::Bignum.new(-2),[-617283945493827160,0]]]
               },
        "%" => { Float => [ [@other, 42.42, [21, 42]],
                            [@other, -42.42, [-21, 1234567890987654279]],
                          ],
                 Integer => [ [@other, 42, [21, 42]],
                              [@other, -42, [-21, 1234567890987654279]]
                            ],
                 Bignum => [[@other,42424242424242424242,[1234567890987654321,448934130662177328]],
                            [@other,-42424242424242424242, [-41189674533254769921, 785633760325476993]]
                            ],
                 LibTom::Math::Bignum=> [[@other,LibTom::Math::Bignum.new(42424242424242424242),[1234567890987654321,448934130662177328]],
                                         [@other,LibTom::Math::Bignum.new(-42424242424242424242), [-41189674533254769921, 785633760325476993]]
                            ],
               },
    }
    answers.each_pair do |op,op_suite|
        op_suite.each do |klass,klass_suite|
            klass_suite.each do |config|
                param1 = config[0]
                param2 = config[1]
                results = config[2]
                tests = [{ :receiver => param1, :param => param2, :result => results[0] },
                         { :receiver => param2, :param => param1, :result => results[1] }]
                tests.each do |test|
                    should_text = "#{test[:receiver]} #{op} #{test[:param]} => #{test[:result]} (#{test[:receiver].class.name} #{op} #{test[:param].class.name})"
                    it should_text do
                        c = test[:receiver].send(op,test[:param])
                        c.should == test[:result]
                    end
                end
            end
        end
    end

    it "should throw ZeroDivisionError when / 0 (Integer)" do
        lambda { @a / 0 }.should raise_error(ZeroDivisionError)
    end
    
    it "should throw ZeroDivisionError when / 0 (Float)" do
        lambda { @a / 0.0 }.should raise_error(ZeroDivisionError)
    end
    
    it "should throw ZeroDivisionError when / 0 (LTMBignum)" do
        zero = LibTom::Math::Bignum.new(0)
        lambda { @a / zero }.should raise_error(ZeroDivisionError)
    end

    it "should perform divmod correctly - all pos " do
        @a.divmod(42).should == [29394473594944150, 21]
    end
    
    it "should perform divmod correctly - param neg " do
        @a.divmod(-42).should == [-29394473594944150, -21]
    end
    
    it "should perform divmod correctly - self neg " do
        (-@a).divmod(42).should == [-29394473594944150, 21]
    end
    
    it "should perform divmod correctly - both neg " do
        (-@a).divmod(-42).should == [29394473594944150, -21]
    end

    it "should perform remainder correctly - all pos" do
        @a.remainder(42).should == 21
    end
    
    it "should perform remainder correctly - param neg " do
        @a.remainder(42).should == 21
    end
    
    it "should perform remainder correctly - self neg " do
        (-@a).remainder(42).should == -21
    end
    
    it "should perform remainder correctly - both neg " do
        (-@a).remainder(-42).should == -21
    end

    it "should be able to raise to a power " do
        (@a ** 8).should == (1234567890987654321 ** 8)
    end

    it "should be able to raise to a big power" do
        (@a**1000).should == (1234567890987654321 ** 1000)
    end
end

describe LibTom::Math::Bignum, "utility operations" do
    before(:each) do
        @a = LibTom::Math::Bignum.new(1234567890987654321)
        @b = LibTom::Math::Bignum.new(9876543210123456789)
        @float = 1234567890987654321.to_f
    end

    [[10,11],[20,21],[100,101]].each do |power,bytes|
        it "should say how many bytes the machine representation is - #{bytes}" do
            a = LibTom::Math::Bignum.new((256**power - 1))
            a.size.should == bytes
        end
    end

    it "should convert to a Float (positive num)" do
        @a.to_f.should == @float
    end

    it "should convert to a Float (negative num)" do
        (-@a).to_f.should == -@float
    end

    it "should compare against other numbers - big < big" do
        @a.should < @b
    end
    
    it "should compare against other numbers - big > big" do
        @b.should > @a 
    end
    
    it "should compare against other numbers - big < int " do
        LibTom::Math::Bignum.new(42).should <  64
    end
    
    it "should compare against other numbers - big > int " do
        @a.should > 42 
    end

    it "should be able to detect nonzero via nonzero?" do
        @a.should be_nonzero
    end

    it "should be able to detect zero via nonzero?" do
        LibTom::Math::Bignum.new(0).should_not be_nonzero
    end

    it "should be able to detect nonzero via zero?" do
        @a.should_not be_zero
    end

    it "should be able to detect zero via nonzero?" do
        LibTom::Math::Bignum.new(0).should be_zero
    end

    it "should be able to zero itself out" do
        @a.zero!
        @a.should be_zero
    end

    it "should hash appropriately" do
        @a.hash.should_not equal(@b.hash)
    end

    it "should match hashes on same numbers" do
        c = @a.dup
        c.hash.should equal(@a.hash)
    end
end


describe LibTom::Math::Bignum, "bitwise operations" do
    before(:each) do
        @a = LibTom::Math::Bignum.new(1234567890987654321)
        @b = LibTom::Math::Bignum.new(9876543210123456789)
        @c = 1234567890987654321
        @d = 9876543210123456789
    end

    it "should AND correctly" do
        (@a & @b).should == (@c & @d)
    end
    
    it "should OR correctly" do
        (@a | @b).should == (@c | @d)
    end
    
    it "should XOR correctly" do
        (@a ^ @b).should == (@c ^ @d)
    end

    it "should << Integer correctly" do
        (@a << 8).should == (@c << 8)
    end

    it "should << Float correctly" do
        (@a << 8.5).should == (@c << 8.5)
    end

    it "should not allow << of non numbers " do
        lambda { @a << "foo" }.should raise_error(TypeError)
    end

    it "should >> Integer correctly" do
        (@a >> 8).should == (@c >> 8)
    end

    it "should << Float correctly" do
        (@a >> 8.5).should == (@c >> 8.5)
    end

    it "should not allow >> of non numbers " do
        lambda { @a >> "foo" }.should raise_error(TypeError)
    end

    it "should NOT implement bitwise negation - NOT IMPLEMENTED " do
        lambda { ~@a }.should raise_error(NotImplementedError);
    end
    
    it "should NOT implement bit refencing - NOT IMPLEMENTED " do
        lambda { @a[0] }.should raise_error(NotImplementedError);
    end

    it "should return the number of bits in the internal representation" do
        @a.num_bits.should == 61
    end
end

describe LibTom::Math::Bignum, "Bonus methods" do 
    before(:each) do
        @a = LibTom::Math::Bignum.new(1234567890987654321)
        @b = LibTom::Math::Bignum.new(9876543210123456789)
        @c = 1234567890987654321
        @d = 9876543210123456789
    end

    it "should right shift 'digits'" do
        @a.right_shift_digits(2).should == 17
    end

    it "should left shift 'digits'" do
        LibTom::Math::Bignum.new(17).left_shift_digits(2).should == 1224979098644774912
    end

    it "should calculate powers of 2" do
        p = LibTom::Math.two_to_the(128)
        p.should == 340282366920938463463374607431768211456 
    end

    it "should square a Bignum" do
        @b.squared.should == (9876543210123456789 * 9876543210123456789)
    end

    it "should do a = b module 2**d" do
        @b.modulo_pow2(12).should == 277
    end

    it "should generate random numbers with minimum bit length" do
        p = LibTom::Math::rand_of_size(1024)
        p.num_bits.should >= 1024
    end

    it "should do d = (a + b) mod c" do
        @a.add_modulus(@b,42).should == ((@c + @d) % 42)
    end

    it "should do d = (a - b) mod c" do
        @a.subtract_modulus(@b,42).should == ((@c - @d) % 42)
    end
    
    it "should do d = (a * b) mod c" do
        @a.multiply_modulus(@b,42).should == ((@c * @d) % 42)
    end
    
    it "should do c = (a ** 2) mod b" do
        @a.square_modulus(42).should == ((@c * @c) % 42)
    end
    
    it "should do d = (a ** b) mod c" do
        @a.exponent_modulus(42,4321).should == ((@c ** 42) % 4321)
    end


    it "should do c = (1/a) mod b" do
        s = LibTom::Math::Bignum.new(4)
        s.inverse_modulus(11).should == 3
    end

    it "should find the greatest common divisor" do
        s = LibTom::Math::Bignum.new(42)
        s.gcd(56).should == 14
    end

    it "should use the extended euclidian algorithm" do
        a = LibTom::Math::Bignum.new(120)
        a.extended_euclidian(23).should == [-9,47,1]
    end

    it "should find the least common multiple" do
        s = LibTom::Math::Bignum.new(6)
        s.lcm(21) == 42
    end

    it "should compute the jacobi symbol of a number " do
        @a.jacobi(42).should == 0
    end

    it "should find the nth root of a number" do
        s = LibTom::Math::Bignum.new(9513149571461376)
        s.nth_root(4).should == 9876
    end

    it "should find the square root of a number" do
        s = LibTom::Math::Bignum.new(1764)
        s.square_root.should == 42
    end

    it "should tell if a number is a square" do
        s = LibTom::Math::Bignum.new(2048**2)
        s.should be_is_square
    end

    it "should say if a number is not a square" do
        s = LibTom::Math::Bignum.new(2)
        s = (s**21701)-1
        s.should_not be_is_square
    end
end
describe LibTom::Math::Bignum, "Prime Number Methods" do
    it "should administer a fermat primality test and get true " do
        a = LibTom::Math::Bignum.new(13)
        a.should be_passes_fermat_primality(4)
    end

    it "should administer a fermat primality test and get false" do
        a = LibTom::Math::Bignum.new(42)
        a.should_not be_passes_fermat_primality(4)
    end

    it "should trick the fermat primality test with a Carmichael number and get true" do
        a = LibTom::Math::Bignum.new(561)
        a.should be_passes_fermat_primality(11)
    end

    it "should check if a number is prime by dividing it by some primes and get true" do
        a = LibTom::Math::Bignum.new(1619**2)
        a.should be_divisible_by_some_primes
    end
    
    it "should check if a number is prime by dividing it by some primes and get false" do
        a = LibTom::Math::Bignum.new(104729)
        a.should_not be_divisible_by_some_primes
    end

    it "should administer a Miller-Rabin test and get true" do
        a = LibTom::Math::Bignum.new(104729)
        a.should be_passes_miller_rabin(1729)
    end
    
    it "should administer a Miller-Rabin test and get false" do
        a = LibTom::Math::Bignum.new(2989441)
        a.should_not be_passes_miller_rabin(13)
    end

    it "should administer a Miller-Rabin on a Carmichale number and get false" do
        a = LibTom::Math::Bignum.new(1729)
        a.should be_passes_miller_rabin(13)
    end

    it "should say how many Miller-Rabin tests to run on a number" do
        ap = LibTom::Math::Bignum.new(2**256-1)
        ap.num_miller_rabin_trials.should == 16
    end

    it "should detect if a number is prime within a miniscule probability" do
        a = 2**256 - 1
        ap = LibTom::Math::Bignum.new(a)
        ap.should_not be_is_prime
    end
    
    it "should throw an error if trials > 256" do
        a = 2**256 - 1
        ap = LibTom::Math::Bignum.new(a)
        lambda{ ap.is_prime?(257) }.should raise_error(ArgumentError)
    end

    it "should throw an error if trials > 256 on next_prime" do
        a = LibTom::Math::Bignum.new(104729)
        lambda{ a.next_prime({ :trials => -1 }) }.should raise_error(ArgumentError)
    end
    
    it "should find the 1,000,001th prime" do
        a = LibTom::Math::Bignum.new(15485863)
        a.next_prime.should == 15485867
    end

    it "should find the first 1,000 primes" do
        require 'mathn'
        a = LibTom::Math::Bignum.new(1) 
        p = Prime.new
        p_list = []
        a_list = []
        1_000.times do 
            a_list << a = a.next_prime
            p_list << p.succ
        end
        a_list.should == p_list
    end 

end
