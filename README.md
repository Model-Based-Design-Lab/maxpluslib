# MaxPlusLib

A library of max-plus algebra related algorithms.
It was originally developed to support the analysis of timed dataflow models in the [SDF3](http://www.es.ele.tue.nl/sdf3) library.



Note that there is additionally a Python library that supports a different set of max-plus algebra related algorithms:
- <https://github.com/Model-Based-Design-Lab/cmlib>

## Getting started

The library can be built with cmake on a Linux platform as follows.

``` bash
cmake .
make
```

This creates a static library that can be used with the provided include files.

Use the following to run the tests and coverage results.

``` bash
cmake -DCODE_COVERAGE=ON -DBUILD_TESTS=ON.
ctest
make maxpluslibcoverage
```

The documentation can be built with the following command.

``` bash
make documentation
```

### Build wih Visual Studio

In Visual Studio, make sure that the `C++ CMake tools for Windows` option is installed as part of the `Desktop development with C++` modules of Visual Studio.
Start VS and select `Open a local folder` and select the main folder of the repository (with the top level `CMakeLists.txt` file).
Select `Project->Configure Cache` and then `Build->Build All`.

### Build with docker

Create a docker image from the provided `Dockerfile`

``` bash
docker build . -t maxpluslib
```

Wait for the construction of the image to complete.
The static library, include files, test and test coverage results and documentation reside in mounted volumes in the image.
The results can be copied from the image to the host by creating a container from the image and copying the volumes to the host as needed.

``` bash
docker cp CONTAINER:/maxpluslib/ <host path>
docker cp CONTAINER:/maxpluslibtest/ <host path>
docker cp CONTAINER:/maxpluslibdoc/ <host path>
docker cp CONTAINER:/maxpluslibcoverage/ <host path>
```


## Authors and acknowledgment

This library contains contributions by the following authors.

- Marc Geilen (m.c.w.geilen@tue.nl)
- Bram van der Sanden
- Sander Stuijk
- Peter Poplavko
- Bart Theelen
- Alireza Mohammadkhani

## Documentation

To make documentation, install doxygen and graphviz and run the following command.

``` bash
make doc
```

Documentation is built in `documentation/hmtml`.

## Contact Information

For questions regarding the library, contact Marc Geilen (m.c.w.geilen@tue.nl)


## License

This software is licensed under the MIT License

