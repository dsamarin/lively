# Lively

Lively is an open-source application for live mixing and effects processing.


## Building

```sh
# Set up autotools
aclocal
automake --add-missing
autoconf

# Use build directory for compiling
mkdir build
cd build

# Standard idiom
# The default backend is the ALSA backend.
# It is installed as `lively_alsa`.
# See `../configure --help` for more backends.
../configure
make
sudo make install
```

## Inspiration

```
         +-----------+.
         | PLAN AHEA || d
         +===========+.
              | |
              | |
             ~~~~~

[Copyright © 2015 Devin Samarin]
```
