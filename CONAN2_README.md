# Learn OpenGL with Conan2

实践记录 · 2026-09-24 · macOS Apple Silicon · CMake + Conan 2

面向已具备 C++ 和终端基础、希望运行 LearnOpenGL 各章示例的开发者。本文整理本次从第一章到第七章的构建、运行与修复经验，提供可复制命令和已验证的排障方法。

结果：七章共 81 个可执行目标完成编译和启动检查，包含补入构建的 Breakout 游戏。多数示例做了限时启动及日志检查；第七章调试示例保留教学错误，Breakout 另做了关卡重载、OpenGL 错误检查及 Retina 四象限像素验证。不能把这些结果理解为全部画面与交互已逐项验收。

## 适用环境与复现前提

| 项目 | 本次实测 |
| --- | --- |
| 操作系统 / 架构 | macOS 15.7.4 / Apple Silicon arm64 |
| 编译器 | Apple Clang 17.0.0；Conan 架构为 armv8 |
| 工具 | Conan 2.29.0、CMake 4.2.3、Ninja 1.12.1 |
| 源码目录 | /opt/github/LearnOpenGL |
| 中文教程目录 | /opt/github/LearnOpenGL-CN |
| Conan 远端 | conancenter → https://center2.conan.io |
| 源码基线 | 上游基线 a545a70；本 fork 包含本文介绍的改动 |

以下命令针对本 fork 中已完成适配的代码；示例路径 /opt/github/LearnOpenGL 请替换为你的克隆目录。上游原始仓库没有这份 conanfile.py，也没有逐章构建选项和 Breakout 修复；只克隆上游并照抄命令并不足以复现。Windows、Linux 和其他工具链未在本次验证。

- 保留 conanfile.py、修改后的 CMakeLists.txt，以及第七章相关源码修改；README.md 已记录各章命令。
- 本机已有默认 Conan profile。换机器时先检查 profile 与实际编译器、架构是否匹配，不要覆盖已有自定义配置。
- 从项目根目录执行安装和配置命令；运行示例时切换到对应 bin 子目录。
- 需要 Ninja 才能使用 -G Ninja；本次所有构建使用 Ninja。不同构建类型、工具链或依赖来源使用独立构建目录。

```bash
cd /opt/github/LearnOpenGL
conan --version
cmake --version
ninja --version
conan profile show
conan remote list
```

预期：工具可执行，profile 显示 apple-clang / armv8，conancenter 已启用。新机器若尚无默认 profile，可执行 conan profile detect 后重新检查；本次沿用已有 profile。

## 依赖策略：已有则复用，缺失则由 Conan 提供

| 依赖 | 本次来源 / 版本 | 用途 |
| --- | --- | --- |
| GLFW | 本机已有 3.4；配方备用 glfw/3.4 | 窗口、输入、OpenGL 上下文 |
| GLM / GLAD / stb_image | 仓库自带头文件或实现 | 数学、函数加载、纹理解码 |
| Assimp | ConanCenter：assimp/6.0.5 | 第 3–6 章构建配置；部分示例包含 model.h |
| FreeType | 系统 2.14.2；配方备用 freetype/2.14.3 | 第 7 章文字与游戏 |
| miniaudio | ConanCenter：miniaudio/0.11.22，header_only=True | Breakout 背景音乐与音效 |
| OpenGL / 原生窗口系统 | 操作系统及其 SDK | Conan 不替代本机图形驱动与系统 SDK |

system_glfw=True 与 system_freetype=True 是明确的依赖来源开关，不是自动探测器。确认系统有库后才传 True；缺少时省略对应参数，配方会声明 Conan 依赖。Assimp 在当前配方中由 Conan 固定提供；FreeType 的 Conan 分支和 GLFW 的 Conan 分支未在本次实机替换系统库验证。

使用 --build=missing：有匹配二进制时直接使用，没有时从配方构建。第三章首次安装 Assimp 时，若干传递依赖需要本地构建；后续章节复用缓存。固定包版本不等于完全锁定传递依赖，本次尚未生成 Conan lockfile。

## CMake 与 Conan 2 的接入方式

Conan 配方按 chapter=1…7 选择依赖，通过 CMakeDeps 生成依赖查找配置，通过 CMakeToolchain 传入工具链和章节开关。各章保留独立 build 目录，输出放在 bin/<章节>。

