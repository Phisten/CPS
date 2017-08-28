#ifdef CPS_MARK_DLL_EXPORTS
#define CPS_MARK_DLL_API __declspec(dllexport) 
#else
#define CPS_MARK_DLL_API __declspec(dllimport) 
#endif

#include "stdafx.h"
#include <mutex>

class CPS_Mark
{
public:
	CPS_MARK_DLL_API struct MarkInfo
	{
		int Id = -1;
		int Angle = 0;
		int Distance = 0;
		int CenterX = 0;
		int CenterY = 0;
	};
	CPS_MARK_DLL_API CPS_Mark();
	CPS_MARK_DLL_API ~CPS_Mark();
	CPS_MARK_DLL_API bool CPS_Mark::IsRunning();
	CPS_MARK_DLL_API CPS_Mark::MarkInfo CPS_Mark::GetMarkInfo();
	CPS_MARK_DLL_API static void CPS_Mark::DetectCardProcess(CPS_Mark*, int);
private:
	MarkInfo markInfo;
	std::mutex mutex1;

	bool running = false;
	void CPS_Mark::UpdateMark(int,int,int,int,int);
};

