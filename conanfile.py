from conans import ConanFile, CMake, tools

class lizardConan(ConanFile):
    name = "lizard"
    version = "1.0.2"
    license = "Devolutions"
    author = "Devolutions CI ci@devolutions.net"
    url = "https://github.com/devolutions/lizard"
    description = "lizard aims at simplifying the task of decompressing a 7-Zip archive to a temporary location from which an executable will be launched."
    topics = ("7-zip", "lizard")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"

    def source(self):
        self.run("git clone https://github.com/Devolutions/lizard.git")
        self.run("cd lizard && git checkout master")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="lizard")
        cmake.build()

    def package(self):
        self.copy("include/lizard/*.h", dst="include/lizard", src="lizard", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["lizard"]

