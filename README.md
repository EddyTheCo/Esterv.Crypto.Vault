# Esterv.Utils.Vault 

This repo implements a C++ class and QML Module for storing encrypted data as a vault. 
The vault is stored on the local storage of the user.
I have tested the library on Linux, Android, and the Browser.

## Dependencies

The repo depends on [Qt](https://doc.qt.io/) and [openssl](https://www.openssl.org/) libraries.
For the case of Android and Browser, openssl binaries will be downloaded from the releases of my [action](https://github.com/EddyTheCo/install-openssl-action).

## Installing the library 

### From source code
```
git clone https://github.com/EddyTheCo/QVault.git

mkdir build
cd build
qt-cmake -G Ninja -DCMAKE_INSTALL_PREFIX=installDir -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF -DQTDEPLOY=OFF -DUSE_QML=OFF -DBUILD_DOCS=OFF ../QVault

cmake --build . 

cmake --install . 
```
where `installDir` is the installation path, `QTDEPLOY` install the examples and Qt dependencies using the 
[cmake-deployment-api](https://www.qt.io/blog/cmake-deployment-api). Setting the `USE_QML` variable produce or not the QML module.
One can choose to build the examples and the documentation with the `BUILD_EXAMPLES` and `BUILD_DOCS` variables.

### From GitHub releases
Download the releases from this repo. 

## Adding the libraries to your CMake project 

```CMake
include(FetchContent)
FetchContent_Declare(
	QtVault	
	GIT_REPOSITORY https://github.com/EddyTheCo/QVault.git
	GIT_TAG v0.1.0 
	FIND_PACKAGE_ARGS 0.1 CONFIG  
	)
FetchContent_MakeAvailable(QtVault)

target_link_libraries(<target> <PRIVATE|PUBLIC|INTERFACE> QtVault::vault)
```
If you want to use the QML module also add
```
$<$<STREQUAL:$<TARGET_PROPERTY:QtVault::vault,TYPE>,STATIC_LIBRARY>:QtVault::vaultplugin>
```

## API reference

You can read the [API reference](https://eddytheco.github.io/QVault/), or generate it yourself like
```
cmake -DBUILD_DOCS=ON ../
cmake --build . --target doxygen_docs
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
