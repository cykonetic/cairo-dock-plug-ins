add_subdirectory(themes)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Toons.conf
	preview.jpg
	icon.png
	DESTINATION ${toonsdatadir})
