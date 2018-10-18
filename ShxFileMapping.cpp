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
#include "ShxFileMapping.h"
#include <algorithm>
using namespace std;
#define THIS void
#include <atlbase.h>
#if defined NDEBUG
#define TRACE( format, ... )
#else
#define TRACE( format, ... )   printf( "%s:%d " format, __FILE__, __LINE__, __VA_ARGS__ )
#endif


std::vector<CShxFileMapping*> CShxFileMapping::s_FileMappings;

CShxFileMapping::CShxFileMapping(const std::string& ShxFileName)
	:CFileMapping(ShxFileName)
{
	m_RefCount = 0;
}

CShxFileMapping::~CShxFileMapping(void)
{
}

CShxFileMapping* CShxFileMapping::AddRef(const std::string& ShxFileName)
{
	CShxFileMapping* pObj;
	for(int i=0; i<(int)s_FileMappings.size(); ++i)
	{
		pObj = s_FileMappings[i];
		if(pObj->m_ShxFileName == ShxFileName)
		{
			++pObj->m_RefCount;
			return pObj;
		}
	}

	char FileName[512];
	GetModuleFileName(NULL, FileName, sizeof(FileName));
	char* pos = _tcsrchr(FileName, _T('\\'));
	*pos = 0;
	strcat(FileName, "\\ACAD\\Fonts\\");
	strcat(FileName, ShxFileName.c_str());

	pObj = new CShxFileMapping(FileName);
	if(pObj->GetStart() == NULL)
	{
		delete pObj;
		TRACE("Can't open AutoCAD font file: %s\n", FileName);
		return NULL;
	}
	pObj->m_ShxFileName = ShxFileName;
	++pObj->m_RefCount;
	s_FileMappings.push_back(pObj);
	return pObj;
}

void CShxFileMapping::Release(const std::string& ShxFileName)
{
	CShxFileMapping* pObj;
	for(vector<CShxFileMapping*>::iterator it = s_FileMappings.begin();
		it != s_FileMappings.end(); ++it)
	{
		pObj = *it;
		if(pObj->m_ShxFileName == ShxFileName)
		{
			--pObj->m_RefCount;
			if(0 == pObj->m_RefCount)
			{
				s_FileMappings.erase(it);
				delete pObj;
			}
			break;
		}
	}
}
