# Path to the musl-gcc

MUSL_GCC := justfile_directory() + "/musl-build/bin/musl-gcc"

MUSL_INSTALL := justfile_directory() + "/musl-build" 

clean:
    make -C tyche-musl clean
    make -C tyche-redis clean
    make -C tyche-redis distclean

build-musl:
  cd tyche-musl && ./configure --prefix={{MUSL_INSTALL}} --exec-prefix={{MUSL_INSTALL}} --disable-shared --enable-debug
  make -C tyche-musl/ -j `nproc`  CFLAGS="-static -Os -Wl,-z,norelro"
  make -C tyche-musl/ install

build-redis-server:
  make -C tyche-redis/ CC={{MUSL_GCC}} CFLAGS="-static -Os -Wl,-z,norelro" LDFLAGS="-static -z norelro" USE_JEMALLOC=no redis-server -j `nproc`

refresh:
  @rm -rf musl-build
  @just clean
  @just build-musl
  @just build-redis-server
  @cp ./tyche-redis/src/redis-server redis-server

# ——————————————————————— Similar but for debugging ———————————————————————— 

MUSL_GCC_DBG := justfile_directory() + "/musl-build-dbg/bin/musl-gcc"

MUSL_INSTALL_DBG := justfile_directory() + "/musl-build-dbg" 

build-musl-dbg:
  cd tyche-musl && ./configure --prefix={{MUSL_INSTALL_DBG}} --exec-prefix={{MUSL_INSTALL_DBG}} --disable-shared --enable-debug
  make -C tyche-musl/ -j `nproc`  CFLAGS="-static -Os -Wl,-z,norelro -DTYCHE_DO_INIT=1"
  make -C tyche-musl/ install 

build-redis-server-dbg:
  make -C tyche-redis/ CC={{MUSL_GCC_DBG}} CFLAGS="-static -Os -Wl,-z,norelro" LDFLAGS="-static -z norelro" USE_JEMALLOC=no redis-server -j `nproc`

build-dbg:
  @rm -rf musl-build-dbg
  @just clean
  @just build-musl-dbg
  @just build-redis-server-dbg
  @cp ./tyche-redis/src/redis-server redis-server-dbg

build:
  @just refresh
  @just build-dbg