```python
from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain


class LearnOpenGLConan(ConanFile):
    """Dependencies for chapters 1–7; GLM, GLAD and stb_image are vendored."""

    settings = "os", "compiler", "build_type", "arch"
    options = {"system_glfw": [True, False], "chapter": [1, 2, 3, 4, 5, 6, 7], "system_freetype": [True, False]}
    default_options = {"system_glfw": False, "chapter": 1, "system_freetype": False, "miniaudio/*:header_only": True}

    def requirements(self):
        if not self.options.system_glfw:
            self.requires("glfw/3.4")
        if str(self.options.chapter) in ("3", "4", "5", "6"):
            self.requires("assimp/6.0.5")

        if str(self.options.chapter) == "7":
            self.requires("miniaudio/0.11.22")
            if not self.options.system_freetype:
                self.requires("freetype/2.14.3")

    def generate(self):
        CMakeDeps(self).generate()
        toolchain = CMakeToolchain(self)
        toolchain.presets_prefix = "chapter{}".format(self.options.chapter)
        toolchain.variables["LEARNOPENGL_GETTING_STARTED_ONLY"] = str(self.options.chapter) == "1"
        toolchain.variables["LEARNOPENGL_LIGHTING_ONLY"] = str(self.options.chapter) == "2"
        toolchain.variables["LEARNOPENGL_MODEL_LOADING_ONLY"] = str(self.options.chapter) == "3"
        toolchain.variables["LEARNOPENGL_ADVANCED_OPENGL_ONLY"] = str(self.options.chapter) == "4"
        toolchain.variables["LEARNOPENGL_ADVANCED_LIGHTING_ONLY"] = str(self.options.chapter) == "5"
        toolchain.variables["LEARNOPENGL_PBR_ONLY"] = str(self.options.chapter) == "6"
        toolchain.variables["LEARNOPENGL_IN_PRACTICE_ONLY"] = str(self.options.chapter) == "7"
        toolchain.generate()
```

CMake 的配套修改包含：最低版本提升到 3.15；校验一次只选择一个章节；只为所选章节查找所需依赖；通过 glfw、assimp::assimp、Freetype::Freetype、miniaudio::miniaudio 导入目标链接。第七章额外加入 3.2d_game/0.full_source，并排除重复的 stb_image.cpp 实现。

每章使用独立 presets_prefix，生成 chapter1-debug、chapter2-debug、chapter3-release 等预设，避免多个 Conan 输出目录被 CMakeUserPresets.json 引用后出现同名预设。生成文件、build/ 和 bin/ 均不应提交为源码。

## 逐章编译与运行

每个代码块均从项目根目录开始。前两章使用 Debug，第 3–7 章使用 Release；conan install 与 cmake 的构建类型必须一致。执行成功后应能看到对应可执行文件，运行时不出现资源或着色器加载失败。

| 章节 | 构建数量 | 运行检查 / 例外 |
| --- | --- | --- |
| 1 入门 | 24 | 逐个启动约 1.5 秒；hello_window 只创建窗口 |
| 2 光照 | 13 | 逐个运行约 3 秒；另 5 个练习是片段 |
| 3 模型加载 | 1 | 背包示例运行约 8 秒 |
| 4 高级 OpenGL | 18 | 逐个运行 3–8 秒；面剔除练习为片段 |
| 5 高级光照 | 16 | 逐个运行 3–8 秒；旧 3.3.csm 不在原 CMake 构建中 |
| 6 PBR | 6 | 基础示例约 3 秒；IBL 约 15 秒 |
| 7 实战 | 3 | 调试、文字、Breakout；游戏另有自动验证 |

### 第一章：入门

```bash
cd /opt/github/LearnOpenGL
conan install . -of build/conan -s build_type=Debug -o '&:system_glfw=True' --build=missing
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DLEARNOPENGL_GETTING_STARTED_ONLY=ON
cmake --build build --parallel
cd bin/1.getting_started
./1.getting_started__1.1.hello_window
```

### 第二章：光照

```bash
cd /opt/github/LearnOpenGL
conan install . -of build/chapter2/conan -s build_type=Debug -o '&:system_glfw=True' -o '&:chapter=2' --build=missing
cmake -S . -B build/chapter2 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build/chapter2 --parallel
cd bin/2.lighting
./2.lighting__6.multiple_lights
```

### 第三章：模型加载

```bash
cd /opt/github/LearnOpenGL
conan install . -of build/chapter3/conan -s build_type=Release -o '&:system_glfw=True' -o '&:chapter=3' -c 'tools.cmake:configure_args=["-DCMAKE_POLICY_VERSION_MINIMUM=3.5"]' --build=missing
cmake -S . -B build/chapter3 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/chapter3 --parallel
cd bin/3.model_loading
./3.model_loading__1.model_loading
```

### 第四章：高级 OpenGL

```bash
cd /opt/github/LearnOpenGL
conan install . -of build/chapter4/conan -s build_type=Release -o '&:system_glfw=True' -o '&:chapter=4' -c 'tools.cmake:configure_args=["-DCMAKE_POLICY_VERSION_MINIMUM=3.5"]' --build=missing
cmake -S . -B build/chapter4 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/chapter4 --parallel
cd bin/4.advanced_opengl
./4.advanced_opengl__10.3.asteroids_instanced
```

