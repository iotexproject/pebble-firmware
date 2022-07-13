if(CONFIG_MIPI_SYST_LIB)
zephyr_interface_library_named(mipi_syst_library)

set(SYST_CFG_VERSION_MAJOR "1" CACHE STRING
	"Supported MIPI SyS-T major specification version.")
set(SYST_CFG_VERSION_MINOR "0" CACHE STRING
	"Supported MIPI SyS-T minor specification version.")
set(SYST_CFG_VERSION_PATCH "0" CACHE PATH
	"SyS-T Library patch level.")
set(SYST_CFG_CONFORMANCE_LEVEL "30" CACHE STRING
	"Supported MIPI SyS-T API conformance level.
	(10=min, 20=low overhead, 30=complete)")

configure_file(library/include/mipi_syst.h.in
		library/include/mipi_syst.h)

zephyr_include_directories(
	mipi_syst_library INTERFACE
	${CMAKE_CURRENT_BINARY_DIR}/library/include/
	library/include/
	include/
	)

zephyr_library()

zephyr_library_sources(library/src/mipi_syst_api.c)
zephyr_library_sources(library/src/mipi_syst_compiler.c)
zephyr_library_sources(library/src/mipi_syst_crc32.c)
zephyr_library_sources(library/src/mipi_syst_init.c)
zephyr_library_sources(library/src/mipi_syst_inline.c)
zephyr_library_sources(library/src/mipi_syst_writer.c)

zephyr_library_link_libraries(mipi_syst_library)

target_link_libraries(mipi_syst_library INTERFACE zephyr_interface)
endif()
