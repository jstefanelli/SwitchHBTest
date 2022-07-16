# SwitchHBTest

Playground and experiments with LibNX-based Nintendo Switch homebrew

## Dependencies

The project is based on the wonderful **[DevkitProA64](https://devkitpro.org/wiki/Getting_Started)** and **[libnx](https://github.com/switchbrew/libnx)**. 

Follow the DevkitProA64 installation guide, then install the following required packages:

 - switch-dev
 - switch-glad
 - switch-glm
 - switch-libdrm_nouveau
 - switch-libpng

## Building

**Make sure that your `DEVKITPRO` environment variable is populated and correct.**

The project is handled by **CMake**, and is configured with the following command:

```
cmake -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/cmake/Switch.cmake"
```

Building the projects generates the `SwitchHBTest.nro` file in your build directory, you can copy that to a jailbroken switch and run it via **HBMenu**, or you can stream it to the console via **nxlink**

## "Features"

Currently, this project is an ugly tic-tac-toe game played against the an "AI".

Graphics are handled via **OpenGL** and the rest is handled via **libnx**.