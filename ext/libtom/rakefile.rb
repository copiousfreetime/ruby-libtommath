# this is a stub rakefile to print out useful information for those that
# are on rubygems < 0.9.5.  This rakefile.rb has a lower order of
# precidence than Rakefile so this will be executed by rubygems < 0.9.5
# and the companion mkrf_conf.rb will be executed by rubygems >= 0.9.5

require 'rubygems/installer'

task :default => :extension
task :extension do 
    msg = <<-EOM

#{'ERROR! ' * 10} 
ERROR!
ERROR! RubyGems version #{Gem::RubyGemsVersion} is not capable of installing 
ERROR! gems that use mkrf for building extensions.  
ERROR! 
ERROR! You can either upgrade to at least rubygems 0.9.5 or install via the
ERROR! ruby-libtommath .tgz or .zip file found on rubyforge.
ERROR!   
ERROR!        http://rubyforge.org/frs/?group_id=3707
ERROR!
#{'ERROR! ' * 10} 

EOM
    raise Gem::InstallError, msg
end
