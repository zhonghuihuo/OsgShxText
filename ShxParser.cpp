//#include "stdafx.h"
#include <Windows.h>
#include "ShxParser.h"
#include "ShxFileMapping.h"
#include <atlconv.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <gl/GL.h>
#include <assert.h>
#include "IGlyphCallback.h"

using namespace std;
#pragma warning(disable:4996)

#if defined(_DEBUG)
#define DEBUG_DRAW
#endif

#if defined(DEBUG_DRAW)
#define RED RGB(255,0,0)
#define GREEN RGB(0,255,0)
#define BLUE RGB(0,0,255)
#define BLACK RGB(0,0,0)
#define YELLOW RGB(255,255,0)
#define CYAN RGB(0,255,255)
#define PURPLE RGB(255,0,255)
#endif

#undef DrawText

void CShxParser::DrawLine(IGlyphCallback* pGlyphCallback)
{
	if(pGlyphCallback)
	{
		if(m_bDrawMode)
		{
			pGlyphCallback->glVertex2d(m_PenX, m_PenY);
		}
		else
		{
			pGlyphCallback->glEnd();
			pGlyphCallback->glBegin(GL_LINE_STRIP);
			pGlyphCallback->glVertex2d(m_PenX, m_PenY);
		}
	}
}

void DrawArc(IGlyphCallback* pGlyphCallback, double CenterX, double CenterY, double radius, double startAng/*in rad*/, double sweepAng/*in rad*/)
{
	int segCount = (int)ceil(fabs(sweepAng)*12.0 / M_PI);
	double detAng = sweepAng / segCount;
	double curAng = startAng;
	double pointX, pointY;
	pointX = CenterX + radius * cos(curAng);
	pointY = CenterY + radius * sin(curAng);
	pGlyphCallback->glEnd();
	pGlyphCallback->glBegin(GL_LINE_STRIP);
	pGlyphCallback->glVertex2d(pointX, pointY);
	for(int i=1; i<=segCount; ++i)
	{
		curAng += detAng;
		pointX = CenterX + radius * cos(curAng);
		pointY = CenterY + radius * sin(curAng);
		pGlyphCallback->glVertex2d(pointX, pointY);
	}
}

BOOL CalCRFromSEH(double start_x, double start_y, double end_x, double end_y,
		double chordHeight,double& center_x, double& center_y, double& radius)
{
	if(chordHeight == 0.0)
		return FALSE;
	double dx = end_x - start_x;
	double dy = end_y - start_y;
	if(fabs(dx) <= 1E-3 && fabs(dy) == 1E-3)
		return FALSE;
	double len = sqrt(dx*dx + dy*dy);
	double fabsH = fabs(chordHeight);
	radius = fabsH/2 + len*len/(8.0*fabsH);
	double CCDist;//distance of chord and center
	if(chordHeight > 0)
	{
		CCDist = chordHeight - radius;
	}
	else//chordHeight < 0
	{
		CCDist = chordHeight + radius;
	}
	double pp = CCDist/len;
	center_x = (start_x+end_x)/2.0 + dy*pp;
	center_y = (start_y+end_y)/2.0 - dx*pp;
	return true;
}

inline void Rad2Deg(float& x)
{
	x *= 180.0f/(float)M_PI;
}

#define ModifyAngles(S, C)\
{\
		S = - S;\
		C = - C;\
}

CShxParser::CShxParser()
{
	m_Type = UNKNOWN;
}

