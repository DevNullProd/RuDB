lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'rudb/version'

Gem::Specification.new do |spec|
  spec.name          = "rudb"
  spec.version       = RuDB::VERSION
  spec.authors       = ["Dev Null Productions"]
  spec.email         = ["devnullproductions@gmail.com"]
  spec.description   = %q{Ruby NuDB Wrapper}
  spec.summary       = %q{Wrapper around the NuDB fast key/value insert-only database.}
  spec.homepage      = "https://github.com/DevNullProd/RuDB"
  spec.license       = "MIT"

  spec.files         = Dir.glob("examples/**/*.rb")           +
                       Dir.glob("lib/**/*.rb")                +
                       Dir.glob("spec/**/*.rb")               +
                       Dir.glob("ext/rudb/NuDB/include/**/*") +
                       ["ext/rudb/rudb.cpp",
                        "README.md",
                        "LICENSE.txt",
                        ".yardopts"]

  spec.require_paths = ["lib"]
  spec.extensions    = ["ext/rudb/extconf.rb"]
end
