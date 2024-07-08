# 当前目录下所有.c文件保存到srcs变量里面去！
file(GLOB srcs
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
        )

# 看看当前的.c文件有哪些
message(${srcs})
# 我是一个共享库哦，共享库内容包括了当前目录下的所有.c文件
add_library(myuzlib STATIC ${srcs})
#add_library(myuzlib SHARED ${srcs})
# 在哪里找头文件？在当前目录下找
target_include_directories(myuzlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


