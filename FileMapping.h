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
	HANDLE m_hFile;//打开的文件句柄
	HANDLE m_hMapFile;//文件映射句柄
	const unsigned char* m_pStart;//文件的起点映射至虚拟内存的位置
	const unsigned char* m_pEnd;//文件的终点映射至虚拟内存的位置
};
