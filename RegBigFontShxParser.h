#pragma once
#include "ShxParser.h"
#include "IGlyphCallback.h"

#pragma push_macro("DrawText")
#undef DrawText

class CRegBigFontShxParser
{
public:
	//ShxFileName必须不带路径信息，会自动在AutoCAD的安装目录下的Fonts目录下寻找
	CRegBigFontShxParser(const char* Reg_Uni_ShxFile, const char* Big_ShxFile);
	CRegBigFontShxParser(){}
	void Init(const char* Reg_Uni_ShxFile, const char* Big_ShxFile);
	void Cleanup();
	~CRegBigFontShxParser(void);
	//设置要绘制的字高
	inline void SetTextHeight(double height);
	inline double GetTextHeight();
	inline double GetDescendHeight();
	//在当前字高和字体下,text的宽度
	inline double GetTextExtent(const char* text);
	inline double GetTextExtent(const wchar_t* text);
	//以(x,y)为左下角点在hdc上绘制文本text.
	double DrawText(IGlyphCallback* pGlyphCallback, const char* text, double x, double y);
	double DrawText(IGlyphCallback* pGlyphCallback, const wchar_t* text, double x, double y);
private:
	CShxParser m_RegFontShx;
	CShxParser m_BigFontShx;
};

void CRegBigFontShxParser::SetTextHeight(double height)
{
	//assert(height > 0.0);
	m_RegFontShx.SetTextHeight(height);
	m_BigFontShx.SetTextHeight(height);
}

double CRegBigFontShxParser::GetTextHeight()
{
	return m_RegFontShx.GetTextHeight();
}

double CRegBigFontShxParser::GetDescendHeight()
{
	return m_RegFontShx.m_DescendHeight * m_RegFontShx.m_TextHeight / m_RegFontShx.m_FontHeight; 
}

double CRegBigFontShxParser::GetTextExtent(const char* text)
{
	return DrawText(false, text, 0, 0);
}

double CRegBigFontShxParser::GetTextExtent(const wchar_t* text)
{
	return DrawText(false, text, 0, 0);
}
#pragma pop_macro("DrawText") 