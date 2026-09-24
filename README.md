# learnopengl.com code repository
Contains code samples for all chapters of Learn OpenGL and [https://learnopengl.com](https://learnopengl.com). 

中文完整构建记录与排障说明：[Learn OpenGL with Conan2](CONAN2_README.md)。

## Chapter 1 with CMake and Conan 2

This builds the 24 runnable examples registered in CMake for `1.getting_started`.
GLM, GLAD and stb_image are included in this repository. OpenGL and the native
window-system SDK come from the operating system; Assimp and FreeType are not
needed for this chapter.

If GLFW is already installed with a CMake config package, reuse it:

```sh
# Run `conan profile detect` first if you do not already have a default profile.
conan install . -of build/conan -s build_type=Debug -o '&:system_glfw=True' --build=missing
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DLEARNOPENGL_GETTING_STARTED_ONLY=ON
cmake --build build --parallel
cd bin/1.getting_started
./1.getting_started__1.1.hello_window
```

If GLFW is missing, omit `-o '&:system_glfw=True'` from `conan install`.
The recipe then obtains [glfw/3.4 from ConanCenter](https://conan.io/center/recipes/glfw).
Use a separate build directory when switching dependency providers.
Ninja must be installed for `-G Ninja`; alternatively omit that argument to use
the default CMake generator in a fresh build directory.

Run examples from `bin/1.getting_started` so their shader files can be found.
For example, `./1.getting_started__2.1.hello_triangle` displays an orange triangle;
press Escape or close the window to exit. The first `hello_window` example only
creates a window and does not draw a scene.
The Conan recipe currently covers chapters 1–7; the legacy full-project build
instructions below require additional dependencies.

## Chapter 2 (Lighting)

Chapter 2 uses the same dependencies as chapter 1. Build its 13 complete programs
in a separate directory:

```sh
conan install . -of build/chapter2/conan -s build_type=Debug -o '&:system_glfw=True' -o '&:chapter=2' --build=missing
cmake -S . -B build/chapter2 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build/chapter2 --parallel
cd bin/2.lighting
./2.lighting__6.multiple_lights
```

Omit `-o '&:system_glfw=True'` if GLFW must be obtained from ConanCenter.
Run from `bin/2.lighting` to load the shaders. Click the window to focus it,
then press Escape to exit; Ctrl+C in the launching terminal also stops it.
The five remaining exercise `.cpp` files contain incomplete code or GLSL snippets,
so they are not standalone executable targets.

## Chapter 3 (Model Loading)

Chapter 3 has one complete program, which loads the included backpack model and
textures. It additionally requires Assimp; Conan obtains `assimp/6.0.5` from
[ConanCenter](https://conan.io/center/recipes/assimp).

```sh
conan install . -of build/chapter3/conan -s build_type=Release -o '&:system_glfw=True' -o '&:chapter=3' -c 'tools.cmake:configure_args=["-DCMAKE_POLICY_VERSION_MINIMUM=3.5"]' --build=missing
cmake -S . -B build/chapter3 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/chapter3 --parallel
cd bin/3.model_loading
./3.model_loading__1.model_loading
```

The policy setting allows older transitive dependencies to build with CMake 4.
Omit `-o '&:system_glfw=True'` if GLFW is missing. Run from the executable's
directory to find its shaders. Use WASD and the mouse to move the camera,
the scroll wheel to zoom, and Escape to exit (or Ctrl+C in the terminal).

## Chapter 4 (Advanced OpenGL)

Build the 18 complete programs covering depth/stencil testing, blending,
framebuffers, cubemaps, uniform buffers, geometry shaders, instancing and MSAA.
Dependencies are the same as chapter 3 (GLFW and Assimp).

```sh
conan install . -of build/chapter4/conan -s build_type=Release -o '&:system_glfw=True' -o '&:chapter=4' -c 'tools.cmake:configure_args=["-DCMAKE_POLICY_VERSION_MINIMUM=3.5"]' --build=missing
cmake -S . -B build/chapter4 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/chapter4 --parallel
cd bin/4.advanced_opengl
./4.advanced_opengl__10.3.asteroids_instanced
```

Omit `-o '&:system_glfw=True'` if GLFW is missing. Launch examples from
`bin/4.advanced_opengl` so shaders can be found. Press Escape in the focused
window or Ctrl+C in the launching terminal to exit. The face-culling exercise
is a code fragment and is not an executable target.

## Chapter 5 (Advanced Lighting)

Build the 16 registered examples covering Blinn-Phong lighting, gamma correction,
shadow mapping, point shadows, normal/parallax mapping, HDR, bloom, deferred
shading and SSAO. GLFW and Assimp are reused from the previous chapters.

```sh
conan install . -of build/chapter5/conan -s build_type=Release -o '&:system_glfw=True' -o '&:chapter=5' -c 'tools.cmake:configure_args=["-DCMAKE_POLICY_VERSION_MINIMUM=3.5"]' --build=missing
cmake -S . -B build/chapter5 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/chapter5 --parallel
cd bin/5.advanced_lighting
./5.advanced_lighting__7.bloom
```

Omit `-o '&:system_glfw=True'` if GLFW is missing. Run from the executable's
folder to find its shaders. Escape exits; Ctrl+C in the terminal also stops it.
The legacy `3.3.csm` source is not registered in the original CMake build; it
uses old GLEW/SOIL APIs and references absent shader files, so it is excluded.

## Chapter 6 (PBR)

Build all six examples covering direct PBR lighting, material textures, HDR
cubemap conversion, diffuse irradiance and specular image-based lighting (IBL).
The included PBR textures and `newport_loft.hdr` environment map are required.

```sh
conan install . -of build/chapter6/conan -s build_type=Release -o '&:system_glfw=True' -o '&:chapter=6' -c 'tools.cmake:configure_args=["-DCMAKE_POLICY_VERSION_MINIMUM=3.5"]' --build=missing
cmake -S . -B build/chapter6 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/chapter6 --parallel
cd bin/6.pbr
./6.pbr__2.2.2.ibl_specular_textured
```

Omit `-o '&:system_glfw=True'` if GLFW is missing. Launch from `bin/6.pbr`
so shaders can be found. IBL examples precompute lighting maps at startup;
allow this to finish before interacting. Use WASD, the mouse and scroll wheel
for the camera; Escape exits, or use Ctrl+C in the launching terminal.

## Chapter 7 (In Practice)

Build debugging, FreeType text rendering and the complete Breakout game. This
build enables Breakout (disabled by the original CMake setup) and uses
[miniaudio from ConanCenter](https://conan.io/center/recipes/miniaudio) for its
background music and sound effects instead of the bundled irrKlang headers.

```sh
conan install . -of build/chapter7/conan -s build_type=Release -o '&:system_glfw=True' -o '&:system_freetype=True' -o '&:chapter=7' --build=missing
cmake -S . -B build/chapter7 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/chapter7 --parallel
cd bin/7.in_practice
./7.in_practice__3.breakout
```

Omit `-o '&:system_glfw=True'` or `-o '&:system_freetype=True'` when the respective
system library is missing; Conan then supplies it. Run from `bin/7.in_practice`
so the shaders are found. Other targets are `7.in_practice__1.debugging` and
`7.in_practice__2.text_rendering`.

Breakout controls: Enter starts, W/S select a level in the menu, A/D move the
paddle, Space launches the ball, Escape exits. Ctrl+C also stops a terminal run.
`./7.in_practice__3.breakout --smoke-test` reloads and validates all four levels,
starts gameplay and exits after six seconds, releasing audio and GPU resources.

The debugging lesson intentionally contains incorrect OpenGL calls (including a
wrong texture target and uniforms set before activating the shader). They are
preserved as teaching exercises; a running window does not mean its image is
correct. Debug callback availability depends on the OpenGL driver.

## Windows building
All relevant libraries are found in /libs and all DLLs found in /dlls (pre-)compiled for Windows. 
The CMake script knows where to find the libraries so just run CMake script and generate project of choice.

Keep in mind the supplied libraries were generated with a specific compiler version which may or may not work on your system (generating a large batch of link errors). In that case it's advised to build the libraries yourself from the source.

## Linux building
First make sure you have CMake, Git, and GCC by typing as root (sudo) `apt-get install g++ cmake git` and then get the required packages:
Using root (sudo) and type `apt-get install libsoil-dev libglm-dev libassimp-dev libglew-dev libglfw3-dev libxinerama-dev libxcursor-dev  libxi-dev libfreetype-dev libgl1-mesa-dev xorg-dev` .

**Build through CMake-gui:** The source directory is LearnOpenGL and specify the build directory as LearnOpenGL/build. Creating the build directory within LearnOpenGL is important for linking to the resource files (it also will be ignored by Git). Hit configure and specify your compiler files (Unix Makefiles are recommended), resolve any missing directories or libraries, and then hit generate. Navigate to the build directory (`cd LearnOpenGL/build`) and type `make` in the terminal. This should generate the executables in the respective chapter folders.

**Build through Cmake command line:**
```
cd /path/to/LearnOpenGL
mkdir build && cd build
cmake ..
cmake --build .
```

Note that CodeBlocks or other IDEs may have issues running the programs due to problems finding the shader and resource files, however it should still be able to generate the executables. To work around this problem it is possible to set an environment variable to tell the tutorials where the resource files can be found. The environment variable is named LOGL_ROOT_PATH and may be set to the path to the root of the LearnOpenGL directory tree. For example:

    `export LOGL_ROOT_PATH=/home/user/tutorials/LearnOpenGL`

Running `ls $LOGL_ROOT_PATH` should list, among other things, this README file and the resources directory.

## Mac OS X building
Building on Mac OS X is fairly simple:
```
brew install cmake assimp glm glfw freetype
cmake -S . -B build
cmake --build build -j$(sysctl -n hw.logicalcpu)
```
## Create Xcode project on Mac platform
Thanks [@caochao](https://github.com/caochao):
After cloning the repo, go to the root path of the repo, and run the command below:
```
mkdir xcode
cd xcode
cmake -G Xcode ..
```

## Glitter
Polytonic created a project called [Glitter](https://github.com/Polytonic/Glitter) that is a dead-simple boilerplate for OpenGL. 
Everything you need to run a single LearnOpenGL Project (including all libraries) and just that; nothing more. 
Perfect if you want to follow along with the chapters, without the hassle of having to manually compile and link all third party libraries!

## Ports
Check out [@srcres258](https://github.com/srcres258)'s port in Rust [here](https://github.com/srcres258/learnopengl-rust/).
