include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(osr_cdi16X_default_library_list )

# Handle files with suffix (s|as|asm|AS|ASM|As|aS|Asm), for group default-XC8
if(osr_cdi16X_default_default_XC8_FILE_TYPE_assemble)
add_library(osr_cdi16X_default_default_XC8_assemble OBJECT ${osr_cdi16X_default_default_XC8_FILE_TYPE_assemble})
    osr_cdi16X_default_default_XC8_assemble_rule(osr_cdi16X_default_default_XC8_assemble)
    list(APPEND osr_cdi16X_default_library_list "$<TARGET_OBJECTS:osr_cdi16X_default_default_XC8_assemble>")

endif()

# Handle files with suffix S, for group default-XC8
if(osr_cdi16X_default_default_XC8_FILE_TYPE_assemblePreprocess)
add_library(osr_cdi16X_default_default_XC8_assemblePreprocess OBJECT ${osr_cdi16X_default_default_XC8_FILE_TYPE_assemblePreprocess})
    osr_cdi16X_default_default_XC8_assemblePreprocess_rule(osr_cdi16X_default_default_XC8_assemblePreprocess)
    list(APPEND osr_cdi16X_default_library_list "$<TARGET_OBJECTS:osr_cdi16X_default_default_XC8_assemblePreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC8
if(osr_cdi16X_default_default_XC8_FILE_TYPE_compile)
add_library(osr_cdi16X_default_default_XC8_compile OBJECT ${osr_cdi16X_default_default_XC8_FILE_TYPE_compile})
    osr_cdi16X_default_default_XC8_compile_rule(osr_cdi16X_default_default_XC8_compile)
    list(APPEND osr_cdi16X_default_library_list "$<TARGET_OBJECTS:osr_cdi16X_default_default_XC8_compile>")

endif()


# Main target for this project
add_executable(osr_cdi16X_default_image_1V1mL2MX ${osr_cdi16X_default_library_list})

set_target_properties(osr_cdi16X_default_image_1V1mL2MX PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    ADDITIONAL_CLEAN_FILES "${output_extensions}"
    RUNTIME_OUTPUT_DIRECTORY "${osr_cdi16X_default_output_dir}")
target_link_libraries(osr_cdi16X_default_image_1V1mL2MX PRIVATE ${osr_cdi16X_default_default_XC8_FILE_TYPE_link})
# Add the link options from the rule file.
osr_cdi16X_default_link_rule( osr_cdi16X_default_image_1V1mL2MX)