### 第五章：高级光照

```bash
cd /opt/github/LearnOpenGL
conan install . -of build/chapter5/conan -s build_type=Release -o '&:system_glfw=True' -o '&:chapter=5' -c 'tools.cmake:configure_args=["-DCMAKE_POLICY_VERSION_MINIMUM=3.5"]' --build=missing
cmake -S . -B build/chapter5 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/chapter5 --parallel
cd bin/5.advanced_lighting
./5.advanced_lighting__7.bloom
```

未编译旧版 3.3.csm：原项目未注册该目标，源码使用旧 GLEW/SOIL API，并引用不存在的 depth_testing 着色器；不能把它计入已验证示例。

### 第六章：PBR

```bash
cd /opt/github/LearnOpenGL
conan install . -of build/chapter6/conan -s build_type=Release -o '&:system_glfw=True' -o '&:chapter=6' -c 'tools.cmake:configure_args=["-DCMAKE_POLICY_VERSION_MINIMUM=3.5"]' --build=missing
cmake -S . -B build/chapter6 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/chapter6 --parallel
cd bin/6.pbr
./6.pbr__2.2.2.ibl_specular_textured
```

核对了 26 个 PBR / HDR 资源路径，文件均存在。IBL 示例在启动阶段预计算环境光照贴图，启动耗时不能直接当成卡死。

### 第七章：实战与 Breakout

```bash
cd /opt/github/LearnOpenGL
conan install . -of build/chapter7/conan -s build_type=Release -o '&:system_glfw=True' -o '&:system_freetype=True' -o '&:chapter=7' --build=missing
cmake -S . -B build/chapter7 -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/chapter7 --parallel
cd bin/7.in_practice
./7.in_practice__3.breakout
```

Enter 开始；W/S 在菜单选关；A/D 移动挡板；空格发球；Esc 退出。调试示例故意保留错误纹理目标和未激活 shader 就设置 uniform 等问题，其驱动警告属于教学内容。

## 运行路径与退出：两个最常见的误判

着色器文件通常通过相对路径读取，必须从 bin/<章节> 启动。模型、纹理和字体主要通过 FileSystem::getPath 定位项目资源，但这不能替代着色器所需的工作目录。IDE 的 Working Directory 也应指向相应 bin 子目录。

示例通常有持续渲染循环，不会自动结束。终端命令持续运行是正常行为。点击窗口获得焦点后按 Esc；也可关闭窗口，或在启动它的终端按 Ctrl+C。批量检查采用限时启动并清理子进程；Breakout 新增了自动退出模式。

## 已验证的构建问题与修复

### 旧 CMake 配置遇到 CMake 4

项目原配置要求 CMake 3.0，CMake 4 已移除过旧兼容策略；本项目最低版本改为 3.15。macOS 创建着色器软链接的 add_custom_command(TARGET) 还移除了不支持的 DEPENDS 参数，并补上路径引号和 VERBATIM。

Conan 构建 bzip2 等旧传递依赖时，同样触发兼容错误。成功的办法是在 CMake 配置命令行传入策略下限：

```bash
-c 'tools.cmake:configure_args=["-DCMAKE_POLICY_VERSION_MINIMUM=3.5"]'
```

本次曾尝试 tools.cmake.cmaketoolchain:extra_variables 注入同名变量，但未解决依赖在最开头 cmake_minimum_required 就报错的问题；最终使用 configure_args，第三至第六章命令已经包含它。

### Assimp 库与旧头文件混用

仓库 includes/assimp 是旧头文件，Conan 提供的是 6.0.5 库。如果头文件搜索仍优先命中仓库，可能发生 API 或 ABI 不匹配。修复不是只追加链接参数，而是让编译阶段也优先使用同一依赖包的头文件。

```cmake
find_package(assimp CONFIG REQUIRED)
target_link_libraries(${NAME} assimp::assimp)
set_target_properties(${NAME} PROPERTIES NO_SYSTEM_FROM_IMPORTED TRUE)
target_include_directories(${NAME} BEFORE PRIVATE
    $<TARGET_PROPERTY:assimp::assimp,INTERFACE_INCLUDE_DIRECTORIES>)
```

在第三章实际编译命令中已确认 Conan Assimp include 目录位于仓库 includes 之前。FreeType 也显式优先使用匹配的系统或 Conan 头文件目录。

### Breakout 从未启用源码到可运行程序

