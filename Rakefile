require 'rubygems'
require 'rake/gempackagetask'
require 'rake/clean'
require 'rake/rdoctask'
require 'spec/rake/spectask'

$: << File.join(File.dirname(__FILE__),"lib")

SPEC = Gem::Specification.new do |s|
   s.name               = "ruby-libtommath"
   s.author             = "Jeremy Hinegardner"
   s.email              = "jjh@hinegardner.org"
   s.homepage           = "http://copiousfreetime.rubyforge.org/ruby-libtommath"
   s.summary            = "Ruby extension for LibTom Math"
   s.description        =<<-DESC
   LibTomMath is a free open source portable number theoretic
   multiple-precision integer library.
   DESC

   s.extensions         = "ext/libtom/math/extconf.rb"
   s.extra_rdoc_files   = FileList["README", "LICENSE"]
   s.files              = FileList["ext/**/*", "lib/**/*", "spec/**/*","examples/**/*"]
   s.has_rdoc           = true
   s.rdoc_options       << [ "--line-numbers" , "--inline-source", "--title", s.summary,
                             "--main", "README" ]

   s.require_paths      << "ext"
   s.requirements       = "LibTomMath version 0.41 or greater"
   s.rubyforge_project  = "copiousfreetime"
   s.version            = Gem::Version.create("0.7.0")
end

rd = Rake::RDocTask.new do |rdoc|
    rdoc.rdoc_dir   = "doc/rdoc"
    rdoc.title      = SPEC.summary
    rdoc.main       = "README"
    rdoc.rdoc_files = SPEC.files + SPEC.extra_rdoc_files
end

Rake::GemPackageTask.new(SPEC) do |pkg|
    pkg.need_tar = true
    pkg.need_zip = true
end

desc "Install as a gem"
task :install_gem => [:clobber, :package] do
    sh "sudo gem install pkg/*.gem"
end

desc "Dump gemspec"
task :spec do
    puts SPEC.to_ruby
end

rspec = Spec::Rake::SpecTask.new do |r|
    r.warning   = true
    r.libs      = SPEC.require_paths
    r.spec_opts = %w(-f s)
end

