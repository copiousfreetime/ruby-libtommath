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
                            [@other, 2.0, [2469135781975308642, 2469135781975308642]]],
                 Integer =>[[@other,42, [51851851421481481482,51851851421481481482]],
                            [@other,2, [2469135781975308642, 2469135781975308642]]],
                 Bignum => [[@other,42424242424242424242, [52375607496445940890385334834126449682,
                                                          52375607496445940890385334834126449682]]],
                 LibTom::Math::Bignum => [[@other,
                                           LibTom::Math::Bignum.new(42424242424242424242),
                                           [ 52375607496445940890385334834126449682,
                                               52375607496445940890385334834126449682]],
                                          [@other,
                                           LibTom::Math::Bignum.new(2),
                                           [2469135781975308642,2469135781975308642]]]
               },
        "/" => { Float => [ [@other, 42.42, [51851851421481481482, 51851851421481481482]],
                            [@other, 2.0, [2469135781975308642, 2469135781975308642]]],
                 Integer =>[[@other,42, [51851851421481481482,51851851421481481482]],
                            [@other,2, [2469135781975308642, 2469135781975308642]]],
                 Bignum => [[@other,42424242424242424242, [52375607496445940890385334834126449682,
                                                          52375607496445940890385334834126449682]]],
                 LibTom::Math::Bignum => [[@other,
                                           LibTom::Math::Bignum.new(42424242424242424242),
                                           [ 52375607496445940890385334834126449682,
                                               52375607496445940890385334834126449682]],
                                          [@other,
                                           LibTom::Math::Bignum.new(2),
                                           [2469135781975308642,2469135781975308642]]]
               }
 
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
                    should_text = "should #{op} with #{klass.name} and produce the right answer (#{test[:receiver].class.name} #{op} #{test[:param].class.name})"
                    it should_text do
                        c = test[:receiver].send(op,test[:param])
                        c.should == test[:result]
                    end
                end
            end
        end
    end
end
