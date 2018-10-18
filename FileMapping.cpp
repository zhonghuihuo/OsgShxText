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
#include "FileMapping.h"
#include <assert.h>
#include <stdio.h>
#include <string>

#if defined NDEBUG
#define TRACE( format, ... )
#else
#define TRACE( format, ... )   printf( "%s:%d " format, __FILE__, __LINE__, __VA_ARGS__ )
#endif

CFileMapping::CFileMapping()
{
	m_hFile = NULL;
	m_hMapFile = NULL;
	m_pStart = NULL;
}

CFileMapping::CFileMapping(const std::string& FileName)
{
	m_hFile = NULL;
	m_hMapFile = NULL;
	m_pStart = NULL;
	Open(FileName);
}

void CFileMapping::Open(const std::string& FileName)
{
	assert(m_hFile == NULL);
	m_hFile = CreateFileA(FileName.c_str(), // file to open
		GENERIC_READ,          // open for reading
		FILE_SHARE_READ,       // share for reading
		NULL,                  // default security
		OPEN_EXISTING,         // existing file only
		FILE_ATTRIBUTE_NORMAL, // normal file
		NULL);                 // no attr. template
	if (m_hFile == NULL || m_hFile == INVALID_HANDLE_VALUE) 
	{ 
		TRACE("Could not open file(%d).\n", GetLastError());
		m_hFile = NULL;
		return;
	}

	m_hMapFile = CreateFileMapping(
		m_hFile,				// use paging file
		NULL,                   // default security 
		PAGE_READONLY,         // read access
		0,    //the maximum size of the file-mapping object is equal to the current size of the file
		0,
		NULL);                // name of mapping object

	if (m_hMapFile == NULL || m_hMapFile == INVALID_HANDLE_VALUE) 
	{ 
		TRACE("Could not create file mapping object (%d).\n", GetLastError());
		m_hMapFile = NULL;
		return;
	}

	DWORD FileLen = GetFileSize(m_hFile, NULL);
	m_pStart = (const unsigned char*)MapViewOfFile(m_hMapFile,// handle to map object
		FILE_MAP_READ, // read permission
		0,                   
		0,                   
		FileLen);
	m_pEnd = m_pStart + FileLen;

	if (m_pStart == NULL) 
	{
		TRACE("Could not map view of file (%d).\n", GetLastError());
	}
}

CFileMapping::~CFileMapping(void)
{
	if(m_pStart)
		UnmapViewOfFile(m_pStart);

	if(m_hMapFile)
		CloseHandle(m_hMapFile);

	if(m_hFile)
		CloseHandle(m_hFile);
}
