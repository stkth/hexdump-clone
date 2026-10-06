# hexdump-clone

A fairly leightweight pure ``C`` simple hexdump tool, intended to refresh my C knowledge.

## Build the program

Build the program with the following command in a release setup:

```
cmake -DCMAKE_BUILD_TYPE=Release -B build-release/
```

Alternatively as debug build:

```
cmake -DCMAKE_BUILD_TYPE=Debug  -B build-debug/
```

## Help

```
./hexdump-clone -h
```

## C23

The project setup enforces C23 ruleset.

## Additional resources

- [cmake](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [gitlint](https://joe.gl/ombek/blog/pr-gitlint/)
- [clang-format](https://clang.llvm.org/docs/ClangFormat.html)

## TODO

- Refresh knowledge for longopts
- Allow different grouping of output like bytes, halfword, doubleword and similiar
- Move source code into source directory
- Extend testing to ubuntu and fedora as well
