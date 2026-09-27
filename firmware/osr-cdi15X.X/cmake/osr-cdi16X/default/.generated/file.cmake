# The following variables contains the files used by the different stages of the build process.
set(osr_cdi16X_default_default_XC8_FILE_TYPE_assemble)
set_source_files_properties(${osr_cdi16X_default_default_XC8_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${osr_cdi16X_default_default_XC8_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(osr_cdi16X_default_default_XC8_FILE_TYPE_assemblePreprocess)
set_source_files_properties(${osr_cdi16X_default_default_XC8_FILE_TYPE_assemblePreprocess} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${osr_cdi16X_default_default_XC8_FILE_TYPE_assemblePreprocess})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(osr_cdi16X_default_default_XC8_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/constant.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/pvs.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/qshifter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/usb_callbacks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/usb_descriptors.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/usb_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/usb_device_hid.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/userinterface.c")
set_source_files_properties(${osr_cdi16X_default_default_XC8_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(osr_cdi16X_default_default_XC8_FILE_TYPE_link)
set(osr_cdi16X_default_image_name "default.elf")
set(osr_cdi16X_default_image_base_name "default")

# The output directory of the final image.
set(osr_cdi16X_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/osr-cdi16X")

# The full path to the final image.
set(osr_cdi16X_default_full_path_to_image ${osr_cdi16X_default_output_dir}/${osr_cdi16X_default_image_name})

# Potential output file extensions
set(output_extensions
    .hex
    .hxl
    .mum
    .o
    .sdb
    .sym
    .cmf)
list(TRANSFORM output_extensions PREPEND "${osr_cdi16X_default_output_dir}/${osr_cdi16X_default_image_base_name}")
