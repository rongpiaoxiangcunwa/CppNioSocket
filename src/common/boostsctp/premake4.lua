project "boost_sctp"
    configuration "Debug or Release"
        kind "SharedLib"

    configuration "UnitTest"
        kind "StaticLib"

    configuration {}

	buildoptions {}
    
	includedirs {
    }
		
	files { "*.cpp" }