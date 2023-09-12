@echo off
conan install . -pr=my_mingw_profile --build missing -s build_type=Debug
premake5 vs2022