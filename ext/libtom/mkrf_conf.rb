require 'rubygems'
require 'mkrf'
Mkrf::Generator.new('libtommath') do |g|
    # completely self contained, defaults work great.
end
