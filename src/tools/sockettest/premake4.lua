project "client"
	kind "ConsoleApp"
    
	includedirs {
        APP_SRC.."/common/buffer",
	    APP_SRC.."/common/net", APP_SRC.."/common/net/channel",
		APP_SRC.."/common/net/handler",APP_SRC.."/common/net/handler/codec",
        "./"
    }
		
	files { "client.cpp"}
    
    links {
        "curl","buffer",
        "xynet","boost_sctp",
        "boost_thread-mt",
		"boost_date_time-mt",
		"boost_filesystem-mt",
		"boost_system-mt",
		"boost_coroutine-mt", "boost_context-mt",
		"pthread", "sctp" }

project "server"
	kind "ConsoleApp"
    
	includedirs {
        APP_SRC.."/common/buffer",
	    APP_SRC.."/common/net", APP_SRC.."/common/net/channel",
		APP_SRC.."/common/net/handler",APP_SRC.."/common/net/handler/codec",
        "./"
    }
		
	files { "server.cpp"}
    
    links {
        "curl",
        "xynet","buffer","boost_sctp",
        "boost_thread-mt",
		"boost_date_time-mt",
		"boost_filesystem-mt",
		"boost_system-mt",
		"boost_coroutine-mt", "boost_context-mt",
		"pthread", "sctp" }
