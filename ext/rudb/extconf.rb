require "mkmf"

RbConfig::CONFIG["CPP"] = "g++ -E -std=gnu++11"
$INCFLAGS << " -I./NuDB/include"

have_library("boost_thread")
have_library("boost_system")

abort "Can't find header or library of NuDB" unless have_header('nudb/nudb.hpp')

create_header
create_makefile("rudb")
