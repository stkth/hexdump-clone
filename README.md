# hexdump-clone

A fairly leightweight pure ``C`` simple hexdump tool, intended to refresh my C knowledge.

## Help

```
./hexdump-clone -h
```

## Build the program

Build the program with the following command in a release setup:

```
cmake -DCMAKE_BUILD_TYPE=Release -B build-release/
```

Alternatively as debug build:

```
cmake -DCMAKE_BUILD_TYPE=Debug  -B build-debug/
```


## Additional resources

- [cmake](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [gitlint](https://joe.gl/ombek/blog/pr-gitlint/)
- [clang-format]([https://clang.llvm.org/docs/ClangFormat.html)

## TODO

- Refresh knowledge for longopts
- Allow different grouping of output like bytes, halfword, doubleword and similiar
- Move source code into source directory
- Extend testing to ubuntu and fedora as well
