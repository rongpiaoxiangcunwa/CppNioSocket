project "buffer"
    configuration "Debug or Release"
        kind "SharedLib"

    configuration "UnitTest"
        kind "StaticLib"

    configuration {}

	buildoptions {}
    
	includedirs {
	    APP_SRC.."/common/buffer"
    }
		
	files { "*.cpp" }