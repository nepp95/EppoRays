# EppoRays

## Introduction

Ray Traced renderer made as a hobby.
Currently does 5 bounces and only works with spheres. Emissive materials are supported.

![Screenshot 2024-01-08 183140](https://github.com/nepp95/EppoRays/assets/4678993/3a790886-7889-4d20-b51b-b40750eb39c3)

## Build

*Currently only works with Windows and Visual Studio unless you make your own adjustments to the build script*

- Run `Scripts/Setup.bat` which will simply execute the included premake program. This will generate the project/solution files for Visual Studio.
- Open the solution and build in release mode - it works in debug of course, but since it is pretty compute intensive, you probably don't want to.
