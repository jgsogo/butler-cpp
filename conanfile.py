
from conans import ConanFile, CMake
from conans.errors import ConanInvalidConfiguration


class Butler(ConanFile):
    name = "butler"
    version = "0.1"

    url = "https://github.com/jgsogo/butler-cpp"
    description = "Common tasks for a butler (C++)"
    license = "MIT"

    settings = "os", "arch", "compiler", "build_type"
    default_options = {"tgbot_cpp:shared": False,}
    scm = {"type": "git", "url": "https://github.com/jgsogo/butler-cpp.git", "revision": "auto"}
    generators = "cmake_find_package"

    def configure(self):
        cppstd = self.settings.get_safe("cppstd")
        compiler_cppstd = self.settings.get_safe("compiler.cppstd")
        cppstd = compiler_cppstd or cppstd
        if cppstd:
            cppstd = int(cppstd[3:]) if 'gnu' in cppstd else int(cppstd)
            if cppstd == 98 or cppstd < 17:
                raise ConanInvalidConfiguration("Requires C++ 14")
        else:
            raise RuntimeError("Provide a value for C++ standard")

    def requirements(self):
        self.requires("tgbot_cpp/1.1@jgsogo/stable")
        self.requires("fmt/5.2.1@bincrafters/stable")
        self.requires("spdlog/1.3.1@bincrafters/stable")
        self.requires("boost/1.69.0@conan/stable", override=True)  # Boost 1.68 fails in Mac/Mojave
        self.requires("libpqxx/6.4.4@bincrafters/stable")
        self.requires("libmpdclient/2.16@jgsogo/stable")

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
