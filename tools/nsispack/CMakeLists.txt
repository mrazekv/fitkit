# Project
project(FITkit)
set(MAJOR_VERSION 1)
set(MINOR_VERSION 2)
set(PATCH_VERSION 7)
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

# Defines
cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0005 NEW)
add_definitions(-Wall -D__VERSION="${VERSION_STRING}")

# Find MinGW GNU make
find_file(GMAKE_BINARY mingw32-make.exe PATH)
if("${GMAKE_BINARY}" STREQUAL "GMAKE_BINARY-NOTFOUND")
    message(FATAL_ERROR "Can'f find gmake")
endif("${GMAKE_BINARY}" STREQUAL "GMAKE_BINARY-NOTFOUND")

# Find MinGW GNU rm
find_file(RM_BINARY mingw32-rm.exe PATH)
if("${RM_BINARY}" STREQUAL "RM_BINARY-NOTFOUND")
   message(ERROR "Can't find rm")
endif("${RM_BINARY}" STREQUAL "RM_BINARY-NOTFOUND")

# Find Python libraries
find_package(PythonLibs REQUIRED)
if(NOT PYTHON_LIBDIR)
    message(ERROR "Can't find PYTHON_LIBDIR")
    get_filename_component(PYTHON_LIBDIR ${PYTHON_LIBRARY} PATH)
endif(NOT PYTHON_LIBDIR)

set(PYTHON_LIBRARIESDIR ${PYTHON_LIBDIR}/../lib)
set(PYTHON_BINARY ${PYTHON_LIBDIR}/../python.exe)

# Compile necessary Python libraries
message(STATUS "Extraction and compilation of neccessary python libs ${CMAKE_BINARY_DIR}/pythonlib (command: ${PYTHON_BINARY} ${CMAKE_SOURCE_DIR}/exportlibs.py)")
execute_process(
    COMMAND ${PYTHON_BINARY} ${CMAKE_SOURCE_DIR}/exportlibs.py
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    OUTPUT_QUIET)

# Enumerate components
set(CPACK_COMPONENTS_ALL
    qdevkit
    fcmake
    gmake
    fkflash
    fkterm
	 fktest
    libkitclient
    libpythonqt
    qtlibs
    python27
    python27lib
    completer
    releaseinfo
    console
    rest
	 #	 virtual
    svn)

# Components
set(CPACK_COMPONENT_QDEVKIT_DISPLAY_NAME       "QDevKit")
set(CPACK_COMPONENT_FCMAKE_DISPLAY_NAME        "FCMake")
set(CPACK_COMPONENT_FKFLASH_DISPLAY_NAME       "FKFlash")
set(CPACK_COMPONENT_FKTERM_DISPLAY_NAME        "FKTerm")
set(CPACK_COMPONENT_FKTEST_DISPLAY_NAME        "FKTest")
set(CPACK_COMPONENT_GMAKE_DISPLAY_NAME         "GNU utilities")
set(CPACK_COMPONENT_LIBKITCLIENT_DISPLAY_NAME  "FITKit (libkitclient)")
set(CPACK_COMPONENT_LIBPYTHONQT_DISPLAY_NAME   "PythonQt")
set(CPACK_COMPONENT_QTLIBS_DISPLAY_NAME        "Qt4 minimal")
set(CPACK_COMPONENT_PYTHON25_DISPLAY_NAME      "Python library (python27)")
set(CPACK_COMPONENT_PYTHON25LIB_DISPLAY_NAME   "Python site libraries")
set(CPACK_COMPONENT_COMPLETER_DISPLAY_NAME     "QDevKit plugin - Completer")
set(CPACK_COMPONENT_RELEASEINFO_DISPLAY_NAME   "QDevKit plugin - Release info")
set(CPACK_COMPONENT_CONSOLE_DISPLAY_NAME       "QDevKit plugin - Python console")
set(CPACK_COMPONENT_REST_DISPLAY_NAME          "QDevKit plugin - reST browser")
#set(CPACK_COMPONENT_VIRTUAL_DISPLAY_NAME       "QDevKit plugin - virtualization")
set(CPACK_COMPONENT_SVN_DISPLAY_NAME           "Subversion client")

