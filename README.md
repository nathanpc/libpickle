# PickLE for C

A plain C89, single source/header, library for parsing and easily working with
[PickLE](https://github.com/nathanpc/pickle) pick list documents.

## Generating a Compilation Database

If you want to have your Visual Studio Code fully setup to work with this
project it's highly recommended that you setup a compilation database to work
with [SonarLint](https://www.sonarsource.com/products/sonarlint/) and other
tools. In order to automate things a bit you'll be required to have
[Bear](https://github.com/rizsotto/Bear) installed in your system. After that,
all that you need to do is run:

```bash
make compiledb
```

## License

This project is licensed under the [MIT License](/LICENSE).
