#pragma once

/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the author.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
#include <string>
typedef void *HANDLE;
class CFileMapping
{
public:
	CFileMapping();
	CFileMapping(const std::string& FileName);
	~CFileMapping(void);
	void Open(const std::string& FileName);

	const unsigned char* GetStart() const{ return m_pStart;}
	const unsigned char* GetEnd() const{ return m_pEnd;}
private:
	HANDLE m_hFile;// file handle
	HANDLE m_hMapFile;// file mapping handle
	const unsigned char* m_pStart;//memory address which maps file beginning
	const unsigned char* m_pEnd;//memory address which maps file end
};
