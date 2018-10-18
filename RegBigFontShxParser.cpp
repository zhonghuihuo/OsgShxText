/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the author.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
//#include "stdafx.h"
#include <Windows.h>
#include "RegBigFontShxParser.h"
#include <atlconv.h>
#include <gl/GL.h>
#include <assert.h>
#include "IGlyphCallback.h"

#undef DrawText

CRegBigFontShxParser::CRegBigFontShxParser(const char* Reg_Uni_ShxFile, const char* Big_ShxFile)
:m_RegFontShx(Reg_Uni_ShxFile)
,m_BigFontShx(Big_ShxFile)
{
	if(m_RegFontShx.m_Type == SHAPEFILE)
		assert(m_BigFontShx.m_Type == UNKNOWN);
	else
	{
		assert(m_RegFontShx.m_Type == UNKNOWN || m_RegFontShx.m_Type == REGFONT || m_RegFontShx.m_Type == UNIFONT);
		assert(m_BigFontShx.m_Type == UNKNOWN || m_BigFontShx.m_Type == BIGFONT);
	}
}

void CRegBigFontShxParser::Init(const char* Reg_Uni_ShxFile, const char* Big_ShxFile)
{
	m_RegFontShx.Init(Reg_Uni_ShxFile);
	m_BigFontShx.Init(Big_ShxFile);

	if(m_RegFontShx.m_Type == SHAPEFILE)
		assert(m_BigFontShx.m_Type == UNKNOWN);
	else
	{
		assert(m_RegFontShx.m_Type == UNKNOWN || m_RegFontShx.m_Type == REGFONT || m_RegFontShx.m_Type == UNIFONT);
		assert(m_BigFontShx.m_Type == UNKNOWN || m_BigFontShx.m_Type == BIGFONT);
	}
}

void CRegBigFontShxParser::Cleanup()
{
	m_RegFontShx.Cleanup();
	m_BigFontShx.Cleanup();
}

CRegBigFontShxParser::~CRegBigFontShxParser(void)
{
}

double CRegBigFontShxParser::DrawText(IGlyphCallback* pGlyphCallback, const char* text, double x, double y)
{
	//sync pen position in 2 files
	m_RegFontShx.m_PenX = x;
	m_RegFontShx.m_PenY = y;
	m_BigFontShx.m_PenX = x;
	m_BigFontShx.m_PenY = y;
	//initialize scale
	m_BigFontShx.m_Scale = m_BigFontShx.m_TextHeight/m_BigFontShx.m_FontHeight;
	m_RegFontShx.m_Scale = m_RegFontShx.m_TextHeight/m_RegFontShx.m_FontHeight;

	if (pGlyphCallback) {
		pGlyphCallback->glBegin(GL_LINE_STRIP);
		pGlyphCallback->glVertex2d(x, y);
	}

	while(*text != 0)
	{
		if(m_BigFontShx.m_Type != UNKNOWN && m_BigFontShx.IsEscapeChar(*(unsigned char*)text))
		{
			//here we need revert 2 bytes, then pass to ParseGlyph
			char first =  *text;
			char second = *(text+1);
			unsigned short character = MAKEWORD(second, first);
			m_BigFontShx.ParseGlyph(pGlyphCallback, /**(unsigned short*)text*/character);
			text += 2;
			//sync pen position in 2 files
			m_RegFontShx.m_PenX = m_BigFontShx.m_PenX;
			m_RegFontShx.m_PenY = m_BigFontShx.m_PenY;
			//sync scale in 2 files
			m_RegFontShx.m_Scale = (m_RegFontShx.m_TextHeight/m_RegFontShx.m_FontHeight)*
				(m_BigFontShx.m_Scale / (m_BigFontShx.m_TextHeight/m_BigFontShx.m_FontHeight));
		}
		else
		{
			m_RegFontShx.ParseGlyph(pGlyphCallback, *(unsigned char*)text);
			++text;
			//sync pen position in 2 files
			m_BigFontShx.m_PenX = m_RegFontShx.m_PenX;
			m_BigFontShx.m_PenY = m_RegFontShx.m_PenY;
			//sync scale in 2 files
			m_BigFontShx.m_Scale = (m_BigFontShx.m_TextHeight/m_BigFontShx.m_FontHeight)*
				(m_RegFontShx.m_Scale / (m_RegFontShx.m_TextHeight/m_RegFontShx.m_FontHeight));
		}
	}
	if (pGlyphCallback)
	{
		pGlyphCallback->glEnd();
	}

	return m_RegFontShx.m_PenX - x;
}

double CRegBigFontShxParser::DrawText(IGlyphCallback* pGlyphCallback, const wchar_t* text, double x, double y)
{
	USES_CONVERSION;
	char* atext = W2A(text);
	return DrawText(pGlyphCallback, atext, x, y);
}
