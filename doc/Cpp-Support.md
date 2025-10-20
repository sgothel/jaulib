# C++ Support

This summary has been updated on: 2025-10-04.

## C++ Features

### C++20
- gcc-11 (complete)
- clang-14 (most)
- clang-19 (complete)
- Satisfied by
  - FreeBSD 14.3 and above
  - Debian 13 'Trixie' and above
  - Ubuntu `24.04 LTS (Noble Numbat)` and above

See
- [cppreference's C++20](https://en.cppreference.com/w/cpp/compiler_support/20.html)
- [GCC C++20](https://gcc.gnu.org/projects/cxx-status.html#cxx20)
- [clang C++20](https://clang.llvm.org/cxx_status.html#cxx20)

### C++23
- gcc-14 (most)
- clang-19 (most)
- Satisfied by
  - FreeBSD 14.3 and above
  - Debian 13 'Trixie' and above
  - Ubuntu `24.04 LTS (Noble Numbat)` and above

See
- [cppreference's C++23](https://en.cppreference.com/w/cpp/compiler_support/23.html)
- [GCC C++23](https://gcc.gnu.org/projects/cxx-status.html#cxx23)
- [clang C++23](https://clang.llvm.org/cxx_status.html#cxx23)

### C++26
- gcc-16 ???
- clang-22 ???
- Satisfied by
  - None yet

See
- [cppreference's C++26](https://en.cppreference.com/w/cpp/compiler_support/26.html)
- [GCC C++26](https://gcc.gnu.org/projects/cxx-status.html#cxx26)
- [clang C++26](https://clang.llvm.org/cxx_status.html#cxx26)


## OS Default Compiler

### FreeBSD
- FreeBSD 14.3 (2025-06-10)
  - Default Software
    - clang 19.1.7 (llvm tools)
  - Highest C++ Version (mostly supported)
    - C++23

### Debian
- Debian 14 'Forky' (Testing)
  - Default Software
    - glibc 2.41-12
    - gcc 15.2
    - clang 19
  - Highest C++ Version (mostly supported)
    - C++23

- Debian 13 'Trixie' (Stable)
  - Default Software
    - glibc 2.41-12
    - gcc 14.2
    - clang 19
  - Highest C++ Version (mostly supported)
    - C++23

- Debian 12 'Bookworm' (Oldstable)
  - Default Software
    - glibc 2.36-1
    - gcc 12.2
    - clang 14
  - Highest C++ Version (mostly supported)
    - C++20

### Ubuntu

Note: **bold** – package is in main

[LLVM/GCC versions](https://documentation.ubuntu.com/ubuntu-for-developers/reference/availability/gcc/)
- 25.10 (Questing Quokka): 11, 12, 13, **14, 15**; **default 14**
- 25.04 (Plucky Puffin): 11, 12, 13, **14, 15**; **default 14**
- 24.10 (Oracular Oriole): 11, 12, 13, **14**; **default 14**
- 24.04 LTS (Noble Numbat): 11, 12, **13, 14**; **default 13**

[LLVM/Clang versions](https://documentation.ubuntu.com/ubuntu-for-developers/reference/availability/llvm/)
- 25.10 (Questing Quokka): 14, 15, 17, **18, 19, 20**, 21; **default 20**
- 25.04 (Plucky Puffin): 14, 15, 17, **18, 19, 20**; **default 20**
- 24.10 (Oracular Oriole): 14, 15, 16, 17, **18, 19**; **default 19**
- 24.04 LTS (Noble Numbat): 14, 15, 16, **17, 18**, 19; **default 18**

## Custom Compiler
Custom LLVM/clang Compiler
- [LLVM Releases](https://releases.llvm.org/)
- [LLVM Debian/Ubuntu](https://apt.llvm.org/)
  - `sudo curl -sS -o /etc/apt/trusted.gpg.d/apt.llvm.org.asc https://apt.llvm.org/llvm-snapshot.gpg.key`

