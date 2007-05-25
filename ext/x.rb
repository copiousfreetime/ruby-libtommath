require 'libtommath'

x = LibTom::Math::Bignum.new(12345)
puts "created LTMBignum from int #{x}"
s = "12345" * 10
y = LibTom::Math::Bignum.new(s)
puts "created LTMBignum from string #{y}"
puts "created LTMBignum from string #{y.to_s(64)}"
z =
LibTom::Math::Bignum.new(123456789098765432109872398792873958719283759817293875918273958712983749218734)
puts "created LTMBignum from ::Bignum #{z}"
