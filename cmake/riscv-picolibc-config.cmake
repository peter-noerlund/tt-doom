if(NOT riscv-picolibc_FOUND)
    set(riscv-picolibc_FOUND OFF)

    find_file(RISCV_PICOLIBC NAMES picolibc.specs PATHS /usr/local/lib/picolibc/riscv64-unknown-elf /usr/lib/picolibc/riscv64-unknown-elf)
    if(NOT RISCV_PICOLIBC STREQUAL "RISCV_PICOLIBC-NOTFOUND")
        message(STATUS "Found picolibc for RISC-V: ${RISCV_PICOLIBC}")
        set(riscv-picolibc_FOUND ON)
    endif()
endif()
