# Port of NuDB/examples/example.cpp to ruby

require 'rudb'

N = 1000
KEY_SIZE = 32/8 # => sizeof(uint32_t) = 4 bytes

dat_path = 'db.dat'
key_path = 'db.key'
log_path = 'db.log'

ec = RuDB::create :dat_path    => dat_path,
                  :key_path    => key_path,
                  :log_path    => log_path,
                  :app_num     => 1,
                  :salt        => RuDB::make_salt,
                  :key_size    => KEY_SIZE,
                  :block_size  => RuDB::block_size('.'),
                  :load_factor => 0.5

db = RuDB::Store.new
ec = db.open(dat_path, key_path, log_path)

data = 0
0.upto(N) { |i|
  ec = db.insert([i].pack("L"),
              [data].pack("L"))
}

0.upto(N) { |i|
  dat, ec = db.fetch([i].pack("L"))
  # do something with dat
}

ec = db.close

RuDB::erase_file dat_path
RuDB::erase_file key_path
RuDB::erase_file log_path
