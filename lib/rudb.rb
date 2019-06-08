require 'rudb.so'

module RuDB
  def self.each(path)
    RuDB::visit(path, proc { |key, val|
      yield key, val
    })
  end

  class Store
    attr_reader :dat_path,
                :key_path,
                :log_path
  end

  # FIXME: should also marshal error_category (somehow)
  class ErrorCode
    def marshal_dump
      value
    end

    def marshal_load(val)
      self.value = val
    end
  end
end
