# Esterv.Crypto.Vault

[TOC]

This repo implements a C++ class and QML Module for storing encrypted data as a vault. 
The vault is stored on the local storage of the user.
I have tested the library on Linux, Android, and the Browser.

## Dependencies

The repo depends on [Qt](https://doc.qt.io/) and [openssl](https://www.openssl.org/) libraries.
For the case of Android and Browser, openssl binaries will be downloaded from the releases of my [action](https://github.com/EddyTheCo/install-openssl-action).

## Configure, build, test, package ...
 
The project uses [CMake presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) as a way to share CMake configurations.
Refer to [cmake](https://cmake.org/cmake/help/latest/manual/cmake.1.html), [ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) and [cpack](https://cmake.org/cmake/help/latest/manual/cpack.1.html) documentation for more information on the use of presets.

## Adding the libraries to your CMake project 

```CMake
include(FetchContent)
FetchContent_Declare(
	EstervVault
	GIT_REPOSITORY https://github.com/EddyTheCo/Esterv.Crypto.Vault.git
	GIT_TAG vMAJOR.MINOR.PATCH 
	FIND_PACKAGE_ARGS MAJOR.MINOR CONFIG  
	)
FetchContent_MakeAvailable(EstervVault)

target_link_libraries(<target> <PRIVATE|PUBLIC|INTERFACE> Esterv::Vault)
```
If you want to use the QML module also add
```
target_link_libraries(<target> <PRIVATE|PUBLIC|INTERFACE> $<$<STREQUAL:$<TARGET_PROPERTY:Esterv::Vault,TYPE>,STATIC_LIBRARY>:Esterv::Vaultplugin>)
```

## API reference

You can read the [API reference](https://eddytheco.github.io/Esterv.Crypto.Vault/), or generate it yourself like
```
cmake --workflow --preset default-documentation
```

## Using the QML modules

One needs to  make available to the QML engine the different modules by setting the [QML import path](https://doc.qt.io/qt-6/qtqml-syntax-imports.html#qml-import-path).

1. In your main function `engine.addImportPath("qrc:/esterVtech.com/imports");` to use the resource file. 

2. Set the environment variable like `export QML_IMPORT_PATH=installDir/CMAKE_INSTALL_LIBDIR`  where `CMAKE_INSTALL_LIBDIR` is where `Esterv` folder was created.

## Examples

The [examples](examples) folder shows the use of the different custom types provided by the QML module.

One can also play with the types [here](https://eddytheco.github.io/qmlonline/?example_url=vault)


## Contributing

We appreciate any contribution!

You can open an issue or request a feature.
You can open a PR to the `develop` branch and the CI/CD will take care of the rest.
Make sure to acknowledge your work, and ideas when contributing.