void CShxParser::Init(const char* FileName)
{
	m_Scale = 1;
	m_TextHeight = 1.0;
	m_DescendHeight = 0;
	m_bDrawMode = true;
	m_PenX = 0, m_PenY = 0;
	m_pShapeDefs = NULL;
	m_Type = UNKNOWN;
	if(FileName == NULL || *FileName == 0)
	{
		m_pStart = NULL;
		m_pEnd = NULL;
		return;
	}
	CShxFileMapping* pFileMapping = CShxFileMapping::AddRef(FileName);
	if(pFileMapping)
	{
		m_pStart = pFileMapping->GetStart();
		m_pEnd = pFileMapping->GetEnd();
		m_ShxFileName = FileName;
	}
	else
	{
		m_pStart = NULL;
		m_pEnd = NULL;
		return;
	}
	if(strnicmp((const char*)&m_pStart[11], "unifont", 7) == 0)
	{
		m_Type = UNIFONT;
		m_GlyphCount = *(unsigned short*)&m_pStart[25];
		m_pShapeDefs = &m_pStart[31];
		//跳过Ansi字体说明文字，含尾0
		while(*m_pShapeDefs != 0)
			++m_pShapeDefs;
		++m_pShapeDefs;
		m_FontHeight = *m_pShapeDefs;
		m_DescendHeight = *(m_pShapeDefs+1);
		if(m_FontHeight == 0)
			m_FontHeight = m_DescendHeight;
		m_pShapeDefs += 6;
	}
	else if(strnicmp((const char*)&m_pStart[11], "bigfont", 7) == 0)
	{
		m_Type = BIGFONT;
		m_IndexCount = *(unsigned short*)&m_pStart[27];
		int RangeCount = *(unsigned short*)&m_pStart[29];
		for(int i=0; i<RangeCount; ++i)
		{
			EscapeRange range;
			range.start = m_pStart[31+i*4];
			range.end = m_pStart[33+i*4];
			m_Ranges.push_back(range);
		}
		m_pIndice = (unsigned short*)&m_pStart[31+RangeCount*4];
		assert(*m_pIndice == 0);
		unsigned short ShapeDefLen = *(m_pIndice+1);
		unsigned int offset = *(unsigned int*)(m_pIndice+2);
		unsigned int AscendHeightPos = offset + ShapeDefLen - 4;
		m_FontHeight = *(m_pStart+AscendHeightPos);
		if(m_FontHeight == 0)
			m_FontHeight = *(m_pStart+AscendHeightPos+1);
		m_DescendHeight = 0;
	}
	else if(strnicmp((const char*)&m_pStart[11], "shapes", 6) == 0)
	{
		m_IndexCount = *(unsigned short*)&m_pStart[28];
		m_pIndice = (unsigned short*)&m_pStart[30];
		if(*m_pIndice == 0)
		{
			m_Type = REGFONT;
			int FirstShapDefLen = *(m_pIndice+1);
			const unsigned char* pFirstShapeDef = (const unsigned char*)m_pIndice + 4*m_IndexCount;
			const unsigned char* pAscendHeight = pFirstShapeDef + FirstShapDefLen - 4;
			m_FontHeight = *pAscendHeight;
			m_DescendHeight = *(pAscendHeight + 1);
			if(m_FontHeight == 0)
				m_FontHeight = m_DescendHeight;
		}
		else
		{
			m_Type = SHAPEFILE;
			m_FontHeight = 4;//随便给的一个值
		}
	}
	else
	{
		//m_Type = UNKNOWN;
		//return;
	}
}

CShxParser::CShxParser(const char* FileName)
{
	Init(FileName);
}

void CShxParser::Cleanup()
{
	if(!m_ShxFileName.empty())
		CShxFileMapping::Release(m_ShxFileName);
}

CShxParser::~CShxParser(void)
{
	Cleanup();
}

