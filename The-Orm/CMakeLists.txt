#
#  Copyright (c) 2020 Christof Ruch. All rights reserved.
#
#  Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
#

cmake_minimum_required(VERSION 3.14)

find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --dirty=-dev --long
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE "PROJECT_VERSION_FULL"
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Project full version: ${PROJECT_VERSION_FULL}")
else()
    set(PROJECT_VERSION_FULL "unknown")
endif()

# Break up the version, number of commits, and the dev tag into parts
string(REGEX REPLACE "^([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" PROJECT_VERSION ${PROJECT_VERSION_FULL})
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+-([0-9]+).*" "\\1" PROJECT_TWEAK_VERSION ${PROJECT_VERSION_FULL})
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+-g[0-9a-f]+(.*)" "\\1" PROJECT_DEV_TAG ${PROJECT_VERSION_FULL})

# If there were no new commits, PROJECT_TWEAK_VERSION will contain a hash or "-dev", so we need to set it to 0 in that case.
if(PROJECT_TWEAK_VERSION MATCHES "-g[0-9a-f]+.*")
    set(PROJECT_TWEAK_VERSION 0)
endif()

# Append the tweak version if it's not 0
if(NOT PROJECT_TWEAK_VERSION EQUAL 0)
    set(PROJECT_VERSION ${PROJECT_VERSION}.${PROJECT_TWEAK_VERSION})
endif()

project(KnobKraftOrm VERSION ${PROJECT_VERSION})

# Now you can use PROJECT_DEV_TAG in your CMake scripts or in your code to handle the "-dev" part.
# For example, you could add a preprocessor definition that your code could use:
if(PROJECT_DEV_TAG STREQUAL "-dev")
    add_definitions(-DPROJECT_DEV)
endif()

# Export this variable to be use by the Azure
# Append the version number to the github action environment file to be used by subsequent steps 
if(DEFINED ENV{GITHUB_ENV})
file(APPEND $ENV{GITHUB_ENV} "ORM_VERSION=${KnobKraftOrm_VERSION}")
endif()

OPTION(CRASH_REPORTING "Turn on crash reporting via Internet/Sentry")
OPTION(SENTRY_LOGGING "Turn on logging of sentry events into the log window")
set(SENTRY_DSN "Sentry DSN URL" CACHE STRING "https://YYYYYYYYYYYYYYYYYY@ZZZZZ.ingest.sentry.io/XXXX")
set(D_LOG_SENTRY "")
OPTION(SPARKLE_UPDATES "Turn on WinSparkle update service")

configure_file("version.cpp.in" "version.cpp")

IF(CRASH_REPORTING)
	message("Internet-based crash reporting is turned on, using Sentry endpoint authorization from SENTRY_DSN CMake variable")
	configure_file("${CMAKE_CURRENT_LIST_DIR}/sentry-config.h.in" "${CMAKE_CURRENT_LIST_DIR}/sentry-config.h")
	SET(SENTRY_INSTALL_PATH "${CMAKE_CURRENT_LIST_DIR}/../third_party/sentry-native/install")
	SET(SENTRY_CONFIG_HEADER sentry-config.h.in)
	SET(SENTRY_DISTRIBUTION_FILES "${SENTRY_INSTALL_PATH}/bin/sentry.dll" "${SENTRY_INSTALL_PATH}/bin/crashpad_handler.exe")
ENDIF()

IF(SPARKLE_UPDATES)
	message("Sparkle and WinSparkle updates are turned on, the executable will be linked against WinSparkle")
	juce_add_binary_data(CodeSigning SOURCES "${CMAKE_CURRENT_LIST_DIR}/../codesigning/dsa_pub.pem") 
	SET(WINSPARKLE_DISTRIBUTION_FILES "${WINSPARKLE_LIBDIR}/WinSparkle.dll")
ENDIF()

