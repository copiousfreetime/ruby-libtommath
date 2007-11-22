require 'spec/rake/spectask'

#-----------------------------------------------------------------------
# Testing - this is either test or spec, include the appropriate one
#-----------------------------------------------------------------------
namespace :test do

    task :default => :spec

    Spec::Rake::SpecTask.new do |r| 
        r.rcov      = true
        r.rcov_dir  = LibTom::Math::SPEC.local_coverage_dir
        r.libs      = LibTom::Math::SPEC.require_paths
        r.spec_opts = %w(--format specdoc --color)
    end

    task :spec => [ "extension:build"]
    task :coverage => [:spec] do
        show_files LibTom::Math::SPEC.local_coverage_dir
    end 
end