void CShxParser::ParseGlyph(IGlyphCallback* pGlyphCallback, int character)
{
	if(m_Type == UNIFONT)
	{
		const unsigned short* pChar = (const unsigned short*)m_pShapeDefs;
		int count = 0;
		while((*pChar != character) && (count < m_IndexCount))
		{
			++count;
			//跳过2字节形编号
			++pChar;
			int defbytes = *pChar;
			//跳过2字节形定义长度
			++pChar;
			//跳过defbytes字节的形名称和形定义
			pChar = (const unsigned short*)((char*)pChar + defbytes);
		}
		if(count >= m_IndexCount)
			return;//没有找到该字符

		++pChar;
		int defbytes = *pChar;
		++pChar;
		const unsigned char* pDefBytes = (const unsigned char*)pChar;
		//跳过形名字
		while(*pDefBytes != 0)
		{
			++pDefBytes;
			--defbytes;
		}
		++pDefBytes;
		--defbytes;
		ParseDefBytes(pGlyphCallback, pDefBytes, defbytes);
	}
	else if(m_Type == BIGFONT)
	{
		const unsigned short* pChar = m_pIndice;
		int count = 0;
		while((*pChar != character) && (count < m_IndexCount))
		{
			++count;
			//跳过8字节形索引项
			pChar += 4;
		}
		if(count >= m_IndexCount)
			return;//没有找到该字符

		unsigned int offset = *(unsigned int*)(pChar+2);
		int defbytes = *(pChar+1);
		const unsigned char* pDefBytes = (const unsigned char*)&m_pStart[offset];

		if(pDefBytes >= m_pEnd)
			return;

		//跳过形名称，如果存在
		while(*pDefBytes != 0)
		{
			++pDefBytes;
			--defbytes;
		}
		assert(*pDefBytes == 0);
		//跳过0x00
		++pDefBytes;
		--defbytes;
		ParseDefBytes(pGlyphCallback, pDefBytes, defbytes);
	}
	else if(m_Type == REGFONT || m_Type == SHAPEFILE)
	{
		const unsigned short* pChar = m_pIndice;
		int count = 0;
		unsigned int OffsetFromFirstShapeDef = 0;
		while((*pChar != character) && (count < m_IndexCount))
		{
			++count;
			//跳过2字节形编号
			++pChar;
			OffsetFromFirstShapeDef += *pChar;
			//跳过2字节形定义长度
			++pChar;
		}
		if(count >= m_IndexCount)
			return;//没有找到该字符

		int defbytes = *(pChar+1);
		const unsigned char* pFirstShapeDef = (const unsigned char*)m_pIndice + 4*m_IndexCount;
		const unsigned char* pDefBytes = &pFirstShapeDef[OffsetFromFirstShapeDef];

		if(pDefBytes >= m_pEnd)
			return;

		//跳过形名称，如果存在
		while(*pDefBytes != 0)
		{
			++pDefBytes;
			--defbytes;
		}
		assert(*pDefBytes == 0);
		//跳过0x00
		++pDefBytes;
		--defbytes;
		ParseDefBytes(pGlyphCallback, pDefBytes, defbytes);
	}
	else
	{
		//can't do anything.
	}
}

void CShxParser::ParseDefBytes(IGlyphCallback* pGlyphCallback, const unsigned char* pDefBytes, int defbytes)
{
	m_bDrawMode = true;//important! default value
	while(defbytes > 0)
	{
		ParseOneCode(pGlyphCallback, pDefBytes, defbytes);
	}
}

