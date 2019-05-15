#include RUBY_EXTCONF_H

#include <ruby.h>
#include <nudb/nudb.hpp>

static VALUE mRuDB;
static VALUE cRuDB_store;
static VALUE cRuDB_ec;

struct nudb_store_pointer {
  nudb::store* store;
};

///

// TODO pull in error_category (requires ref mark?)

void ec_free(nudb::error_code* ec_pointer){
  delete ec_pointer;
}

VALUE ec2obj(nudb::error_code* ec){
  return Data_Wrap_Struct(cRuDB_ec, 0, ec_free, ec);
}

VALUE rudb_ec_value(VALUE _self){
  nudb::error_code* ec_pointer;
  Data_Get_Struct(_self, nudb::error_code , ec_pointer);
  return INT2NUM(ec_pointer->value());
}

VALUE rudb_ec_message(VALUE _self){
  nudb::error_code* ec_pointer;
  Data_Get_Struct(_self, nudb::error_code , ec_pointer);
  return rb_str_new_cstr(ec_pointer->message().c_str());
}

///

VALUE rudb_create(VALUE _self, VALUE opts) {
  VALUE dat         = rb_hash_aref(opts, ID2SYM(rb_intern("dat_path")));
  VALUE key         = rb_hash_aref(opts, ID2SYM(rb_intern("key_path")));
  VALUE log         = rb_hash_aref(opts, ID2SYM(rb_intern("log_path")));
  VALUE app_num     = rb_hash_aref(opts, ID2SYM(rb_intern("app_num")));
  VALUE salt        = rb_hash_aref(opts, ID2SYM(rb_intern("salt")));
  VALUE key_size    = rb_hash_aref(opts, ID2SYM(rb_intern("key_size")));
  VALUE block_size  = rb_hash_aref(opts, ID2SYM(rb_intern("block_size")));
  VALUE load_factor = rb_hash_aref(opts, ID2SYM(rb_intern("load_factor")));

  Check_Type(dat,         T_STRING);
  Check_Type(key,         T_STRING);
  Check_Type(log,         T_STRING);
  Check_Type(app_num,     T_FIXNUM);
  Check_Type(salt,        T_FIXNUM);
  Check_Type(key_size,    T_FIXNUM);
  Check_Type(block_size,  T_FIXNUM);
  Check_Type(load_factor, T_FLOAT);

  std::string _dat = std::string((char*)RSTRING_PTR(dat), RSTRING_LEN(dat));
  std::string _key = std::string((char*)RSTRING_PTR(key), RSTRING_LEN(key));
  std::string _log = std::string((char*)RSTRING_PTR(log), RSTRING_LEN(log));

  std::uint64_t _app_num    = NUM2ULL(app_num);
  std::uint64_t _salt       = NUM2ULL(salt);
  nudb::nsize_t _key_size   = NUM2UINT(key_size);
  nudb::nsize_t _block_size = NUM2UINT(block_size);
  float _load_factor        = NUM2DBL(load_factor);

  // TODO parameterize hasher typer
  nudb::error_code ec;
  nudb::create<nudb::xxhasher>(_dat, _key, _log,
                               _app_num,
                               _salt,
                               _key_size,
                               _block_size,
                               _load_factor,
                               ec);

  return ec2obj(new nudb::error_code(ec));
}

VALUE rudb_erase_file(VALUE _self, VALUE file) {
  Check_Type(file, T_STRING);
  std::string fil = std::string((char*)RSTRING_PTR(file), RSTRING_LEN(file));

  nudb::error_code ec;
  nudb::erase_file(fil, ec);

  return ec2obj(new nudb::error_code(ec));
}

VALUE rudb_make_salt(VALUE _self) {
  return INT2NUM(nudb::make_salt());
}

VALUE rudb_block_size(VALUE _self, VALUE _path) {
  Check_Type(_path, T_STRING);
  std::string path  = std::string((char*)RSTRING_PTR(_path), RSTRING_LEN(_path));
  return INT2NUM(nudb::block_size(path));
}

// TODO parameterize progress callback
VALUE rudb_visit(VALUE _self, VALUE path, VALUE callback){
  Check_Type(path, T_STRING);
  std::string _path  = std::string((char*)RSTRING_PTR(path), RSTRING_LEN(path));

  nudb::error_code ec;
  nudb::visit(_path, [&](void const* key, std::size_t key_size,
                         void const* val, std::size_t val_size,
                         nudb::error_code& ec1){
          VALUE key_str = rb_str_new((char const*)key, key_size);
          VALUE val_str = rb_str_new((char const*)val, val_size);

          rb_funcall(callback, rb_intern("call"), 2, key_str, val_str);
        }, nudb::no_progress{}, ec);

  return ec2obj(new nudb::error_code(ec));
}

///

void db_free(nudb_store_pointer* store_pointer){
  if(store_pointer->store != NULL){
    // FIXME nudb::store is polymorphic w/out virtual destructor
    delete store_pointer->store;
    store_pointer->store = NULL;
  }
  delete store_pointer;
}

