#!lua
conan_build_type = "Debug"
conan_arch = "x86_64"

conan_includedirs = {"C:/Users/Lenovo/.conan/data/cpp-httplib/0.10.3/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include",
"C:/Users/Lenovo/.conan/data/cpp-httplib/0.10.3/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include/httplib",
"C:/Users/Lenovo/.conan/data/jsoncpp/1.9.5/_/_/package/2e8009f4998c3521e80ce5e585765e95fb01946f/include"}
conan_libdirs = {"C:/Users/Lenovo/.conan/data/jsoncpp/1.9.5/_/_/package/2e8009f4998c3521e80ce5e585765e95fb01946f/lib"}
conan_bindirs = {}
conan_libs = {"jsoncpp"}
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

conan_includedirs_jsoncpp = {"C:/Users/Lenovo/.conan/data/jsoncpp/1.9.5/_/_/package/2e8009f4998c3521e80ce5e585765e95fb01946f/include"}
conan_libdirs_jsoncpp = {"C:/Users/Lenovo/.conan/data/jsoncpp/1.9.5/_/_/package/2e8009f4998c3521e80ce5e585765e95fb01946f/lib"}
conan_bindirs_jsoncpp = {}
conan_libs_jsoncpp = {"jsoncpp"}
conan_system_libs_jsoncpp = {}
conan_defines_jsoncpp = {}
conan_cxxflags_jsoncpp = {}
conan_cflags_jsoncpp = {}
conan_sharedlinkflags_jsoncpp = {}
conan_exelinkflags_jsoncpp = {}
conan_frameworks_jsoncpp = {}
conan_rootpath_jsoncpp = "C:/Users/Lenovo/.conan/data/jsoncpp/1.9.5/_/_/package/2e8009f4998c3521e80ce5e585765e95fb01946f"

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
