require 'mkmf'
dir_config("libtommath")
find_library("tommath","mp_init") 
find_header("tommath.h")
create_makefile("_math")

