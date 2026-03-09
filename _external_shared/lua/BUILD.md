# clean build

get latest release: https://github.com/lua/lua

## windows build

...

## linux build

rm -rf _build && \
make clean || true && \
\
mkdir -p _build/release && \
\
make liblua.a \
MYCFLAGS="-DNDEBUG -fPIC" \
WARN="" \
SYSCFLAGS="" \
-j$(nproc) && \
\
cp liblua.a _build/release/ && \
\
make clean && \
\
mkdir -p _build/debug && \
\
make liblua.a \
MYCFLAGS="-g -fPIC" \
WARN="" \
SYSCFLAGS="" \
-j$(nproc) && \
\
cp liblua.a _build/debug/ && \
\
make clean && \
\
echo -e "\n##########\n\nFinished building into _build/release and _build/debug.\n\n##########\n"
