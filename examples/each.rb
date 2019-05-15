require 'rudb'

N = 10
KEY_SIZE = 32/8 # => sizeof(uint32_t) = 4 bytes

dat_path = 'db.dat'
key_path = 'db.key'
log_path = 'db.log'

RuDB::create :dat_path    => dat_path,
             :key_path    => key_path,
             :log_path    => log_path,
             :app_num     => 1,
             :salt        => RuDB::make_salt,
             :key_size    => KEY_SIZE,
             :block_size  => RuDB::block_size('.'),
             :load_factor => 0.5

db = RuDB::Store.new
db.open(dat_path, key_path, log_path)

0.upto(N) { |i|
  kv = [i].pack("L")
  db.insert(kv, kv)
}

db.close

RuDB::each(dat_path) { |key, value|
  uk = key.unpack("L").first
  uv = value.unpack("L").first
  puts "#{uk}: #{uv}"
}

RuDB::erase_file dat_path
RuDB::erase_file key_path
RuDB::erase_file log_path
