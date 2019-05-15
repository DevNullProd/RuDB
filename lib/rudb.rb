require 'rudb.so'

module RuDB
  def self.each(path)
    RuDB::visit(path, proc { |key, val|
      yield key, val
    })
  end
end
