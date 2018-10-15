#pragma once

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
