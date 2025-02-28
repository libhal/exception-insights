from conan import ConanFile
from conan.errors import ConanException
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy
from conan.tools.build import check_min_cppstd
import os
from traceback import format_exception


required_conan_version = ">=2.0.14"

class safe_conan(ConanFile):
    name = "safe"
    license = "Apache-2.0"
    description = ("A collection of interfaces and abstractions for embedded "
                   "peripherals and devices using modern C++")
    topics = ("toolchain", "developer-experience")
    settings = "compiler", "build_type", "os", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualBuildEnv"

    @property
    def _min_cppstd(self) -> str:
        return "23"

    """
    Requirements for the application
    format: `self.requires(<package_name_on_conancenter>)`
    """
    def requirements(self):
        pass

    def generate(self):
        c = CMake(self)
        c.configure()
        c.build(target="copy_compile_commands")
        
    """
    Manage requirements to build our project, this includes all build tools.
    """
    def build_requirements(self):
        self.tool_requires("cmake/[^3.27.1]")

    """
    Manages the layout of where build artifacts will go.
    """
    def layout(self):
        cmake_layout(self)
    
    """
    Method to build the project
    """
    def build(self):
        print(f"Compiler: {self.settings.compiler}")
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        
        
    