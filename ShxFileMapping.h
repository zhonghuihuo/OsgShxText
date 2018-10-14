#pragma once
#include <vector>
#include "FileMapping.h"

class CShxFileMapping : public CFileMapping
{
public:
	//ShxFileName必须不带路径信息，会自动在EXE所在目录\AutoCAD\Fonts目录下寻找。
	//每次调用本函数，都会增加对返回值指向的对象的引用，因此在用完该对象后，请调用
	//Release()
	static CShxFileMapping* AddRef(const std::string& ShxFileName);
	//减少引用计数，当引用计数为0时，从表中删除该对象的指针，并且删除该对象
	static void Release(const std::string& ShxFileName);
private:
	//ShxFileName必须带全路径信息
	CShxFileMapping(const std::string& ShxFileName);
	~CShxFileMapping(void);
	std::string m_ShxFileName;//不带路径信息
	int m_RefCount;//本对象的引用计数
	//文件映射对象的数组
	static std::vector<CShxFileMapping*> s_FileMappings;
};
