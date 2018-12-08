#pragma once
#include "StdAfx.h"

class DffExporterClassDesc : public ::ClassDesc2
{
public:
	virtual int IsPublic() override;
	virtual void* Create(BOOL loading= FALSE) override;
	virtual const MCHAR* ClassName() override;
	virtual ::SClass_ID SuperClassID() override;
	virtual ::Class_ID ClassID() override;
	virtual const MCHAR* Category() override;
	virtual const MCHAR* InternalName() override;
	virtual HINSTANCE HInstance() override;

	static ::ClassDesc2* GetClassDescInstance();
};