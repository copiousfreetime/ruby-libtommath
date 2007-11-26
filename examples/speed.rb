#!/usr/bin/env ruby

top_level = File.expand_path(File.join(File.dirname(__FILE__),".."))
$: << File.join(top_level,"lib")
$: << File.join(top_level,"ext/libtom")

require 'libtom/math'
require 'mathn'
require 'benchmark'

number   = 1000
bitsize  = 1024
ltm_randoms = []
number.times { 
 ltm_randoms << LibTom::Math::Bignum::random_of_size(bitsize)
}

ltm_results = []

ruby_randoms = ltm_randoms.collect { |b| b.to_s.to_i }
ruby_results = []

puts "Comparing Ruby ::Bignum with LibTom::Math::Bignum over #{number} #{bitsize} bit numbers"
puts

Benchmark.bm(25) do |x|
    x.report("Ruby   GCD") do 
        ruby_randoms.each_with_index { |n,i| ruby_results << n.gcd(ruby_randoms[i-1]) }
    end
    x.report("LibTom GCD") do 
        ltm_randoms.each_with_index { |n,i| ltm_results << n.greatest_common_divisor(ltm_randoms[i-1]) }
    end
end

if ruby_results == ltm_results then
    puts "Ruby Results are the same as the LibTom results"
else
    puts "Ruby Results differ from LibTom"
end
puts
ruby_results.clear
ltm_results.clear

Benchmark.bm(25) do |x|
    x.report("Ruby   LCM") do 
        ruby_randoms.each_with_index { |n,i| ruby_results << n.lcm(ruby_randoms[i-1]) }
    end
    x.report("LibTom LCM") do 
        ltm_randoms.each_with_index { |n,i| ltm_results << n.least_common_multiple(ltm_randoms[i-1]) }
    end
end

if ruby_results == ltm_results then
    puts "Ruby Results are the same as the LibTom results"
else
    puts "Ruby Results differ from LibTom"
end
puts
ruby_results.clear
ltm_results.clear


