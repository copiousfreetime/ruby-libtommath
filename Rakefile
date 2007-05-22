require 'rake'
require 'rake/clean'

desc "Does a full compile and test run"
task :default => [:compile, :test]

desc "Compiles extensions"
tast :compile => [:libtommath] do
    puts "Yeah!"
end

desc "Compiles libtommath extension" 
task :libtommath do
    extconf "ext/extconf.rb"
end