void CShxParser::ParseOneCode(IGlyphCallback* pGlyphCallback, const unsigned char*& pDefBytes, int& defbytes)
{
	unsigned char code = *pDefBytes;
	++pDefBytes;
	--defbytes;
	switch(code)
	{
	case 0://结束码
		return;
	case 1://绘制模式打开
		m_bDrawMode = true;
		break;
	case 2://绘制模式关闭
		m_bDrawMode = false;
		break;
	case 3://将比例系数除以下一个字节
		{
			unsigned char num = *pDefBytes;
			m_Scale /= num;
			//跳过下一个字节
			++pDefBytes;
			--defbytes;
		}
		break;
	case 4://将比例系数乘以下一个字节
		{
			unsigned char num = *pDefBytes;
			m_Scale *= num;
			//跳过下一个字节
			++pDefBytes;
			--defbytes;
		}
		break;
	case 5://把当前笔位置压栈
		m_PenPosStack.push(m_PenX);
		m_PenPosStack.push(m_PenY);
		break;
	case 6://弹出当前笔位置
		{
			int size = (int)m_PenPosStack.size();
			if(size > 1)//如果字体设计正确，本来不必加上这个保护的．
			{
				m_PenY = m_PenPosStack.top();
				m_PenPosStack.pop();
				m_PenX = m_PenPosStack.top();
				m_PenPosStack.pop();
				if (pGlyphCallback)
				{
					pGlyphCallback->glEnd();
					pGlyphCallback->glBegin(GL_LINE_STRIP);
					pGlyphCallback->glVertex2d(m_PenX, m_PenY);
				}

			}
		}
		break;
	case 7://绘制编号由下一个或两个字节指定的子形
		{
			//double tempScale = m_Scale;
			int character;
			if(m_Type == UNIFONT)
			{
				character = *(unsigned short*)pDefBytes;
				//跳过后两个字节
				pDefBytes += 2;
				defbytes -= 2;
			}
			else
			{
				character = *pDefBytes;
				//跳过后一个字节
				++pDefBytes;
				--defbytes;
			}
			ParseGlyph(pGlyphCallback, character);
			//m_Scale = tempScale;
		}
		break;
	case 8://后两个有符号字节表示移动的x，y距离
		Case_Code_8(pGlyphCallback, pDefBytes, defbytes);
		break;
	case 9://后面一系列两个有符号字节表示移动的x，y距离，直到（0，0）终止
		while(Case_Code_8(pGlyphCallback, pDefBytes, defbytes))
			;
		break;
	case 0xA://用下两个字节定义一个八分圆弧
		{
			int radius = *pDefBytes;
			++pDefBytes;
			--defbytes;
			char SC = *(char*)pDefBytes;
			++pDefBytes;
			--defbytes;
			int sign = 1;
			if(SC < 0)
			{
				sign = -1;
			}
			int S = (SC & 0x70)>>4;
			int C = (SC & 0x07) * sign;
			double r = radius*m_Scale;
			double SA = S*M_PI_4;
			double EA = (S + C)*M_PI_4;
			double CenterX = m_PenX - r*cos(SA);
			double CenterY = m_PenY - r*sin(SA);
			m_PenX = CenterX + r*cos(EA);
			m_PenY = CenterY + r*sin(EA);
			if(pGlyphCallback)
			{
				if(C == 0)
					C = 8;
				//不必判断m_bDrawMode，因为字体设计者不可能用这种复杂的方式移动笔
				DrawArc(pGlyphCallback, CenterX, CenterY, r, S*45.0/180.0*M_PI, C*45.0/180.0*M_PI);
			}
		}
		break;
	case 0xB://用下五个字节定义一个不规则圆弧
		{
			int start_offset = *pDefBytes;
			++pDefBytes;
			--defbytes;
			int end_offset = *pDefBytes;
			++pDefBytes;
			--defbytes;
			int high_radius = *pDefBytes;
			++pDefBytes;
			--defbytes;
			int radius = (*pDefBytes) + 256*high_radius;
			++pDefBytes;
			--defbytes;
			char SC = *(char*)pDefBytes;
			++pDefBytes;
			--defbytes;
			int sign = 1;
			if(SC < 0)
			{
				sign = -1;
			}
			double S = ((SC & 0x70)>>4) + sign*start_offset/256.0;
			double C = (SC & 0x07);
			if(C == 0)
				C = 8;
			if(end_offset != 0)
				C = C - 1;
			C = (C + (end_offset-start_offset)/256.0) * sign;
			double r = radius*m_Scale;
			double SA = S*M_PI_4;
			double EA = (S + C)*M_PI_4;
			double CenterX = m_PenX - r*cos(SA);
			double CenterY = m_PenY - r*sin(SA);
			m_PenX = CenterX + r*cos(EA);
			m_PenY = CenterY + r*sin(EA);
			if(pGlyphCallback)
			{
				//不必判断m_bDrawMode，因为字体设计者不可能用这种复杂的方式移动笔
				DrawArc(pGlyphCallback, CenterX, CenterY, r, S*45.0/180.0*M_PI, C*45.0/180.0*M_PI);
			}
		}
		break;
	case 0xC://（x，y，b）
		Case_Code_C(pGlyphCallback, pDefBytes, defbytes);
		break;
	case 0xD://（x，y，b），...,(0,0)
		while(Case_Code_C(pGlyphCallback, pDefBytes, defbytes))
			;
		break;
	case 0xE://下一代码被忽略
		{
			double X = m_PenX;
			double Y = m_PenY;
			bool bDrawMode = m_bDrawMode;
			ParseOneCode(false, pDefBytes, defbytes);
			m_bDrawMode = bDrawMode;//见Bold.shx::0x30就用14控制了可见性
			m_PenX = X;
			m_PenY = Y;
		}
		break;
	case 0xF:
		//assert(FALSE);
		break;
	default:
		{
			ParseLenDirByte(pGlyphCallback, code);
		}
		break;
	}
}

