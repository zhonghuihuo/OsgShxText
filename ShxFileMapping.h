#pragma once
#include <vector>
#include "FileMapping.h"

class CShxFileMapping : public CFileMapping
{
public:
	//ShxFileName must be file name without directory. search under \AutoCAD\Fonts relative to program.
	//increase reference count to CShxFileMapping, so call Release to balance it after using CShxFileMapping
	static CShxFileMapping* AddRef(const std::string& ShxFileName);
	//decrease reference count to CShxFileMapping, when reference count is 0, delete its CShxFileMapping object
	static void Release(const std::string& ShxFileName);
private:
	//ShxFileName must be absolute path
	CShxFileMapping(const std::string& ShxFileName);
	~CShxFileMapping(void);
	std::string m_ShxFileName;//must be file name without directory
	int m_RefCount;//reference count of this object
	static std::vector<CShxFileMapping*> s_FileMappings;
};
