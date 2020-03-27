newoption {
	trigger     = "no-building-highlight",
	description = "no color highlight in building log"
}

ROOT_DIR=os.getcwd()
APP_HEADER = ROOT_DIR.."/src/include"
APP_SRC = ROOT_DIR.."/src"
config = 'debug64'
THIRD_PARTY_LIB = ROOT_DIR.."/third_party_libs/libs"
solution "nio_framework"
    language "C++"
    
	includedirs {
		APP_HEADER,		
		APP_SRC
	}

    configurations { "Debug", "Release", "UnitTest" }
        buildoptions { "-Dx86", "-D_REENTRANT", "-D_POSIX_PTHREAD_SEMANTICS", "-Wall", "-DDLOG_WITH_PTHREAD_ID", "-DUSE_BOOST_1_68", "-DBOOST_SYSTEM_ENABLE_DEPRECATED",
        "-DBOOST_ASIO_ENABLE_OLD_SERVICES"}
        configuration "Debug"
        buildoptions { "-g", "-std=c++11"}
     
    configuration "UnitTest"
      buildoptions { "-fprofile-arcs", "-ftest-coverage", "-D_UNIT_TEST_COVERAGE_", "-fPIC", "-g", "-ggdb" }
      linkoptions { "-fprofile-arcs", "-fPIC" }
    
    platforms { "x64" }
    
    configuration "x64"
    	includedirs {"/usr/lib64/glib-2.0/include",ROOT_DIR.."/third_party_libs/include"}
        TARGETDIR = ROOT_DIR.."/bin/x86_64"
    	libdirs { THIRD_PARTY_LIB,  TARGETDIR }
        buildoptions {"-O2"}
        linkoptions {"-Wl,-rpath "}
    	targetdir (TARGETDIR)
    
    configuration "no-building-highlight"                         
        buildoptions { "-fno-diagnostics-color" }    	   

    configuration "Debug or UnitTest"
        flags { "Symbols" }
    configuration "Release"                         
        flags { "Optimize" }    	   
    location "Build"
	
    dofile( "src/common/net/premake4.lua")
    dofile( "src/common/buffer/premake4.lua")
    dofile( "src/common/boostsctp/premake4.lua")
    dofile( "src/tools/buffer/premake4.lua")
    dofile( "src/tools/sockettest/premake4.lua")