bool CShxParser::Case_Code_8(IGlyphCallback* pGlyphCallback, const unsigned char*& pDefBytes, int& defbytes)
{
	char* pChar = (char*)pDefBytes;
	int dx = (*pChar);
	m_PenX += m_Scale * dx;
	//跳过x
	++pDefBytes;
	--defbytes;
	pChar = (char*)pDefBytes;
	int dy = (*pChar);
	m_PenY += m_Scale * dy;
	//跳过y
	++pDefBytes;
	--defbytes;
	if((dx != 0) || (dy != 0))
	{
		DrawLine(pGlyphCallback);
		return true;
	}
	return false;
}

bool CShxParser::Case_Code_C(IGlyphCallback* pGlyphCallback, const unsigned char*& pDefBytes, int& defbytes)
{
	char* pChar = (char*)pDefBytes;
	double X = m_PenX;
	int dx_noscale = (*pChar);
	//跳过x
	++pDefBytes;
	--defbytes;
	pChar = (char*)pDefBytes;
	double Y = m_PenY;
	int dy_noscale = (*pChar);
	//跳过y
	++pDefBytes;
	--defbytes;

	if((dx_noscale != 0) || (dy_noscale != 0))
	{
		double dx = m_Scale * dx_noscale;
		double dy = m_Scale * dy_noscale;

		m_PenX += dx;
		m_PenY += dy;
		double dist = sqrt(dx*dx + dy*dy);
		if(pGlyphCallback)
		{
			//不必判断m_bDrawMode，因为字体设计者不可能用这种复杂的方式移动笔
			int bulge = *(char*)pDefBytes;
			double ChordHeight = bulge * (dist / 2.0) / 127.0;
			double radius;
			double center_x, center_y;
			BOOL bResult = CalCRFromSEH(X,Y, m_PenX, m_PenY, ChordHeight, center_x, center_y, radius);

			if(bResult)
			{
				float startAngle = atan2f((float)(Y-center_y), (float)(X-center_x));
				float sweepAngle = 2.0f*(float)asin((dist / 2.0)/radius);
				if(bulge < 0)
					sweepAngle = - sweepAngle;
				if (pGlyphCallback)
				{
					DrawArc(pGlyphCallback, center_x, center_y, radius, startAngle, sweepAngle);
				}
			}
			else
			{
				if (pGlyphCallback)
				{
					pGlyphCallback->glVertex2d(m_PenX, m_PenY);
				}
			}
		}
		//跳过b
		++pDefBytes;
		--defbytes;
		return true;
	}
	return false;
}

