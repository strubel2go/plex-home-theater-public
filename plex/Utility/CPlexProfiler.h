/////////////////////////////////////////////////////////////////////////////////////////
// CPlexProfiler Class Definition
/////////////////////////////////////////////////////////////////////////////////////////
// Author :
//        LongChair - 2013
//
// Description :
//        When trying to profile Plex on RPi, the lack of proper tools to
//        efficiently get some figures on timing functions led me to create
//				this utility class. It will just time functions and subfunctions
//				when the macros are used and will then report the timing into
//				an hierachically organised text file
//
// Usage :
//        Drop PROFILE_START / PROFILE_END macros at the begining/end of the functions
//        you want to trace.
//        To use steps within on function you can use the step macros
//        PROFILE_STEP has to be included at the beginnig, once per scope
//        Then use PROFILE_STEP_START(<your message>) / PROFILE_STEP_END
//        The profiler can be enabled/disabled with g_profiler.Enable (true/false);
//        The profile results will be saved upon call of method PrintStats() into
//        a file called 'Profile.txt' in current directory.
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CPLEXPROFILER_H_
#define _CPLEXPROFILER_H_

#include <list>
#include <map>
#include "StdString.h"
#include "Stopwatch.h"
#include "log.h"
#include "stdio_utf8.h"

extern CStopWatch g_ProfileWatch;

// This class grabs information about a function call.
// will be created by the CPlexProfiler class and maintained under a tree form
// to get some proper profiling output.
class CProfiledFunction
{

  protected:
    CStdString  m_Name;
    float m_StartTime;
    float m_EndTime;
    float m_TotalTime;
    bool  m_bStarted;
    int   m_NumHits;

    std::list<CProfiledFunction*> ChildFunctions;
    CProfiledFunction *m_pParentFunction;

  public:
    CProfiledFunction(CStdString aName)
    {
      m_Name = aName;
      m_StartTime = m_EndTime =  m_TotalTime = 0;
      m_pParentFunction = NULL;
      m_bStarted = false;
      m_NumHits = 0;

    }

    ~CProfiledFunction() { Clear(); }

    inline float GetStartTime()       { return m_StartTime; }
    inline float GetEndTime()         { return m_EndTime; }
    inline float GetTotalTime()       { return m_TotalTime; }
    inline float GetElapsedTime()			{ return (m_EndTime - m_StartTime); }
    inline CStdString GetName()				{ return m_Name; }
    inline CProfiledFunction* GetParent()             { return m_pParentFunction; }
    inline void SetParent(CProfiledFunction *pParent) { m_pParentFunction = pParent; }

    inline static float GetTime()     { return g_ProfileWatch.GetElapsedSeconds();}

    CProfiledFunction *FindChild(CStdString aName);
    CProfiledFunction *AddChildFunction(CProfiledFunction *pFunc);
    inline CProfiledFunction *AddChildFunction(CStdString aName) { return AddChildFunction(new CProfiledFunction(aName));}

    void Start();
    void End();
    void Clear();
    void PrintStats(FILE* File,int Level);
};

// Profiler class, will handle the functions called within the code through macros
class CPlexProfiler
{
  protected:
    std::map<int,CProfiledFunction*> m_Root;
    std::map<int,CProfiledFunction*> m_CurrentFunction;
    bool m_Enabled;
    bool m_bStopWatchStarted;

  public:
    CPlexProfiler();
    ~CPlexProfiler();

    void Reset();
    void StartFunction(CStdString FunctionName);
    void EndFunction(CStdString FunctionName);
    void PrintStats();
    inline void Enable(bool State) { m_Enabled = State; }
};

extern CPlexProfiler g_Profiler;

inline CStdString GetClassMethod(const char *Text)
{
  CStdString strName = Text;
  unsigned int pos1 = strName.find("::");
  if (pos1!=CStdString::npos)
  {
    unsigned int pos2 = strName.rfind(" ",pos1);
    if (pos2!=CStdString::npos)
    {
      unsigned int pos3 = strName.find("(",pos1);
      if (pos3!=CStdString::npos)
      {
        return strName.substr(pos2,pos3-pos2);
      }
    }
  }
  return strName;
}

// Profiling Macros functions
#define PROFILER_ACTIVE 0

#if PROFILER_ACTIVE
#define PROFILE_START					g_Profiler.StartFunction(GetClassMethod(__PRETTY_FUNCTION__)+"()");
#define PROFILE_END						g_Profiler.EndFunction(GetClassMethod(__PRETTY_FUNCTION__)+"()");
#define PROFILE_STEP          CStdString fName;
#define PROFILE_STEP_START(format,...)	fName.Format("%s() - " format, GetClassMethod(__PRETTY_FUNCTION__).c_str(), __VA_ARGS__); g_Profiler.StartFunction(fName);
#define PROFILE_STEP_END	g_Profiler.EndFunction(fName);
#else
#define PROFILE_START                   ;
#define PROFILE_END                     ;
#define PROFILE_STEP                    ;
#define PROFILE_STEP_START(format,...)	;
#define PROFILE_STEP_END                ;
#endif

// internal debug macros
#if 0
#define PROFILE_DEBUG(format,...)		CLog::Log(LOGDEBUG,"PROFILE - %20s - %10X - " format, __PRETTY_FUNCTION__,(unsigned int)CThread::GetCurrentThreadId(), __VA_ARGS__);
#else
#define PROFILE_DEBUG(format,...) ;
#endif

#endif /* _CPLEXPROFILER_H_ */