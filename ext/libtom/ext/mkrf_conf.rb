require 'rubygems'
require 'mkrf'
Mkrf::Generator.new('math') do |g|
    g.logger.level = Logger::DEBUG
    g.include_library('tommath','mp_init')
    g.include_header('tommath.h')
end
