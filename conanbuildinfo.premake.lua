#!lua
conan_build_type = "Debug"
conan_arch = "x86_64"

conan_includedirs = {"C:/Users/Lenovo/.conan/data/cpp-httplib/0.10.3/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include",
"C:/Users/Lenovo/.conan/data/cpp-httplib/0.10.3/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include/httplib",
"C:/Users/Lenovo/.conan/data/nlohmann_json/3.11.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include"}
conan_libdirs = {}
conan_bindirs = {}
conan_libs = {}
conan_system_libs = {"crypt32", "cryptui", "ws2_32"}
conan_defines = {}
conan_cxxflags = {}
conan_cflags = {}
conan_sharedlinkflags = {}
conan_exelinkflags = {}
conan_frameworks = {}

conan_includedirs_cpp_httplib = {"C:/Users/Lenovo/.conan/data/cpp-httplib/0.10.3/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include",
"C:/Users/Lenovo/.conan/data/cpp-httplib/0.10.3/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include/httplib"}
conan_libdirs_cpp_httplib = {}
conan_bindirs_cpp_httplib = {}
conan_libs_cpp_httplib = {}
conan_system_libs_cpp_httplib = {"crypt32", "cryptui", "ws2_32"}
conan_defines_cpp_httplib = {}
conan_cxxflags_cpp_httplib = {}
conan_cflags_cpp_httplib = {}
conan_sharedlinkflags_cpp_httplib = {}
conan_exelinkflags_cpp_httplib = {}
conan_frameworks_cpp_httplib = {}
conan_rootpath_cpp_httplib = "C:/Users/Lenovo/.conan/data/cpp-httplib/0.10.3/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9"

conan_includedirs_nlohmann_json = {"C:/Users/Lenovo/.conan/data/nlohmann_json/3.11.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include"}
conan_libdirs_nlohmann_json = {}
conan_bindirs_nlohmann_json = {}
conan_libs_nlohmann_json = {}
conan_system_libs_nlohmann_json = {}
conan_defines_nlohmann_json = {}
conan_cxxflags_nlohmann_json = {}
conan_cflags_nlohmann_json = {}
conan_sharedlinkflags_nlohmann_json = {}
conan_exelinkflags_nlohmann_json = {}
conan_frameworks_nlohmann_json = {}
conan_rootpath_nlohmann_json = "C:/Users/Lenovo/.conan/data/nlohmann_json/3.11.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9"

function conan_basic_setup()
    configurations{conan_build_type}
    architecture(conan_arch)
    includedirs{conan_includedirs}
    libdirs{conan_libdirs}
    links{conan_libs}
    links{conan_system_libs}
    links{conan_frameworks}
    defines{conan_defines}
    bindirs{conan_bindirs}
end