# Descriptions
set(CPACK_COMPONENT_QDEVKIT_DESCRIPTION
"FITKit graphical development kit written in Qt4. This component includes necessary plugins (fkflash, filetransfer).")
set(CPACK_COMPONENT_FCMAKE_DESCRIPTION
"Makefile generator for FITKit applications.")
set(CPACK_COMPONENT_FKFLASH_DESCRIPTION
"FITKit programming utility.")
set(CPACK_COMPONENT_FKTERM_DESCRIPTION
"Simple FITKit terminal.")
set(CPACK_COMPONENT_FKTEST_DESCRIPTION
"Simple regexp utility.")
set(CPACK_COMPONENT_LIBKITCLIENT_DESCRIPTION
"Multiplatform library used to talk with FITKit devices.")
set(CPACK_COMPONENT_LIBPYTHONQT_DESCRIPTION
"Library used to embed Python scripting in Qt application.")
set(CPACK_COMPONENT_QTLIBS_DESCRIPTION
"Minimal Qt4 libraries to run this application."
"Dont install if you have already Qt4 installed and compiled with msvcr90 support.")
set(CPACK_COMPONENT_PYTHON25_DESCRIPTION
"Python25 library used to execute the scripts")
set(CPACK_COMPONENT_PYTHON25LIB_DESCRIPTION
"Minimal Python25 libraries to run this application.")
set(CPACK_COMPONENT_GMAKE_DESCRIPTION
"GNU utilities used to process the Makefiles (make, rm)")
set(CPACK_COMPONENT_SVN_DESCRIPTION
"Subversion command-line client, version 1.5.2"
"Dont install if you have already SVN installed."
)

# Dependencies
set(CPACK_COMPONENT_QDEVKIT_DEPENDS      libkitclient libpythonqt python27 python27lib)
set(CPACK_COMPONENT_FKFLASH_DEPENDS      libkitclient python27)
set(CPACK_COMPONENT_FKTERM_DEPENDS       libkitclient python27)
set(CPACK_COMPONENT_FKTEST_DEPENDS       python27)
set(CPACK_COMPONENT_PYTHON25LIB_DEPENDS  python27)
set(CPACK_COMPONENT_COMPLETER_DEPENDS    qdevkit)
set(CPACK_COMPONENT_RELEASEINFO_DEPENDS  qdevkit)
set(CPACK_COMPONENT_CONSOLE_DEPENDS      qdevkit)
set(CPACK_COMPONENT_REST_DEPENDS         qdevkit)
#set(CPACK_COMPONENT_VIRTUAL_DEPENDS      qdevkit)

# Groups
set(CPACK_COMPONENT_QDEVKIT_GROUP      "Applications")
set(CPACK_COMPONENT_FCMAKE_GROUP       "Applications")
set(CPACK_COMPONENT_FKFLASH_GROUP      "Applications")
set(CPACK_COMPONENT_FKTERM_GROUP       "Applications")
set(CPACK_COMPONENT_FKTEST_GROUP       "Applications")
set(CPACK_COMPONENT_LIBKITCLIENT_GROUP "Libraries")
set(CPACK_COMPONENT_LIBPYTHONQT_GROUP  "Contrib")
set(CPACK_COMPONENT_QTLIBS_GROUP       "Contrib")
set(CPACK_COMPONENT_PYTHON25_GROUP     "Contrib")
set(CPACK_COMPONENT_PYTHON25LIB_GROUP  "Contrib")
set(CPACK_COMPONENT_GMAKE_GROUP        "Contrib")
set(CPACK_COMPONENT_SVN_GROUP          "Contrib")
set(CPACK_COMPONENT_COMPLETER_GROUP    "Plugins")
set(CPACK_COMPONENT_RELEASEINFO_GROUP  "Plugins")
set(CPACK_COMPONENT_CONSOLE_GROUP      "Plugins")
set(CPACK_COMPONENT_REST_GROUP         "Plugins")
#set(CPACK_COMPONENT_VIRTUAL_GROUP      "Plugins-optional")

