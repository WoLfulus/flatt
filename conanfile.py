from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout


class Flatt(ConanFile):
    name = "flatt"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    options = {
        "sol2/*:with_lua": ["lua", "luajit"],
        "spdlog/*:shared": [True, False],
        "spdlog/*:no_exceptions": [True, False],
    }

    default_options = {
        "sol2/*:with_lua": "luajit",
        "spdlog/*:shared": False,
        "spdlog/*:no_exceptions": True,
    }

    # exports_sources = "src/*", "CMakeLists.txt"

    def requires(self):
        self.requires("argparse/3.1")
        self.requires("cryptopp/8.9.0")
        self.requires("flatbuffers/24.3.25")
        self.requires("inja/3.4.0")
        self.requires("nlohmann_json/3.11.3")
        self.requires("sol2/3.3.1")
        self.requires("spdlog/1.14.1")
        self.requires("entt/3.13.2")

    def layout(self):
        cmake_layout(self)
        self.folders.source = "src"
        self.folders.build = "build"

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_file = "ConanPresets.json"
        tc.generate()

        deps = CMake(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["flatt"]

    def build_requirements(self):
        pass
