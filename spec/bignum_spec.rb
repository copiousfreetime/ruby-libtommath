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
    before(:each) do
        @a = LibTom::Math::Bignum.new(1234567890987654321)
        @b = LibTom::Math::Bignum.new(9876543210123456789)
    end

    it "should add correctly" do
        c = @a - @b
        c.should == 11111111101111111110
    end
end
