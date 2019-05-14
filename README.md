### RuDB - Ruby NuDB wrapper

RuDB provides a native wrapper around the [NuDB](https://github.com/CPPAlliance/NuDB) database. As NuDB is a *header only* library, this gem includes all the necessary logic builtin to manage a NuDB instance. As per the NuDB requirements, the following are necessary to install this gem:

* Boost 1.61 or higher (development headers)
* C++11 or greater

#### Installation / Documentation

rudb may be installed with the following command:

```ruby
$ gem install rrudb
```

(Yes, there are two '**r**'s in there!)

#### Usage

See **examples/example.rb** for a Ruby port of **NuDB/examples/example.cpp**

#### License

Copyright (C) 2019 Dev Null Productions

Made available under the MIT License