set(SOURCES
	AdaptationView.cpp AdaptationView.h
	AutoCategorizeWindow.cpp AutoCategorizeWindow.h
	AutoDetectProgressWindow.cpp AutoDetectProgressWindow.h
	AutoThumbnailingDialog.cpp AutoThumbnailingDialog.h
	BCR2000_Component.cpp BCR2000_Component.h
	BulkRenameDialog.cpp BulkRenameDialog.h
	CreateListDialog.cpp CreateListDialog.h
	CurrentPatchDisplay.cpp CurrentPatchDisplay.h
	EditCategoryDialog.cpp EditCategoryDialog.h
	ElectraOneRouter.cpp ElectraOneRouter.h
	ExportDialog.cpp ExportDialog.h
	ImportFromSynthDialog.cpp ImportFromSynthDialog.h
	KeyboardMacroView.cpp KeyboardMacroView.h
	LibrarianProgressWindow.h
	MacroConfig.cpp MacroConfig.h
	MainComponent.h MainComponent.cpp	
	Main.cpp
	OrmLookAndFeel.cpp OrmLookAndFeel.h
	PatchButtonPanel.cpp PatchButtonPanel.h
	PatchDiff.cpp PatchDiff.h
	PatchHistoryPanel.cpp PatchHistoryPanel.h
	PatchHolderButton.cpp PatchHolderButton.h
	PatchListTree.cpp PatchListTree.h
	PatchPerSynthList.cpp PatchPerSynthList.h
	PatchSearchComponent.cpp PatchSearchComponent.h
	PatchTextBox.cpp PatchTextBox.h
	PatchView.cpp PatchView.h
	ReceiveManualDumpWindow.cpp ReceiveManualDumpWindow.h
	RecordingView.cpp RecordingView.h
	RotaryWithLabel.cpp RotaryWithLabel.h
	ScriptedQuery.cpp ScriptedQuery.h
	SecondaryWindow.cpp SecondaryWindow.h
	SettingsView.cpp SettingsView.h
	SetupView.cpp SetupView.h
	SimplePatchGrid.cpp SimplePatchGrid.h
	SynthBankPanel.cpp SynthBankPanel.h
	UIModel.cpp UIModel.h
	VerticalPatchButtonList.cpp VerticalPatchButtonList.h
	win_resources.rc
	version.cpp.in
	setup.iss
	ideas.txt
	${SENTRY_CONFIG_HEADER}
	redist/agpl-3.0.txt
)

# Mac Icon Magic
if(APPLE)
set(KnobKraftOrm_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon_orm.icns)
set_source_files_properties(resources/icon_orm.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
ENDIF()

set(MIDIKRAFT_LIBRARIES
		juce-utils 
		juce-widgets 
		midikraft-base 
		midikraft-librarian 
		midikraft-database 
		midikraft-access-virus
		midikraft-behringer-bcr2000
		midikraft-kawai-k3
		midikraft-oberheim-matrix1000
		midikraft-roland-mks50 
		midikraft-roland-mks80 
		midikraft-sequential-rev2   
		midikraft-sequential-ob6  
		knobkraft-generic-adaptation
		pytschirp_embedded
)

add_executable(KnobKraftOrm MACOSX_BUNDLE WIN32 ${KnobKraftOrm_ICON} ${SOURCES})
target_include_directories(KnobKraftOrm 
	INTERFACE ${CMAKE_CURRENT_LIST_DIR}})
target_include_directories(KnobKraftOrm SYSTEM
	PRIVATE "${SENTRY_INSTALL_PATH}/include" "${CMAKE_CURRENT_LIST_DIR}/../third_party/dtl" "${CMAKE_CURRENT_BINARY_DIR}" "${WINSPARKLE_INCLUDE}")
