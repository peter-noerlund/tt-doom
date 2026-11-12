if(NOT riscv-compiler_FOUND)
    set(riscv-compiler_FOUND OFF)

    find_program(RISCV_GCC NAMES riscv64-unknown-elf-gcc HINTS ${RISCV_GCC_PATH})
    if(NOT RISCV_GCC STREQUAL "RISCV_GCC-NOTFOUND")
        message(STATUS "Found gcc for RISC-V: ${RISCV_GCC}")
    endif()

    find_program(RISCV_OBJCOPY NAMES riscv64-unknown-elf-objcopy HINTS ${RISCV_OBJCOPY_PATH})
    if(NOT RISCV_OBJCOPY STREQUAL "RISCV_OBJCOPY-NOTFOUND")
        message(STATUS "Found objcopy for RISC-V: ${RISCV_OBJCOPY}")
    endif()

    if(NOT RISCV_GCC STREQUAL "RISCV_GCC-NOTFOUND" AND NOT RISCV_OBJCOPY STREQUAL "RISCV_OBJCOPY-NOTFOUND")
        set(riscv-compiler_FOUND ON)
    endif()
endif()
