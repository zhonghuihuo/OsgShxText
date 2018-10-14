#pragma once

#include <vector>
#include <stack>
#include "IGlyphCallback.h"

enum SHX_TYPE {REGFONT, UNIFONT, BIGFONT, SHAPEFILE, UNKNOWN};
struct EscapeRange
{
	unsigned char start;
	unsigned char end;
};

#pragma push_macro("DrawText")
#undef DrawText

class CShxFileMapping;
class CShxParser
{
public:
	//ShxFileName必须为有效的ACAD字体文件
	//ShxFileName必须不带路径信息
	CShxParser(const char* ShxFileName);
	CShxParser();
	void Init(const char* ShxFileName);
	void Cleanup();
	~CShxParser(void);
	//设置要绘制的字高
	inline void SetTextHeight(double height);
	inline double GetTextHeight();
	//在当前字高和字体下,text的宽度
	inline double GetTextExtent(const char* text);
	inline double GetTextExtent(const wchar_t* text);
	//以(x,y)为左下角点在hdc上绘制文本text.
	double DrawText(IGlyphCallback* pGlyphCallback, const char* text, double x, double y);
	double DrawText(IGlyphCallback* pGlyphCallback, const wchar_t* text, double x, double y);
	SHX_TYPE GetType(){return m_Type;}
private:
	//计算单个字符的宽度,如果hdc非0，绘制该字符
	void ParseGlyph(IGlyphCallback* pGlyphCallback, int character);
	void ParseDefBytes(IGlyphCallback* pGlyphCallback, const unsigned char* pDefBytes, int defbytes);
	void ParseOneCode(IGlyphCallback* pGlyphCallback, const unsigned char*& pDefBytes, int& defbytes);
	bool Case_Code_8(IGlyphCallback* pGlyphCallback, const unsigned char*& pDefBytes, int& defbytes);
	bool Case_Code_C(IGlyphCallback* pGlyphCallback, const unsigned char*& pDefBytes, int& defbytes);
	void ParseLenDirByte(IGlyphCallback* pGlyphCallback, unsigned char thebyte);
	bool IsEscapeChar(unsigned char character);
	void DrawLine(IGlyphCallback* pGlyphCallback);


	double m_TextHeight;//文字绘制时上高
	unsigned int m_DescendHeight;//字体文件内部记录的字体下高
	unsigned int m_FontHeight;//字体文件内部记录的字体高度
	double m_Scale;//长度比例系数
	SHX_TYPE m_Type;//Shx文件类型
	std::vector<EscapeRange> m_Ranges;//如果是BIGFONT字体,m_Ranges会记录转义码范围
	bool m_bDrawMode;//在分析定义字节过程中，控制是否绘制的标志
	double m_PenX, m_PenY;//当前笔的位置,在比例调整前或字形定义结束前更新,见ParseOneCode方法
	std::stack<double> m_PenPosStack;//笔位置栈
	const unsigned char* m_pStart;//文件的起点映射至虚拟内存的位置
	const unsigned char* m_pEnd;//文件的终点映射至虚拟内存的位置
	union
	{
		const unsigned char* m_pShapeDefs;//如果是UNIFONT shx文件,指向第一个形定义处
		const unsigned short* m_pIndice;//如果是REGFONT或者BIGFONT shx文件,指向形定义的索引表的第一项
	};
	union
	{
		int m_IndexCount;//索引项数
		int m_GlyphCount;//字符数目
	};
	std::string m_ShxFileName;

public:
	//下面的两个函数用于遍历字体文件中所有字符。
	void ResetNextGlyph();//从第一个字符开始显示
	//如果返回false，表示显示完毕。
	bool ShowNextGlyph(IGlyphCallback* pGlyphCallback, double x, double y);
private:
	union
	{
		const unsigned char* m_pCurrentShapeDef;//如果是UNIFONT shx文件,指向下一个要显示的形定义处
		const unsigned short* m_pCurrentIndice;//如果是REGFONT或者BIGFONT shx文件,指向形定义的索引表的下一个要显示的项
	};
	int m_NextCharDefOffsetFromFirstShapeDef;//仅用于REGFONT文件
	int m_CurrentCount;
	friend class CRegBigFontShxParser;
};

void CShxParser::SetTextHeight(double height)
{
	//assert(height > 0.0);
	m_TextHeight = height>0.0f?height:-height;
}

double CShxParser::GetTextHeight()
{
	return m_TextHeight;
}

double CShxParser::GetTextExtent(const char* text)
{
	return DrawText(nullptr, text, 0, 0);
}

double CShxParser::GetTextExtent(const wchar_t* text)
{
	return DrawText(nullptr, text, 0, 0);
}
#pragma pop_macro("DrawText") 