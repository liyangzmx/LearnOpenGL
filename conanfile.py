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
