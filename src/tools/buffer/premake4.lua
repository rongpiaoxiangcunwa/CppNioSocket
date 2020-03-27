project "buffer_test"
	kind "ConsoleApp"
    
	includedirs {
	    APP_SRC.."/common/buffer",
        "./"
    }
		
	files { "*.cpp" }
    
    links {"gmock","gtest","gtest_main", "gmock_main", "pthread", "buffer",
    "boost_system-mt",
	"boost_thread-mt"}