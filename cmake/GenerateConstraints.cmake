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
            list(GET segs 1 property)
            list(LENGTH segs widgets_count)
            message(STATUS "widget: ${widget}, property: ${property}")
            string(APPEND contents "    // begin ${widget}\n")
            set(seg_num 0)
            foreach(seg ${segs})
                if(seg_num GREATER 1)
                    list(GET states ${seg_num} state)
                    if(seg STREQUAL "-")
                        #message(STATUS "ignore ${widget} with state ${state}")
                    else()
                        set(enabled_value true)
                        if (NOT seg)
                            set(enabled_value false)
                        endif()
                        string(APPEND contents "    BIND_PROP_TO_STATE(${widget}, \"${property}\", ${enabled_value}, ${state}State);\n")
                    endif()
                endif()
                math(EXPR seg_num "${seg_num} + 1")
            endforeach()
            string(APPEND contents "    // end ${widget}\n\n")
        endif()

        math(EXPR line_num "${line_num} + 1")
    endforeach()

    #message(STATUS ${contents})
    configure_file(
         ${CMAKE_CURRENT_LIST_DIR}/../src/ui/LaserControllerWindowConstraints.cpp.in
         ${CMAKE_CURRENT_LIST_DIR}/../src/ui/LaserControllerWindowConstraints.cpp
    )
endfunction()

generate_constraints()