target_compile_definitions(KnobKraftOrm PRIVATE JUCE_MODULE_AVAILABLE_gin_gui JUCE_MODULE_AVAILABLE_gin)
get_target_property(gin_include_dirs gin INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(KnobKraftOrm SYSTEM PRIVATE ${gin_include_dirs})
IF(WIN32)	
	IF(CRASH_REPORTING)
		IF(SENTRY_LOGGING)
			set(D_LOG_SENTRY "-DLOG_SENTRY")
		ENDIF()
		target_compile_options(KnobKraftOrm PRIVATE -DUSE_SENTRY ${D_LOG_SENTRY})
		set(SENTRY_LIB sentry)
	ENDIF()
	IF(SPARKLE_UPDATES)
		target_compile_options(KnobKraftOrm PRIVATE -DUSE_SPARKLE)
		set(SPARKLE_DEPENDENCY CodeSigning)
	ENDIF()
	target_link_directories(KnobKraftOrm PRIVATE "${SENTRY_INSTALL_PATH}/lib" "${WINSPARKLE_LIBDIR}")
	if(ASAN)
		set(ASAN_LINK_LIB clang_rt.asan_dbg_dynamic-x86_64)
		target_link_options(KnobKraftOrm PRIVATE  /wholearchive:clang_rt.asan_dbg_dynamic-x86_64.lib)
	endif()
	target_link_libraries(KnobKraftOrm PRIVATE ${JUCE_LIBRARIES} ${MIDIKRAFT_LIBRARIES}
		icuuc
		SQLiteCpp
		sqlite3
		gin
		spdlog::spdlog
		${SENTRY_LIB}
		${SPARKLE_DEPENDENCY}
		)
ELSEIF(APPLE)
	target_link_libraries(KnobKraftOrm PRIVATE ${JUCE_LIBRARIES} ${MIDIKRAFT_LIBRARIES}
		ICU::data ICU::uc
		SQLiteCpp
		sqlite3
		gin
		spdlog::spdlog
		)
	SET_TARGET_PROPERTIES(KnobKraftOrm PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KnobKraft Orm ${KnobKraftOrm_VERSION}"
		MACOSX_BUNDLE_ICON_FILE icon_orm.icns
		MACOSX_BUNDLE_BUNDLE_VERSION ${KnobKraftOrm_VERSION}
		)
ELSEIF(UNIX)
	target_link_libraries(KnobKraftOrm PRIVATE 
		${JUCE_LIBRARIES} 
		${LINUX_JUCE_LINK_LIBRARIES}
		ICU::data ICU::uc
		${MIDIKRAFT_LIBRARIES}
		SQLiteCpp
		sqlite3
		gin
		spdlog::spdlog
		)
	target_compile_options(KnobKraftOrm PRIVATE -pthread -I/usr/include/webkitgtk-4.0 -I/usr/include/gtk-3.0 -I/usr/include/at-spi2-atk/2.0 -I/usr/include/at-spi-2.0 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/include/gtk-3.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/libsoup-2.4 -I/usr/include/libxml2 -I/usr/include/webkitgtk-4.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include)
ENDIF()

# Use all cores
IF (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Pedantic about warnings
if (MSVC)
    # warning level 4 and all warnings as errors
	# Turn off 4068 unknown pragmas
    target_compile_options(KnobKraftOrm PRIVATE /W4 /WX /we4388 /we4389 /wd4068)
else()    
	if(APPLE)
		# lots of warnings and all warnings as errors    
		target_compile_options(KnobKraftOrm PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unknown-pragmas -Wno-missing-field-initializers)
	else()
		# only lots of warnings now
		target_compile_options(KnobKraftOrm PRIVATE -Wall -Wextra -pedantic -Wno-unknown-pragmas -Wno-missing-field-initializers)
	endif()
endif()

IF(WIN32)
	# Post build commands - copy the DLLs (Windows only I guess)
	add_custom_command(
			TARGET KnobKraftOrm POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
					"${icu_SOURCE_DIR}/bin64/icuuc74.dll" "${icu_SOURCE_DIR}/bin64/icudt74.dll"
					${SENTRY_DISTRIBUTION_FILES} ${WINSPARKLE_DISTRIBUTION_FILES}
					${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
	add_custom_command(
			TARGET KnobKraftOrm POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
					"${PYTHON_EMBEDDED_PATH}/python${PYTHON_VERSION_TO_COPY}.dll"
					${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
ENDIF()

# Installation - we want to copy the sequential module directory as a subdirectory to the binary output dir. Same with roland and knobkraft python modules
add_custom_command(TARGET KnobKraftOrm POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/adaptations/sequential
		$<TARGET_FILE_DIR:KnobKraftOrm>/sequential
		)
add_custom_command(TARGET KnobKraftOrm POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/adaptations/roland
		$<TARGET_FILE_DIR:KnobKraftOrm>/roland
		)
add_custom_command(TARGET KnobKraftOrm POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/adaptations/knobkraft
		$<TARGET_FILE_DIR:KnobKraftOrm>/knobkraft
		)
add_custom_command(TARGET KnobKraftOrm POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/adaptations/testing
		$<TARGET_FILE_DIR:KnobKraftOrm>/testing
		)
file(GLOB ADAPTATION_PYTHON_FILES "${CMAKE_SOURCE_DIR}/adaptations/*.py")
list(FILTER ADAPTATION_PYTHON_FILES EXCLUDE REGEX "conftest.py")
list(FILTER ADAPTATION_PYTHON_FILES EXCLUDE REGEX "test_.*py")
list(FILTER ADAPTATION_PYTHON_FILES EXCLUDE REGEX "DSI_OB6.py")
add_custom_command(TARGET KnobKraftOrm POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory 
		$<TARGET_FILE_DIR:KnobKraftOrm>/adaptations
		)
add_custom_command(TARGET KnobKraftOrm POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		${ADAPTATION_PYTHON_FILES}
		$<TARGET_FILE_DIR:KnobKraftOrm>/adaptations
		)

IF(WIN32)
	# Windows only: Build InnoSetup installer
	find_program(INNOSETUP NAMES iscc ISCC HINTS "${CMAKE_CURRENT_LIST_DIR}/../third_party/Tools.InnoSetup.6.0.5/tools" "c:/program files (x86)/Inno Setup 6")
	if (INNOSETUP)
	else()
		message(WARNING "Didn't find InnoSetup 6 installation, skipping creation of Client installer '")
	endif()
ENDIF()

IF(WIN32)
	 # Parse Inno Setup config file
	configure_file("${CMAKE_CURRENT_LIST_DIR}/setup.iss"
				   "${CMAKE_CURRENT_BINARY_DIR}/setup.iss")
	set(no_copy $<NOT:$<CONFIG:RelWithDebInfo>>)
	add_custom_command(
		TARGET KnobKraftOrm POST_BUILD
		COMMAND  "${CMAKE_COMMAND}" -E $<${no_copy}:echo> $<${no_copy}:"Not building Installer with "> chdir . ${INNOSETUP} "${CMAKE_CURRENT_BINARY_DIR}/setup.iss"
		DEPENDS setup.iss 
	)
ENDIF()

IF(APPLE)
    # This is supposed to build a relocatable macOS DMG installer when you specify the
    # --target package
    SET(MY_RELEASE_DIR ${CMAKE_BINARY_DIR}/macInstaller)
    SET(APPS "\${CMAKE_INSTALL_PREFIX}/KnobKraftOrm.app")
    SET(DIRS ${CMAKE_BINARY_DIR})
    INSTALL(TARGETS KnobKraftOrm BUNDLE DESTINATION . COMPONENT Runtime)
    INSTALL(CODE "include(BundleUtilities)
	set(BU_CHMOD_BUNDLE_ITEMS TRUE)
    fixup_bundle(\"${APPS}\"  \"\"  \"${DIRS}\" IGNORE_ITEM \"Python\")
    " COMPONENT Runtime)
    
    # Setup CPack variables
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon_orm.png")
    set(CPACK_PACKAGE_VERSION ${KnobKraftOrm_VERSION})
    set(CPACK_PACKAGE_VENDOR "Christof Ruch Beratungs UG (haftungsbeschraenkt)")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/redist/agpl-3.0.txt")
	#set(CPACK_RESOURCE_FILE_README "${CMAKE_PROJECT_DIR}/readme.md")
    
    # Select only the dmg generator for now
    SET(CPACK_BINARY_DRAGNDROP ON)
    SET(CPACK_BINARY_TGZ OFF)
    SET(CPACK_BINARY_STGZ OFF)
    INCLUDE(CPack)
ELSE()
	INSTALL(TARGETS KnobKraftOrm knobkraft-generic-adaptation
			BUNDLE DESTINATION . COMPONENT Runtime
			)
	IF(UNIX)
		SET(CPACK_BINARY_TGZ ON)
		set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon_orm.png")
		set(CPACK_PACKAGE_VERSION ${KnobKraftOrm_VERSION})
		set(CPACK_PACKAGE_VENDOR "Christof Ruch Beratungs UG (haftungsbeschraenkt)")
		set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/redist/agpl-3.0.txt")
		INCLUDE(CPack)
	ENDIF()
ENDIF()