# Package settings
set(CPACK_GENERATOR                    "NSIS")
set(CPACK_CMAKE_GENERATOR              "MinGW Makefiles")
set(CPACK_PACKAGE_NAME                 ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR               "VUT FIT")
set(CPACK_PACKAGE_VERSION              ${VERSION_STRING})
set(CPACK_PACKAGE_CONTACT              "Marek Vavrusa, marek@vavrusa.com")
set(CPACK_PACKAGE_DESCRIPTION          "FITKit development kit based on Qt4.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY  ${CPACK_PACKAGE_DESCRIPTION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY    ${CPACK_PACKAGE_NAME})
set(CPACK_PACKAGE_FILE_NAME            "${PROJECT_NAME}-${VERSION_STRING}-win32")
set(CPACK_NSIS_DISPLAY_NAME            "QDevKit")
set(CPACK_NSIS_HELP_LINK               "http://www.fit.vutbr.cz/fitkit")
set(CPACK_NSIS_URL_INFO_ABOUT          "http://www.fit.vutbr.cz/fitkit")
set(CPACK_NSIS_MUI_ICON                "${CMAKE_SOURCE_DIR}/qdevkit.ico")
set(CPACK_NSIS_MODIFY_PATH             "ON")

set(CPACK_RESOURCE_FILE_LICENSE        ${CMAKE_SOURCE_DIR}/COPYING)
set(CPACK_PACKAGE_EXECUTABLES          "qdevkit;QDevKit")

#create desktop shortcut
set(CPACK_NSIS_CREATE_ICONS_EXTRA "StrCmp $INSTALL_DESKTOP \\\"1\\\" 0 doNotInstallDesktop
   CreateShortCut \\\"$DESKTOP\\\\QDevKit.lnk\\\" \\\"$INSTDIR\\\\bin\\\\qdevkit.exe\\\"
doNotInstallDesktop:")

#delete desktop shortcut
set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete \\\"$DESKTOP\\\\QDevKit.lnk\\\"")

# Target directories
if("${QDEVKIT_BUILD}" STREQUAL "")
   set(QDEVKIT_BUILD     ${CMAKE_INSTALL_PREFIX}/bin)
endif("${QDEVKIT_BUILD}" STREQUAL "")

if("${FCMAKE_BUILD}" STREQUAL "")
   set(FCMAKE_BUILD     ${CMAKE_INSTALL_PREFIX}/bin)
endif("${FCMAKE_BUILD}" STREQUAL "")

if("${FKFLASH_SOURCE}" STREQUAL "")
   set(FKFLASH_SOURCE     ${CMAKE_SOURCE_DIR}/../fkflash)
endif("${FKFLASH_SOURCE}" STREQUAL "")

if("${FKTERM_SOURCE}" STREQUAL "")
   set(FKTERM_SOURCE     ${CMAKE_SOURCE_DIR}/../fkterm)
endif("${FKTERM_SOURCE}" STREQUAL "")

if("${FKTEST_SOURCE}" STREQUAL "")
   set(FKTEST_SOURCE     ${CMAKE_SOURCE_DIR}/../fktest)
endif("${FKTEST_SOURCE}" STREQUAL "")

if("${LIBKITCLIENT_BUILD}" STREQUAL "")
   set(LIBKITCLIENT_BUILD      ${CMAKE_INSTALL_PREFIX}/bin)
endif("${LIBKITCLIENT_BUILD}" STREQUAL "")

if("${LIBKITCLIENT_SOURCE}" STREQUAL "")
   set(LIBKITCLIENT_SOURCE     ${CMAKE_SOURCE_DIR}/../libkitclient)
endif("${LIBKITCLIENT_SOURCE}" STREQUAL "")

if("${LIBPYTHONQT_BUILD}" STREQUAL "")
   set(LIBPYTHONQT_BUILD      ${CMAKE_INSTALL_PREFIX}/bin)
endif("${LIBPYTHONQT_BUILD}" STREQUAL "")

if("${PLUGINS_BUILD}" STREQUAL "")
   set(PLUGINS_BUILD      ${CMAKE_INSTALL_PREFIX}/plugins)
endif("${PLUGINS_BUILD}" STREQUAL "")

if("${LOCALE_DIR}" STREQUAL "")
   set(LOCALE_DIR      ${CMAKE_INSTALL_PREFIX}/locale)
endif("${LOCALE_DIR}" STREQUAL "")

if("${PLUGINS_SOURCE}" STREQUAL "")
   set(PLUGINS_SOURCE     ${CMAKE_SOURCE_DIR}/../qdevkit-plugins)
endif("${PLUGINS_SOURCE}" STREQUAL "")

if("${SVN_BINARIES}" STREQUAL "")
   set(SVN_BINARIES     ${CMAKE_SOURCE_DIR}/../svn)
endif("${SVN_BINARIES}" STREQUAL "")

# Include them
message(STATUS "QDevKit      build:  ${QDEVKIT_BUILD}")
message(STATUS "QDevKit     locale:  ${LOCALE_DIR}")
message(STATUS "QDK plugins source:  ${PLUGINS_SOURCE}")
message(STATUS "QDK plugins  build:  ${PLUGINS_BUILD}")
message(STATUS "FCMake       build:  ${FCMAKE_BUILD}")
message(STATUS "FKFlash     source:  ${FKFLASH_SOURCE}")
message(STATUS "FKTerm      source:  ${FKTERM_SOURCE}")
message(STATUS "FKTest      source:  ${FKTEST_SOURCE}")
message(STATUS "KitClient    build:  ${LIBKITCLIENT_BUILD}")
message(STATUS "PythonQt     build:  ${LIBPYTHONQT_BUILD}")
message(STATUS "Python   libraries:  ${PYTHON_LIBRARIESDIR}")
message(STATUS "GNU make    binary:  ${GMAKE_BINARY}")
message(STATUS "GNU rm      binary:  ${RM_BINARY}")
message(STATUS "SVN       binaries:  ${SVN_BINARIES}")

message(STATUS "Compile sources: ${FKFLASH_SOURCE}/fkutils")
execute_process(COMMAND ${PYTHON_BINARY} -c "import compileall;compileall.compile_dir(\"fkutils\", force=True)"
                WORKING_DIRECTORY ${FKFLASH_SOURCE}
                OUTPUT_QUIET
               )

message(STATUS "Compile sources: ${FKFLASH_SOURCE}/mspgcc")
execute_process(COMMAND ${PYTHON_BINARY} -c "import compileall;compileall.compile_dir(\"mspgcc\", force=True)"
                WORKING_DIRECTORY ${FKFLASH_SOURCE}
                OUTPUT_QUIET
               )

message(STATUS "Compile sources: ${FKFLASH_SOURCE}")
execute_process(COMMAND ${PYTHON_BINARY} "setup.py" "py2exe"
                WORKING_DIRECTORY ${FKFLASH_SOURCE}
                OUTPUT_QUIET
               )

message(STATUS "Compile sources: ${FKTERM_SOURCE}")
execute_process(COMMAND ${PYTHON_BINARY} "setup.py" "py2exe"
                WORKING_DIRECTORY ${FKTERM_SOURCE}
                OUTPUT_QUIET
               )

message(STATUS "Compile sources: ${FKTEST_SOURCE}")
execute_process(COMMAND ${PYTHON_BINARY} "setup.py" "py2exe"
              	 WORKING_DIRECTORY ${FKTEST_SOURCE}
                OUTPUT_QUIET
               )
				
# Include Qt4
find_package(Qt4)
set(QT_USE_QTXML 1)
set(QT_USE_QTSVG 1)
include(${QT_USE_FILE})
set(QT_BINDIR ${QT_LIBRARY_DIR}/../bin)

install( FILES ${RM_BINARY}
         DESTINATION bin
         COMPONENT gmake
         RENAME rm.exe
         )

install( FILES ${GMAKE_BINARY}
         DESTINATION bin
         COMPONENT gmake
         RENAME gmake.exe
         )

install( FILES ${FKFLASH_SOURCE}/dist/python27.dll #${LIBKITCLIENT_BUILD}/msvcr90.dll  ${LIBKITCLIENT_BUILD}/msvcp90.dll
         DESTINATION bin
         COMPONENT python27
         )

install( DIRECTORY ${CMAKE_BINARY_DIR}/pythonlib/
         DESTINATION lib/python
         COMPONENT python27lib
         )

install( FILES ${QDEVKIT_BUILD}/qdevkit.exe ${QDEVKIT_BUILD}/qdevkitcore.dll
         DESTINATION bin
         COMPONENT qdevkit
         )

install( FILES ${LOCALE_DIR}/qdevkit_cs.qm
         DESTINATION locale
         COMPONENT qdevkit
         )

install( FILES ${FCMAKE_BUILD}/fcmake.exe ${FCMAKE_BUILD}/fcmakecore.dll
         DESTINATION bin
         COMPONENT fcmake
         )

install( FILES ${FKTERM_SOURCE}/dist/fkterm.exe
         DESTINATION bin
         COMPONENT fkterm
         PERMISSIONS  OWNER_EXECUTE OWNER_READ GROUP_EXECUTE GROUP_READ
         WORLD_EXECUTE WORLD_READ
         )

install( FILES ${FKTEST_SOURCE}/dist/fktest.exe
         DESTINATION bin
         COMPONENT fktest
         PERMISSIONS  OWNER_EXECUTE OWNER_READ GROUP_EXECUTE GROUP_READ
         WORLD_EXECUTE WORLD_READ
         )
		
install( FILES ${FKFLASH_SOURCE}/dist/fkflash.exe
         DESTINATION bin
         COMPONENT fkflash
         PERMISSIONS  OWNER_EXECUTE OWNER_READ GROUP_EXECUTE GROUP_READ
         WORLD_EXECUTE WORLD_READ
         )

install (FILES ${PLUGINS_BUILD}/fkflash/fkflash.dll
               ${PLUGINS_SOURCE}/fkflash/fkflash.png
               ${PLUGINS_SOURCE}/fkflash/plugin.xml
         DESTINATION plugins/fkflash
         COMPONENT qdevkit
         )

install (DIRECTORY ${FKFLASH_SOURCE}/fkutils
         DESTINATION plugins/fkflash
         COMPONENT qdevkit
         FILES_MATCHING PATTERN "*.pyc"
         PATTERN ".svn" EXCLUDE
         )

install (DIRECTORY ${FKFLASH_SOURCE}/mspgcc
         DESTINATION plugins/fkflash
         COMPONENT qdevkit
         FILES_MATCHING PATTERN "*.py" PATTERN "*.pyc"
         PATTERN ".svn" EXCLUDE
         )

install (FILES ${PLUGINS_BUILD}/filetransfer/filetransfer.dll
               ${PLUGINS_SOURCE}/filetransfer/plugin.xml
         DESTINATION plugins/filetransfer
         COMPONENT qdevkit
         )

if (MSVC)
install( FILES ${LIBKITCLIENT_SOURCE}/contrib/d2xx/ftd2xx.dll
         DESTINATION bin
         COMPONENT libkitclient
         )
else(MSVC)
install( FILES ${LIBKITCLIENT_SOURCE}/contrib/d2xx/msvc/ftd2xx.dll
         DESTINATION bin
         COMPONENT libkitclient
         )

endif(MSVC) 

install( FILES ${LIBKITCLIENT_BUILD}/kitclient.dll ${LIBKITCLIENT_BUILD}/ssh.dll ${LIBKITCLIENT_BUILD}/libeay32.dll ${LIBKITCLIENT_BUILD}/zlib1.dll
         DESTINATION bin
         COMPONENT libkitclient
         )

install (FILES ${PLUGINS_SOURCE}/releaseinfo/release.py
               ${PLUGINS_SOURCE}/releaseinfo/plugin.xml
         DESTINATION plugins/releaseinfo
         COMPONENT releaseinfo
         )

install (FILES ${PLUGINS_BUILD}/completer/completer.dll
               ${PLUGINS_SOURCE}/completer/plugin.xml
         DESTINATION plugins/completer
         COMPONENT completer
         )

install (FILES ${PLUGINS_BUILD}/pythonqtconsole/pythonqtconsole.dll
               ${PLUGINS_SOURCE}/pythonqtconsole/console.png
               ${PLUGINS_SOURCE}/pythonqtconsole/plugin.xml
         DESTINATION plugins/pythonqtconsole
         COMPONENT console
         )

install (FILES ${PLUGINS_BUILD}/rstbrowser/rstbrowser.dll
               ${PLUGINS_SOURCE}/rstbrowser/rstbrowser.png
               ${PLUGINS_SOURCE}/rstbrowser/plugin.xml
         DESTINATION plugins/rstbrowser
         COMPONENT rest
         )

# install (FILES ${PLUGINS_BUILD}/virtualkit/virtualkit.dll
#                ${PLUGINS_SOURCE}/virtualkit/plugin.xml
#          DESTINATION plugins/virtualkit
#          COMPONENT virtual
#          )
# 
# #OpenCV libraries
# install (FILES ${PLUGINS_SOURCE}/virtualkit/bin/cxcore210.dll
# 			      ${PLUGINS_SOURCE}/virtualkit/bin/highgui210.dll
# 			      ${PLUGINS_SOURCE}/virtualkit/bin/opencv_ffmpeg210.dll
#          DESTINATION bin
#          COMPONENT virtual
#          )

		
install( FILES ${LIBPYTHONQT_BUILD}/PythonQt.dll ${LIBPYTHONQT_BUILD}/PythonQt.dll
         DESTINATION bin
         COMPONENT libpythonqt
         )

install( FILES ${QT_BINDIR}/QtCore4.dll ${QT_BINDIR}/QtGui4.dll ${QT_BINDIR}/QtXml4.dll ${QT_BINDIR}/QtSvg4.dll ${QT_BINDIR}/QtNetwork4.dll ${QT_BINDIR}/QtWebKit4.dll ${QT_BINDIR}/QtOpenGL4.dll ${QT_BINDIR}/QtSql4.dll ${QT_BINDIR}/phonon4.dll 
         DESTINATION bin
         COMPONENT qtlibs
         )

install (DIRECTORY ${SVN_BINARIES}/
         DESTINATION contrib/svn
         COMPONENT svn
         )


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})

# Include CPack
include(CPack)
