
########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Network-Monitor.conf
	preview.jpg
	icon.png
	link-0.svg
	link-1.svg
	link-2.svg
	link-3.svg
	link-4.svg
	link-5.svg
	network-wired.png
	network-not-connected.png
	DESTINATION ${network_monitordatadir})

install(FILES wifi
	DESTINATION ${network_monitordatadir}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
