add_library(designBrowser designBrowser.cpp)

target_include_directories(designBrowser
    PUBLIC ${DESIGNBROWSER_HOME}/include
    PRIVATE
    ${OPENROAD_HOME}/include
    ${OPENSTA_HOME}/include
    ${DBSTA_HOME}/include
    ${OPENDB_HOME}/include
    ${OPENSTA_INCLUDE_DIRS}
    ${TCL_INCLUDE_PATH}
)
