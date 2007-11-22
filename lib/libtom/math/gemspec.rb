require 'rubygems'
require 'libtom/math/specification'
require 'libtom/math/version'
require 'rake'

# The Gem Specification plus some extras for libtommath.
module LibTom
    module Math
        SPEC = LibTom::Math::Specification.new do |spec|
                    spec.name               = "ruby-libtommath"
                    spec.version            = LibTom::Math::VERSION
                    spec.rubyforge_project  = "copiousfreetime"
                    spec.author             = "Jeremy Hinegardner"
                    spec.email              = "jeremy@hinegardner.org"
                    spec.homepage           = "http://copiousfreetime.rubyforge.org/libtommath/"

                    spec.summary            = "Ruby extension for LibTom Math"
                    spec.description        = <<-DESC
                    ruby-libtommath is a ruby extension encapsulating the LibTomMath multi-precision
                    integer library (http://libtom.org/?page=features&newsitems=5&whatfile=ltm).
                    
                    It has been written to be an almost complete drop in replacement for ruby's 
                    Bignum.
                    DESC

                    spec.extra_rdoc_files   = FileList["README", "CHANGES", "LICENSE"]
                    spec.has_rdoc           = true
                    spec.rdoc_main          = "README"
                    spec.rdoc_options       = [ "--line-numbers" , "--inline-source" ]

                    spec.test_files         = FileList["spec/**/*.rb", "test/**/*.rb"]
                    spec.files              = spec.test_files + spec.extra_rdoc_files + 
                                              FileList["lib/**/*.rb", "examples/**/*","ext/**/*"]
                
                    spec.extensions         << "ext/libtom/ext/mkrf_conf.rb"
                    spec.requirements       = "LibTomMath version 0.40 or greater"
                    spec.require_paths      << "ext"
                    
                    
                    # add dependencies
                    spec.add_dependency("mkrf", ">= 0.2.2")
                
                    spec.platform           = Gem::Platform::RUBY

                    spec.local_rdoc_dir     = "doc/rdoc"
                    spec.remote_rdoc_dir    = ""
                    spec.local_coverage_dir = "doc/coverage"
                    spec.remote_coverage_dir= "coverage"

                    spec.remote_site_dir    = "#{spec.name}/"

               end
    end
end