- 将完整源码目录加入第七章构建，目标名为 7.in_practice__3.breakout；排除游戏目录自带的 stb_image.cpp，复用公共实现。
- 用小型 AudioEngine 封装 miniaudio，替换 irrKlang 调用，支持循环音乐和可重叠的一次性音效；实际听感未进行人工验收。
- ResourceManager 的 GetShader / GetTexture 返回已缓存对象的引用，修复临时值无法绑定非 const 引用的编译错误。
- FreeType advance.x 转为 unsigned int 时增加显式转换，修复 Clang 列表初始化的窄化报错。
- 关卡重置统一使用 FileSystem::getPath("resources/levels/…")，修复从 bin 目录运行时重置读不到关卡。
- 先创建 OpenGL 上下文，再初始化游戏；关闭时先释放游戏和音频，再销毁上下文。

### 文字中的空格触发纹理警告

FreeType 的空格有字距但没有位图。旧代码仍绘制其零尺寸纹理，引发 macOS “texture unloadable” 警告。文字示例和游戏文字渲染器均改为：位图宽或高为零时只推进光标，不采样纹理。复测后两者该警告消失。

## 关键复盘：Retina 只显示左下 1/4

用户实际看到 Breakout 只占左下角。窗口逻辑尺寸为 800×600，实测帧缓冲为 1600×1200；旧代码用逻辑尺寸调用 glViewport，宽和高各只覆盖一半，面积就是 1/4。这种问题不会必然产生 OpenGL 错误，之前的“启动正常、日志无错误”检查没有发现它。

修复必须区分两个渲染目标：屏幕默认帧缓冲使用实际像素尺寸；游戏离屏帧缓冲仍为 800×600。不能仅把启动时的 glViewport 改大，否则离屏阶段仍可能被错误裁切。

```cpp
int framebufferWidth, framebufferHeight;
glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
glViewport(0, 0, framebufferWidth, framebufferHeight);
```

每帧查询实际尺寸；最小化造成尺寸为零时跳过渲染。PostProcessor::BeginRender 保存屏幕 viewport 后切到离屏目标尺寸；EndRender 绑定默认帧缓冲并恢复屏幕 viewport：

```cpp
glGetIntegerv(GL_VIEWPORT, ScreenViewport);
glBindFramebuffer(GL_FRAMEBUFFER, MSFBO);
glViewport(0, 0, Width, Height);
// 绘制并 resolve 离屏图像后
glBindFramebuffer(GL_FRAMEBUFFER, 0);
glViewport(ScreenViewport[0], ScreenViewport[1],
           ScreenViewport[2], ScreenViewport[3]);
```

修复后重新编译，自动检查读取真实后缓冲区四个象限的像素，并核对 viewport；本机输出如下。该检查证明四个象限都有非黑像素，不代替整幅画面和所有特效的人工验收。

```text
Framebuffer 1600x1200: viewport and four-quadrant check passed
```

## 验证方法、结果与复现边界

- 构建：各章编译和链接通过，共 81 个可执行目标。
- 启动检查：逐个限时运行，检查提前退出、资源加载失败、shader 编译或链接错误，并回收进程。第七章调试程序的故意错误单独记录。
- 游戏回归：--smoke-test 重载四关、自动进入游戏并发球、检查 OpenGL 错误，约 6 秒后正常退出；另检查普通菜单启动。
- Retina 回归：实测 1600×1200 下视口与四象限读回检查通过。
- 未覆盖：各章所有视觉细节、全部用户交互、长时间稳定性、其他操作系统和显示器组合。

```bash
cd /opt/github/LearnOpenGL/bin/7.in_practice
./7.in_practice__3.breakout --smoke-test
```

正常结果包含四关重载成功、viewport and four-quadrant check passed，退出码为 0。构建及运行日志位于 build/chapterN/ 下，部分早期章节使用 build/ 根目录；参考 runtime-smoke.log、runtime-logs/、build.log。Retina 最后一次验证结果来自本次终端输出，早先的第七章汇总日志不包含后来新增的像素检查。

复用这套经验时，按“依赖能找到 → 编译能通过 → 资源能加载 → 画面覆盖正确 → 输入退出正常”逐层验收。不要仅以进程仍在运行判断渲染正确，也不要把教学故意制造的错误直接消掉。

## 资料与维护入口

维护入口：本仓库 [README.md](README.md)、[conanfile.py](conanfile.py)、[CMakeLists.txt](CMakeLists.txt)；第七章修复位于 src/7.in_practice/2.text_rendering 和 src/7.in_practice/3.2d_game/0.full_source。本文记录的是本次已验证工作区状态，不是上游默认构建说明。

[LearnOpenGL 中文教程](https://learnopengl-cn.github.io)

[GLFW 包](https://conan.io/center/recipes/glfw)

[Assimp 包](https://conan.io/center/recipes/assimp)

[FreeType 包](https://conan.io/center/recipes/freetype)

[miniaudio 包](https://conan.io/center/recipes/miniaudio)
