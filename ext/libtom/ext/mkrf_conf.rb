require 'rubygems'
require 'mkrf'
Mkrf::Generator.new('math') do |g|
    LIB_PATHS = %w[ /opt/local/lib ]
    
    g.logger.level = Logger::DEBUG
    if not g.include_library('tommath','mp_init',*LIB_PATHS) then
        abort "LibTomMath library not found.  Unable to continue."
    end
    
    INCLUDE_PATHS = %w[ /usr/include/tommath /usr/local/include/libtommath /opt/local/include /opt/local/include/libtommath ]
    
    if not g.include_header('tommath.h',*INCLUDE_PATHS) then
        abort "LibTomMath header file not found.  Unable to continue."
    end
end
