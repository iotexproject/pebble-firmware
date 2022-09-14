if(CONFIG_TINYCBOR)

zephyr_include_directories(../include)

zephyr_library()
zephyr_library_sources(
    ../src/cbor_buf_reader.c
    ../src/cbor_buf_writer.c
    ../src/cborencoder.c
    ../src/cborerrorstrings.c
    ../src/cborparser.c
)
zephyr_library_sources_ifdef(
    CONFIG_NEWLIB_LIBC ../src/cborparser_dup_string.c)

zephyr_library_sources_ifdef(
    CONFIG_CBOR_PRETTY_PRINTING ../src/cborpretty.c)

endif()
