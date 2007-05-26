require 'rake'
require 'rake/clean'
require 'spec/rake/spectask'

desc "Does a full compile and test run"
task :default => [:compile, :test]

desc "Compiles extensions"
task :compile => [:libtommath] do
    puts "Yeah!"
end

desc "Compiles libtommath extension" 
task :libtommath do
    extconf "ext/extconf.rb"
end

rspec = Spec::Rake::SpecTask.new do |r|
    r.warning   = true
    r.libs      << "./lib"
    r.libs      << "./ext/"
    r.rcov      = false
    r.spec_opts  = %w(-f s)
end


