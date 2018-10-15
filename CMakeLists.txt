SET(TEST_osgshxtext TRUE)

IF(TEST_osgshxtext)
	SET(TARGET_SRC
	    main.cpp
	    IGlyphCallback.h
		FileMapping.cpp
		FileMapping.h
		ShxFileMapping.cpp
		ShxFileMapping.h
		ShxParser.cpp
		ShxParser.h
		RegBigFontShxParser.h
		RegBigFontShxParser.cpp
		ShxText.cpp
		ShxText.h
	)

	#### end var setup  ###
	SETUP_EXAMPLE(osgshxtext)
ELSE()
	SET(TARGET_SRC
	    GdiGlyphCallback.cpp
	    IGlyphCallback.h
		FileMapping.cpp
		FileMapping.h
		ShxFileMapping.cpp
		ShxFileMapping.h
		ShxParser.cpp
		ShxParser.h
		RegBigFontShxParser.h
		RegBigFontShxParser.cpp
	)

	#### end var setup  ###
	SETUP_EXAMPLE(gdishxtext)
ENDIF()
