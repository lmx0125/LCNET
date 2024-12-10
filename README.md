# Description

A lib can make net connection easier!<br>
(support linux now!)

## Build

1. **Clone this project in to your project**

    ```
    git clone https://github.com/lmx0125/LCNET.git
    ```

1. **Include the head file like "net.h"**

    ```
    #include "net.h"
    ```

> other modules like p2p needs include other head file ( like p2p/p2p.h ) <br>
> If you are using visual studio or etc., you may link ws2_32.lib

2. **Build it**

> use msvc and g++ or etc. to build it<br>
> (g++ needs to add all cpp files by your self)

##### CMAKE

> if you are using cmake to build while using this lib<br>
> you may add this template in to your CMakeLists.txt<br>

```
project(lcnet-test)
cmake_minimum_required(VERSION 3.10)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

file(GLOB SOURCES 
	${PROJECT_SOURCE_DIR}/LCNET/IDgen/*.cpp
	${PROJECT_SOURCE_DIR}/LCNET/STUN/*.cpp
	${PROJECT_SOURCE_DIR}/LCNET/encoder/*.cpp
	${PROJECT_SOURCE_DIR}/LCNET/log/*.cpp
	${PROJECT_SOURCE_DIR}/LCNET/UDP/*.cpp
	${PROJECT_SOURCE_DIR}/LCNET/p2p/*.cpp
	${PROJECT_SOURCE_DIR}/LCNET/*.cpp
)

file(GLOB HEADERS 
	${PROJECT_SOURCE_DIR}/LCNET/IDgen/*.h
	${PROJECT_SOURCE_DIR}/LCNET/STUN/*.h
	${PROJECT_SOURCE_DIR}/LCNET/encoder/*.h
	${PROJECT_SOURCE_DIR}/LCNET/log/*.h
	${PROJECT_SOURCE_DIR}/LCNET/UDP/*.h
	${PROJECT_SOURCE_DIR}/LCNET/p2p/*.h
	${PROJECT_SOURCE_DIR}/LCNET/*.h
	/usr/include/uuid/uuid.h
)

include_directories(
	${PROJECT_SOURCE_DIR}/LCNET/IDgen
	${PROJECT_SOURCE_DIR}/LCNET/STUN
	${PROJECT_SOURCE_DIR}/LCNET/encoder
	${PROJECT_SOURCE_DIR}/LCNET/log
	${PROJECT_SOURCE_DIR}/LCNET/UDP
	${PROJECT_SOURCE_DIR}/LCNET/p2p
	${PROJECT_SOURCE_DIR}/LCNET
)

add_library(uuid-dev SHARED IMPORTED)
set_property(TARGET uuid-dev PROPERTY IMPORTED_LOCATION /usr/lib/x86_64-linux-gnu/libuuid.so)

add_executable(lcnet-test main.cpp ${SOURCES} ${HEADERS})

target_link_libraries(lcnet-test uuid-dev)
```


