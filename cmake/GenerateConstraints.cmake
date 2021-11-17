MESSAGE(STATUS "Generating constraints.cpp")

MESSAGE(STATUS  "Reading from ${CMAKE_CURRENT_LIST_DIR}/../res/constraints.csv")

function(generate_constraints)
    # read all lines from csv file
    file(STRINGS ${CMAKE_CURRENT_LIST_DIR}/../res/constraints.csv all_lines)

    set(line_num 0)     # line number
    set(states "")      # names of all states
    set(contents "")

    foreach(line ${all_lines})
        MESSAGE(STATUS "${line_num}. ${line}")
        string(REPLACE "," ";" segs ${line})
        #MESSAGE(STATUS ${segs})

        if (line_num EQUAL 0)
            foreach(seg ${segs})
                list(APPEND states ${seg})
            endforeach()
            list(LENGTH states states_count)
            message(STATUS "count of states is ${states_count}")
        else()
            list(GET segs 0 widget)
            list(LENGTH segs widgets_count)
            message(STATUS "widget: ${widget}, count: ${widgets_count}")
            set(seg_num 0)
            foreach(seg ${segs})
                if(seg_num GREATER 0)
                    list(GET states ${seg_num} state)
                    set(enabled_value true)
                    if(NOT seg)
                        set(enabled_value false)
                    endif()
                    #message(STATUS "BIND_PROP_TO_STATE(${widget}, \"enabled\", false, ${state}State);")
                    string(APPEND contents "    BIND_PROP_TO_STATE(${widget}, \"enabled\", ${enabled_value}, ${state}State);\n")
                endif()
                math(EXPR seg_num "${seg_num} + 1")
            endforeach()
        endif()

        math(EXPR line_num "${line_num} + 1")
    endforeach()

    message(STATUS ${contents})
    #configure_file(
    #     ${CMAKE_CURRENT_LIST_DIR}/../src/ui/LaserControllerWindowConstraints.cpp.in
    #     ${CMAKE_CURRENT_LIST_DIR}/../src/ui/LaserControllerWindowConstraints.cpp
    #)
endfunction()

generate_constraints()

