project "xynet"
    configuration "Debug or Release"
        kind "SharedLib"

    configuration "UnitTest"
        kind "StaticLib"

    configuration {}

	buildoptions {"-DDLOG_WITH_PTHREAD_ID", '-DHAVE_CONFIG_H' }
    
	includedirs {
	     APP_SRC.."/common/buffer", "./handler", "./handler/codec/", "./channel", "./"
    }
		
	files { "*.cpp",   "./handler/*.cpp", "./handler/codec/*.cpp", "./channel/*.cpp"}