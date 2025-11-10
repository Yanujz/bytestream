cmake -S . -B build \
  -DBYTESTREAM_ENABLE_COVERAGE=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
gcovr \
  -r . build \
  --object-directory build \
  --filter "$(pwd)/include" \
  --exclude '.*build/_deps/.*' \
  --exclude '.*tests/.*' \
  --exclude '.*examples/.*' \
  --html --html-details --html-self-contained \
  --gcov-executable "llvm-cov gcov" \
  -o mkdocs/docs/coverage/index.html
