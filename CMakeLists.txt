SET(TARGET_SRC
    main.cpp
    IGlyphCallback
    # CoordinateTranformer.h
    # CoordinateTranformer.cpp
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
