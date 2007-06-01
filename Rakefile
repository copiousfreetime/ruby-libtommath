# make sure ./lib is added to the ruby search path
$: << File.expand_path(File.join(File.dirname(__FILE__),"lib"))

require 'ostruct'
require 'rubygems'
require 'rake/gempackagetask'
require 'rake/clean'
require 'rake/rdoctask'
require 'spec/rake/spectask'

PKG_INFO = OpenStruct.new

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
    r.libs      << "./ext"
    r.rcov      = false
    r.spec_opts  = %w(-f s)
end

rd = Rake::RDocTask.new do |rdoc|
    rdoc.rdoc_dir   = "doc/rdoc"
    rdoc.rdoc_files = FileList['lib/**/*.rb', 'ext/**/*.c']
end

