require 'libtom/math'

describe LibTom::Math::Prime do
    it "should say how many Miller-Rabin tests to run on a number" do
        b = LibTom::Math::two_to_the(256) - 1
        LibTom::Math::Prime.num_miller_rabin_trials_for(b).should == 16
    end

    it "should raise an exception if the number of bits is not given when generating a random prime" do
        lambda { LibTom::Math::Prime::random_of_size }.should raise_error(ArgumentError)
    end 

    it "should find the 1,000,001th prime" do
        a = LibTom::Math::Prime.new(15485863)
        a.next.should == 15485867
    end

    it "should find the first 1,000 primes" do
        require 'mathn'
        a = LibTom::Math::Prime.new
        a_list = []
        
        p = Prime.new
        p_list = []

        1_000.times do
            a_list << a.succ
            p_list << p.succ
        end

        a_list.should == p_list
    end

    it "should generate a random prime of a minimum bitsize" do
        rp = LibTom::Math::Prime::random_of_size(256)
        rp.num_bits.should >= 256
        rp.should be_is_prime
    end

    it "can start with a large prime " do
        rp = LibTom::Math::Prime::random_of_size(256)
        p = LibTom::Math::Prime.new(rp)
        p.succ.should be_is_prime
    end

    it "should generate primes according to parameters" do
        options = { :congruency => true,
                    :msb => true,
                    :safe => true }
        rp = LibTom::Math::Prime::random_of_size(256, options)
        (rp % 4).should == 3 
        ((rp - 1)/2).should be_is_prime
    end 
    

end
