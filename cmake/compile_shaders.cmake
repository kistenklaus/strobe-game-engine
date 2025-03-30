# Function to compile GLSL shaders to SPIR-V
function(compile_shader_shaderc SHADER_FILE OUTPUT_FILE)
    # Read the shader file content
    file(READ ${SHADER_FILE} SHADER_CONTENT)

    # Determine shader stage based on pragma
    if(SHADER_CONTENT MATCHES "#pragma shader vertex")
        set(SHADER_STAGE "vertex")
    elseif(SHADER_CONTENT MATCHES "#pragma shader fragment")
        set(SHADER_STAGE "fragment")
    else()
        message(WARNING "Shader ${SHADER_FILE} does not contain a valid pragma directive.")
        return()
    endif()

    # Call glslc to compile the shader with the proper stage
    execute_process(
        COMMAND glslc -fshader-stage=${SHADER_STAGE} ${SHADER_FILE} -o ${OUTPUT_FILE}
        RESULT_VARIABLE result
        ERROR_VARIABLE result_error
        OUTPUT_VARIABLE result_output
    )

    # Check for errors during compilation
    if(result)
        message(FATAL_ERROR "Error compiling shader ${SHADER_FILE}: ${result_error}\n${result_output}")
    else()
        message(STATUS "Successfully compiled shader ${SHADER_FILE} to ${OUTPUT_FILE}")
    endif()
endfunction()

# Compile all shaders in the resources/shaders directory
file(GLOB SHADER_FILES "resources/shaders/*.spirv.glsl")

foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    set(OUTPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/${SHADER_NAME}.spirv.bin")

    # Call the function to compile each shader
    compile_shader_shaderc(${SHADER} ${OUTPUT_FILE})
endforeach()
