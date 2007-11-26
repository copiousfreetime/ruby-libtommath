#-----------------------------------------------------------------------
# Extension building
#-----------------------------------------------------------------------
require 'pathname'
namespace :extension do 
    vendor_dir   = File.join('vendor','libtommath-0.41')
    ext_dir      = File.dirname(LibTom::Math::SPEC.extensions.first)

    task :headers do 
        %w[ tommath.h tommath_class.h tommath_superclass.h ].each do |h|
            h_file = File.join(ext_dir, h)
            cp File.join(vendor_dir, h), h_file
            puts "copying #{h_file}"
            LibTom::Math::SPEC.files << h_file

        end
    end

    src_files = FileList["#{vendor_dir}/bn*.c"]
    file "#{ext_dir}/libtommath.c" => src_files do
        puts "Building #{ext_dir}/libtommath.c"
        File.open("#{ext_dir}/libtommath.c","w") do |all|
            all.puts <<EOH
/**********************************************************************
 **********************************************************************
 ** 
 ** File generated on : #{Time.now.strftime("%Y-%m-%d %H:%M:%S")}
 **
 ** libtommath.c is an Amalgamated source code file of all LibTomMath 
 ** functions generated on by a rake task as part of building the 
 ** ruby-libtommath gem.  
 ** 
 ** LibTomMath is in the Public Domain and available from the following 
 ** URL.
 **
 **      http://libtom.org/?page=features&newsitems=5&whatfile=ltm
 **
 *********************************************************************
 *********************************************************************/
EOH
            src_files.sort.each do |dot_c|
                all.puts
                all.puts "/*" + "-" * 70
                all.puts " -- Start: #{dot_c}"
                all.puts " " + "-" * 69 + "*/"
                all.puts IO.read(dot_c)
                all.puts "/*" + "-" * 70
                all.puts " --   End: #{dot_c}"
                all.puts " " + "-" * 69 + "*/"
            end
            all.puts "/** END OF GENERATED FILE **/"
            LibTom::Math::SPEC.files << "#{ext_dir}/libtommath.c"
        end
    end
    task :amalgamated_source => "#{ext_dir}/libtommath.c"
    
    
    desc "Prepare to build the extension"
    task :prepare => [:headers, :amalgamated_source]
    
    desc "Build the extension"
    task :build => [ :prepare ]do
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
        rm_f FileList["#{ext_dir}/**/*tommath*"]
    end
    
end
