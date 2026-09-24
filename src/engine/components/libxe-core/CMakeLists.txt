
find_package(fmt CONFIG REQUIRED)

set (target "xe-core")

set (headers 
	"src/xe/XE.h"
	"src/xe/Predef.h"
	"src/xe/Buffer.h"
	"src/xe/Span.h"
	"src/xe/DataType.h"
	"src/xe/Core.h"
	"src/xe/Timer.h"
	"src/xe/FPSCounter.h"
		"src/xe/FileUtil.h"
	"src/xe/Logger.h"

    "src/xe/input/Input.h"
    "src/xe/input/InputManager.h"
    "src/xe/input/DeviceStatus.h"

	"src/xe/io/FileStream.h"
    "src/xe/io/FileStreamSource.h"
    "src/xe/io/MemoryStream.h"
    "src/xe/io/Stream.h"
    "src/xe/io/StreamSource.h"

	"src/xe/FPSCounter.h"

	"src/xe/BasicApplication.h"
	"src/xe/MessageBus.h" 
)

set (sources
	${headers}

	"src/xe/Buffer.cpp"
    "src/xe/DataType.cpp"
	"src/xe/Core.cpp"
	"src/xe/Timer.cpp"
	"src/xe/FPSCounter.cpp"
		"src/xe/FileUtil.cpp"
		"src/xe/Logger.cpp"

	"src/xe/input/DeviceStatus.cpp"
    "src/xe/input/InputManager.cpp"
	
	"src/xe/io/FileStream.cpp"
	"src/xe/io/FileStreamSource.cpp"
	"src/xe/io/MemoryStream.cpp"
	"src/xe/io/Stream.cpp"
	"src/xe/io/StreamSource.cpp"
	"src/xe/FPSCounter.cpp"
	"src/xe/BasicApplication.cpp"
	"src/xe/MessageBus.cpp"
)

add_library (${target} ${sources})
add_library(xe::core ALIAS ${target})

if (XE_ENABLE_PCH)
	target_precompile_headers(${target} PRIVATE ${headers})
endif()

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target}
		PUBLIC xe::math
		PRIVATE xe::interface
		PUBLIC fmt::fmt
)