void CShxParser::ParseLenDirByte(IGlyphCallback* pGlyphCallback, unsigned char thebyte)
{
	int len = (thebyte & 0xF0) >> 4;
	int dir = thebyte &0xF;
	double x = 0, y = 0;
	switch(dir)
	{
	case 0:
		x = 1;
		y = 0;
		break;
	case 1:
		x = 1;
		y = 0.5;
		break;
	case 2:
		x = 1;
		y = 1;
		break;
	case 3:
		x = 0.5;
		y = 1;
		break;
	case 4:
		x = 0;
		y = 1;
		break;
	case 5:
		x = -0.5;
		y = 1;
		break;
	case 6:
		x = -1;
		y = 1;
		break;
	case 7:
		x = -1;
		y = 0.5;
		break;
	case 8:
		x = -1;
		y = 0;
		break;
	case 9:
		x = -1;
		y = -0.5;
		break;
	case 10:
		x = -1;
		y = -1;
		break;
	case 11:
		x = -0.5;
		y = -1;
		break;
	case 12:
		x = 0;
		y = -1;
		break;
	case 13:
		x = 0.5;
		y = -1;
		break;
	case 14:
		x = 1;
		y = -1;
		break;
	case 15:
		x = 1;
		y = -0.5;
		break;
	default:
		assert(false);
		break;
	}
	m_PenX += x*len*m_Scale;
	m_PenY += y*len*m_Scale;
	DrawLine(pGlyphCallback);
}



bool CShxParser::IsEscapeChar(unsigned char character)
{
	for(vector<EscapeRange>::size_type i=0; i<m_Ranges.size(); ++i)
	{
		if(character >= m_Ranges[i].start && character <= m_Ranges[i].end)
			return true;
	}
	return false;
}

double CShxParser::DrawText(IGlyphCallback* pGlyphCallback, const char* text, double x, double y)
{
	if(m_Type == UNIFONT)
	{
		USES_CONVERSION;
		wchar_t* wtext = A2W(text);
		DrawText(pGlyphCallback, wtext, x, y);
	}
	else if(m_Type == BIGFONT)
	{
		m_Scale = m_TextHeight/m_FontHeight;
		m_PenX = x;
		m_PenY = y;
		if(pGlyphCallback)
		{
			pGlyphCallback->glBegin(GL_LINE_STRIP);
			pGlyphCallback->glVertex2d(x, y);
		}
		while(*text != 0)
		{
			if(IsEscapeChar(*(unsigned char*)text))
			{
				//这里需要颠倒这两个字节，然后传给ParseGlyph
				char first =  *text;
				char second = *(text+1);
				unsigned short character = MAKEWORD(second, first);
				ParseGlyph(pGlyphCallback, /**(unsigned short*)text*/character);
				text += 2;
			}
			else
			{
				ParseGlyph(pGlyphCallback, *(unsigned char*)text);
				++text;
			}
		}
		if (pGlyphCallback)
		{
			pGlyphCallback->glEnd();
		}
	}
	else if(m_Type == REGFONT || m_Type == SHAPEFILE)
	{
		m_Scale = m_TextHeight/m_FontHeight;
		m_PenX = x;
		m_PenY = y;
		if(pGlyphCallback)
		{
			pGlyphCallback->glBegin(GL_LINE_STRIP);
			pGlyphCallback->glVertex2d(x, y);
		}
		while(*text != 0)
		{
			ParseGlyph(pGlyphCallback, *(unsigned char*)text);
			++text;
		}
		pGlyphCallback->glEnd();
	}
	else
	{
		//do nothing.
	}
	return m_PenX - x;
}

double CShxParser::DrawText(IGlyphCallback* pGlyphCallback, const wchar_t* text, double x, double y)
{
	if(m_Type == UNIFONT)
	{
		m_Scale = m_TextHeight/m_FontHeight;
		m_PenX = x;
		m_PenY = y;
		if(pGlyphCallback)
		{
			pGlyphCallback->glBegin(GL_LINE_STRIP);
			pGlyphCallback->glVertex2d(x, y);
		}
		while(*text != 0)
		{
			ParseGlyph(pGlyphCallback, *text++);
		}
		if (pGlyphCallback)
		{
			pGlyphCallback->glEnd();
		}
	}
	else if(m_Type == BIGFONT || m_Type == REGFONT  || m_Type == SHAPEFILE)
	{
		USES_CONVERSION;
		char* atext = W2A(text);
		DrawText(pGlyphCallback, atext, x, y);
	}
	else
	{
		//do nothing.
	}
	return m_PenX - x;
}

