if(BUILD_EXAMPLES)
        foreach(example "simple" )
		qt_add_executable(${example} ${example}.cpp )

                target_link_libraries(${example} PRIVATE PRIVATE vault)

		set_target_properties(${example} PROPERTIES
			WIN32_EXECUTABLE ON
			MACOSX_BUNDLE ON
			)
	endforeach()
endif(BUILD_EXAMPLES)
