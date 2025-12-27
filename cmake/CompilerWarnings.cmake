# CompilerWarnings.cmake
# Helper function to add common warning flags to targets

function(add_compiler_warnings target)
    target_compile_options(${target} PRIVATE
        # Core warnings
        -Wall
        -Wextra
        -Wpedantic
        
        # Useful additional warnings
        -Wshadow                    # Warn if a variable shadows another
        -Wnon-virtual-dtor          # Warn if a class with virtual functions has a non-virtual destructor
        -Wold-style-cast            # Warn for C-style casts
        -Wcast-align                # Warn for potential performance problem casts
        -Wunused                    # Warn on anything being unused
        -Woverloaded-virtual        # Warn if you overload (not override) a virtual function
        -Wconversion                # Warn on type conversions that may lose data
        -Wsign-conversion           # Warn on sign conversions
        -Wnull-dereference          # Warn if a null dereference is detected
        -Wdouble-promotion          # Warn if float is implicit promoted to double
        -Wformat=2                  # Warn on security issues around printf/scanf
        
        # Suppress some overly noisy warnings
        -Wno-unused-parameter       # Allow unused parameters (common in virtual methods)
    )
    
    # GCC-specific warnings
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE
            -Wmisleading-indentation    # Warn if indentation implies blocks where blocks do not exist
            -Wduplicated-cond           # Warn if if/else chain has duplicated conditions
            -Wduplicated-branches       # Warn if if/else branches have duplicated code
            -Wlogical-op                # Warn about logical operations being used where bitwise were probably wanted
            -Wuseless-cast              # Warn if you perform a cast to the same type
        )
    endif()
    
    # Clang-specific warnings
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target} PRIVATE
            -Wno-unknown-warning-option # Don't warn about unknown warning flags
        )
    endif()
endfunction()

# Optional: Function for strict warnings (for new code)
function(add_strict_warnings target)
    add_compiler_warnings(${target})
    target_compile_options(${target} PRIVATE
        -Werror                     # Treat warnings as errors
    )
endfunction()