void CShxParser::ResetNextGlyph()
{
	if(m_Type == UNIFONT)
	{
		m_pCurrentShapeDef = m_pShapeDefs;
		m_CurrentCount = 0;
	}
	else if(m_Type == BIGFONT)
	{
		m_pCurrentIndice = m_pIndice + 4;
		m_CurrentCount = 1;
	}
	else if(m_Type == REGFONT || m_Type == SHAPEFILE)
	{
		m_pCurrentIndice = m_pIndice/* + 2*/;
		m_CurrentCount = 0/*1*/;
		m_NextCharDefOffsetFromFirstShapeDef = 0/**(m_pIndice+1)*/;
	}
}
bool CShxParser::ShowNextGlyph(IGlyphCallback* pGlyphCallback, double x, double y)
{
	++m_CurrentCount;
	int character = 0;
	m_Scale = m_TextHeight/m_FontHeight;
	m_PenX = x;
	m_PenY = y;
	if (pGlyphCallback)
	{
		pGlyphCallback->glEnd();
		pGlyphCallback->glBegin(GL_LINE_STRIP);
		pGlyphCallback->glVertex2d(x, y);
	}
	if(m_Type == UNIFONT)
	{
		const unsigned short* pChar = (const unsigned short*)m_pCurrentShapeDef;
		character = *pChar;
		//跳过2字节形编号
		++pChar;
		int defbytes = *pChar;
		//跳过2字节形定义长度
		++pChar;
		const unsigned char* pDefBytes = (const unsigned char*)pChar;
		//跳过形名字
		while(*pDefBytes != 0)
		{
			++pDefBytes;
			--defbytes;
		}
		++pDefBytes;
		--defbytes;
		ParseDefBytes(pGlyphCallback, pDefBytes, defbytes);

		m_pCurrentShapeDef = pDefBytes + defbytes;
	}
	else if(m_Type == BIGFONT)
	{
		character = *m_pCurrentIndice;
		if(character != 0)
		{
			int defbytes = *(m_pCurrentIndice+1);
			unsigned int offset = *(unsigned int*)(m_pCurrentIndice+2);
			const unsigned char* pDefBytes = (const unsigned char*)&m_pStart[offset];
			if(pDefBytes < m_pEnd)
			{
				//跳过形名称，如果存在
				while(*pDefBytes != 0)
				{
					++pDefBytes;
					--defbytes;
				}
				assert(*pDefBytes == 0);
				//跳过0x00
				++pDefBytes;
				--defbytes;
				ParseDefBytes(pGlyphCallback, pDefBytes, defbytes);
			}
		}

		m_pCurrentIndice += 4;
	}
	else if(m_Type == REGFONT || m_Type == SHAPEFILE)
	{
		character = *(unsigned char*)m_pCurrentIndice;
		int defbytes = *(m_pCurrentIndice+1);
		const unsigned char* pFirstShapeDef = (const unsigned char*)m_pIndice + 4*m_IndexCount;
		const unsigned char* pDefBytes = &pFirstShapeDef[m_NextCharDefOffsetFromFirstShapeDef];
		m_NextCharDefOffsetFromFirstShapeDef += defbytes;

		if(character != 0)
		{
			if(pDefBytes < m_pEnd)
			{
				//跳过形名称，如果存在
				while(*pDefBytes != 0)
				{
					++pDefBytes;
					--defbytes;
				}
				assert(*pDefBytes == 0);
				//跳过0x00
				++pDefBytes;
				--defbytes;
				ParseDefBytes(pGlyphCallback, pDefBytes, defbytes);
			}
		}

		m_pCurrentIndice += 2;
	}
	else
	{
		return false;
	}
	return m_CurrentCount < m_GlyphCount;
}
