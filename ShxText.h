#pragma once
#include <string>
#include <vector>
#include <osg/Geometry>
#include <osg/BoundingBox>
#include "IGlyphCallback.h"

class ShxText : 
	public osg::Geometry, private IGlyphCallback
{
public:
	ShxText();
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	ShxText(const ShxText& pg, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
	~ShxText();
    virtual osg::Object* cloneType() const { return new ShxText(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new ShxText(*this, copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const ShxText*>(obj) != NULL; }
    virtual const char* className() const { return "ShxText"; }
    virtual const char* libraryName() const { return "osgText"; }

    float length() const;

	// Ensure color component is in [0, 1]
    void setColor(const osg::Vec3& color);
    const osg::Vec3& getColor() const { return _color; }

	const std::string& GetRegFontFile() const { return m_RegFontFile; }
	const std::string& GetBigFontFile() const { return m_BigFontFile; }
	void SetFontFile(const char* Reg_Uni_ShxFile, const char* Big_ShxFile);

	const std::wstring& GetText() const { return _text; }
	void SetText(const std::wstring& text);

    /** Set the rendered character size in object coordinates.*/
    void setCharacterSize(float height);

    /** Set the rendered character size in object coordinates.*/
    void setCharacterSize(float height, float widthRatio);

    float getCharacterHeight() const { return _characterHeight; }
    float getCharacterAspectRatio() const { return _widthRatio; }

    enum CharacterSizeMode
    {
        OBJECT_COORDS, /// default
        SCREEN_COORDS, /// internally scale the characters to be constant screen size.
        OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT /// text that behavaves like OBJECT_COORDS sized text when a long distance way, but has its screen sized capped automatically when the viewer gets near.
    };

    /** Set how the CharacterSize value relates to the final rendered character.*/
    void setCharacterSizeMode(CharacterSizeMode mode) { _characterSizeMode = mode; }

    /** Get the CharacterSizeMode.*/
    CharacterSizeMode getCharacterSizeMode() const { return _characterSizeMode; }

    /** Set the line spacing of the text box, given as a percentage of
    * the character height. The default value is 0 for backward
    * compatibility. For longer paragraphs of text, a value of at
    * least 25% (i.e. set line spacing to 0.25) is recommended. */
    void setLineSpacing(float lineSpacing);

    /** Get the line spacing of the text box. */
    float getLineSpacing() const { return _lineSpacing; }



    /** Set the position of text.*/
    void setPosition(const osg::Vec3& pos);

    /** Get the position of text.*/
    const osg::Vec3& getPosition() const { return _position; }


    enum AlignmentType
    {
        LEFT_TOP,
        LEFT_CENTER,
        LEFT_BOTTOM,

        CENTER_TOP,
        CENTER_CENTER,
        CENTER_BOTTOM,

        RIGHT_TOP,
        RIGHT_CENTER,
        RIGHT_BOTTOM,
    };

    void setAlignment(AlignmentType alignment);
    AlignmentType getAlignment() const { return _alignment; }

    void setRotation(const osg::Quat& quat);
    const osg::Quat& getRotation() const { return _rotation; }

    void setAutoRotateToScreen(bool autoRotateToScreen);
    bool getAutoRotateToScreen() const { return _autoRotateToScreen; }

    /** Get the number of wrapped lines - only valid after computeGlyphRepresentation() has been called, returns 0 otherwise */
    unsigned int getLineCount() const { return _lineCount; }

    /** Get the cached internal matrix used to provide positioning of text.  The cached matrix is originally computed by computeMatrix(..). */
    const osg::Matrix& getMatrix() const { return _matrix; }

    /** compute the matrix that positions the text in model space for the given viewpoint.*/
    bool computeMatrix(osg::Matrix& matrix, osg::State* state = 0) const;
    

    osg::BoundingBox computeBoundingBox() const;

    void drawImplementation(osg::RenderInfo& renderInfo) const;

private:

    // members which have public access.
    osg::Vec3                               _color;
	std::string                             m_RegFontFile;
	std::string                             m_BigFontFile;
    float                                   _widthRatio;
    float                                   _characterHeight;
    CharacterSizeMode                       _characterSizeMode;
    float                                   _lineSpacing;

    std::wstring                            _text;
    osg::Vec3                               _position;
    AlignmentType                           _alignment;
    osg::Quat                               _rotation;
    bool                                    _autoRotateToScreen;
    unsigned int                            _lineCount;

    mutable osg::Matrix                     _matrix;
    typedef osg::ref_ptr<osg::Vec2Array> Coords;
    mutable Coords                          _coords;
    mutable osg::ref_ptr<osg::Vec3Array>    _colors;
	mutable osg::ref_ptr<osg::MultiDrawArrays> m_primitiveSet;

	mutable bool m_EmGlyphLengthValid;
	mutable bool m_CoordsValid;
	mutable bool m_ColorsValid;
	mutable bool m_MatrixValid;
	mutable double m_EmGlyphLength;
	static const float m_EmHeight;
	mutable size_t m_startIndex;

	//在变换之前，文字对齐点在(0,0,0)，文字方向为(1,0,0)，法线方向为(0,0,1)。
	void calEmGlyph() const;
	// when character height is m_EmHeight
	osg::Vec3f emLeftBottom() const;
	// when character height is m_EmHeight
	float emLength() const;

	friend class CRegBigFontShxParser;
	friend class CShxParser;
    friend class ShxTextUpdateCallback;

	/** method to invoke to rebuild the geometry that renders the text.*/
	void build();
	void glBegin(int mode);
	void glVertex2d(double x, double y);
	void glEnd();
};
