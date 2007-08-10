#-----------------------------------------------------------------------
# Extension building
#-----------------------------------------------------------------------
namespace :extension do 
    desc "Build the extension"
    task :build do
        LibTom::Math::SPEC.extensions.each do |extension|
            path = Pathname.new(extension)
            parts = path.split
            conf = parts.last
            Dir.chdir(path.dirname) do |d|
                puts "in #{d}"
                ruby conf.to_s
                sh "rake default"
            end
        end
    end
    
    desc "Clobber generated files from extension building"
    task :clobber do 
        LibTom::Math::SPEC.extensions.each do |extension|
            ext_dir = File.dirname(extension)
            rm_f FileList["#{ext_dir}/**/*.o","#{ext_dir}/**/*.log", "#{ext_dir}/**/*.so", "#{ext_dir}/**/*.bundle", "#{ext_dir}/**/Rakefile"]
        end
    end
end