static VALUE store_alloc(VALUE klass){
  nudb_store_pointer* store_pointer = ALLOC(nudb_store_pointer);
  return Data_Wrap_Struct(klass, 0, db_free, store_pointer);
}

VALUE rudb_store_init(VALUE _self){
  nudb_store_pointer* store_pointer;
  Data_Get_Struct(_self, nudb_store_pointer, store_pointer);
  store_pointer->store = new nudb::store();
  return _self;
}

VALUE rudb_store_open(VALUE _self, VALUE dat_path, VALUE key_path, VALUE log_path) {
  Check_Type(dat_path, T_STRING);
  Check_Type(key_path, T_STRING);
  Check_Type(log_path, T_STRING);

  nudb_store_pointer* store_pointer;
  Data_Get_Struct(_self, nudb_store_pointer, store_pointer);

  std::string dat = std::string((char*)RSTRING_PTR(dat_path),
                                       RSTRING_LEN(dat_path));
  std::string key = std::string((char*)RSTRING_PTR(key_path),
                                       RSTRING_LEN(key_path));
  std::string log = std::string((char*)RSTRING_PTR(log_path),
                                       RSTRING_LEN(log_path));

  nudb::error_code ec;
  store_pointer->store->open(dat, key, log, ec);

  return ec2obj(new nudb::error_code(ec));
}

VALUE rudb_store_insert(VALUE _self, VALUE key, VALUE value){
  Check_Type(key,   T_STRING);
  Check_Type(value, T_STRING);

  std::string key_str   = std::string((char*)RSTRING_PTR(key),   RSTRING_LEN(key));
  std::string value_str = std::string((char*)RSTRING_PTR(value), RSTRING_LEN(value));

  nudb_store_pointer* store_pointer;
  Data_Get_Struct(_self, nudb_store_pointer, store_pointer);

  if (store_pointer->store == NULL) {
    rb_raise(rb_eRuntimeError, "db not open");
  }

  nudb::error_code ec;
  store_pointer->store->insert(key_str.c_str(),
                               value_str.c_str(),
                               value_str.size(), ec);

  return ec2obj(new nudb::error_code(ec));
}

VALUE rudb_store_fetch(VALUE _self, VALUE key){
  Check_Type(key, T_STRING);
  std::string key_str = std::string((char*)RSTRING_PTR(key), RSTRING_LEN(key));

  nudb_store_pointer* store_pointer;
  Data_Get_Struct(_self, nudb_store_pointer, store_pointer);

  if (store_pointer->store == NULL) {
    rb_raise(rb_eRuntimeError, "db not open");
  }

  std::string result;
  nudb::error_code ec;
  store_pointer->store->fetch(key_str.c_str(),
    [&](void const* buffer, std::size_t size){
      result = std::string((char*) buffer, size);
    }, ec);

  VALUE result_obj = rb_str_new(result.c_str(),
                                result.size());

  // TODO if block given invoke w/ result_obj (only if succeeded)

  VALUE ec_obj = ec2obj(new nudb::error_code(ec));

  VALUE ret = rb_ary_new();
  rb_ary_push(ret, result_obj); // TODO Qnil if not success
  rb_ary_push(ret, ec_obj);

  return ret;
}

VALUE rudb_store_close(VALUE _self){
  nudb_store_pointer* store_pointer;
  Data_Get_Struct(_self, nudb_store_pointer, store_pointer);

  nudb::error_code ec;
  store_pointer->store->close(ec);

  return ec2obj(new nudb::error_code(ec));
}

///

typedef VALUE (*METHOD)(...);

extern "C"{
  void Init_rudb(){
      mRuDB = rb_define_module("RuDB");
      rb_define_singleton_method(mRuDB, "create",     (METHOD)rudb_create,     1);
      rb_define_singleton_method(mRuDB, "erase_file", (METHOD)rudb_erase_file, 1);
      rb_define_singleton_method(mRuDB, "make_salt",  (METHOD)rudb_make_salt,  0);
      rb_define_singleton_method(mRuDB, "block_size", (METHOD)rudb_block_size, 1);
      rb_define_singleton_method(mRuDB, "visit",      (METHOD)rudb_visit,      2);

      cRuDB_store = rb_define_class_under(mRuDB, "Store", rb_cObject);
      rb_define_alloc_func(cRuDB_store, store_alloc);
      rb_define_method(cRuDB_store, "initialize", (METHOD)rudb_store_init, 0);
      rb_define_method(cRuDB_store, "open",       (METHOD)rudb_store_open, 3);
      rb_define_method(cRuDB_store, "insert",     (METHOD)rudb_store_insert, 2);
      rb_define_method(cRuDB_store, "fetch",      (METHOD)rudb_store_fetch, 1);
      rb_define_method(cRuDB_store, "close",      (METHOD)rudb_store_close, 0);

      cRuDB_ec = rb_define_class_under(mRuDB, "ErrorCode", rb_cObject);
      rb_define_method(cRuDB_ec, "value",    (METHOD)rudb_ec_value, 0);
      rb_define_method(cRuDB_ec, "message",  (METHOD)rudb_ec_message, 0);
  }
}
