/*
 * P R O F I L E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

//  This file must be compiled with the -ep (call
//  prolog hook routine) and -ee (call epilog hook routine)
//  options *disabled*

#include "base/IProgressReporter.hpp"
#include "base/diag.hpp"

#include <csignal>
#include <ctime>
#include <cstring>

// TODO: windows libs
#ifdef _MSC_VER // Windows
#include <windows.h>
#include <direct.h>
#include <conio.h>
#include <process.h> // _beginthread, _endthread
// #include <dos.h>
#else // Linux
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif /*_MSC_VER*/

#include "profiler/profiler.hpp"
#include "base/logbuff.hpp"

// TODO: handle timer
static size_t count;
static size_t call_stack;

static uint32 next_update_time_ls;
static uint32 next_update_time_ms;
static uint32 time_increment_ls;
static uint32 time_increment_ms;

static bool profiling_enabled;

void ProProfilerAnchor();

extern "C" { /*
               extern  size_t  count;
               extern  size_t  call_stack;

               extern  size_t  next_update_time_ls;
               extern  size_t  next_update_time_ms;
               extern  uint32  time_increment_ls;
               extern  uint32  time_increment_ms;

               extern  uint8   profiling_enabled;
           */
#ifdef _MSC_VER
void checkForProProfilerShutdown(void* dummy);
#else
void* checkForProProfilerShutdown(void* dummy);
#endif // _MSC_VER
}

unsigned int RDTSC(uint32[2]);
#pragma aux RDTSC = 0x0f 0x31 /* RDTSC */                                                                              \
    " mov [esi],eax   "                                                                                                \
    " mov [esi+4],edx " parm[esi] modify[eax edx] value[eax];

// static
ProProfiler& ProProfiler::instance(IProgressReporter* pReporter)
{
    static ProProfiler instance_;
    if (!instance_.isInitialized())
    {
        // The profiler is broken (it does not set ticksPerSecond_). Disable for now.
        // instance_.init(pReporter);
    }

    return instance_;
}

ProProfiler::ProProfiler()
    : traceIntervalSeconds_(50.0 / 1000.0)
    , outputStream_("profiler.dat")
{
}

void ProProfiler::init(IProgressReporter* pReporter)
{
    calibrate(pReporter);

    // ProProfilerAnchor();
    registerAnchor("ProProfilerAnchor");

    setupTraceInterval();

    // If this enviroment variable is set, lauch a separate thread
    // which checks for the presence of a stop.now file.
    const char* launchThread = getenv("CB_STOP_NOW");

    if (launchThread)
#ifdef _WIN32
    //_beginthread( checkForProProfilerShutdown, 1024, NULL );
#else
        pthread_create(&threadId, nullptr, checkForProProfilerShutdown, nullptr);
#endif // _MSC_VER

        TEST_INVARIANT;
}

bool ProProfiler::isInitialized() const
{
    return ticksPerSecond_;
}

ProProfiler::~ProProfiler()
{
    TEST_INVARIANT;
    delete pMemoryBuffer_;
}

void ProProfiler::traceInterval(double timeSeconds)
{
    PRE(! traceIntervalFixed());

    traceIntervalSeconds_ = timeSeconds;

    setupTraceInterval();
}

void ProProfiler::enableProfiling()
{
    profiling_enabled = true;

    traceIntervalFixed_ = true;

    uint32 currentTime[2];
    // RDTSC( currentTime );

    next_update_time_ls = currentTime[0];
    next_update_time_ms = currentTime[1];

    POST(traceIntervalFixed());
}

void ProProfiler::disableProfiling()
{
    profiling_enabled = false;
}

bool ProProfiler::traceIntervalFixed() const
{
    return traceIntervalFixed_;
}

void ProProfiler::setupTraceInterval() const
{
    double ticks = traceIntervalSeconds_ * ticksPerSecond_;

    doubleToUint64(ticks, &time_increment_ls, &time_increment_ms);

    //     const double TWO_POWER_32 =
    //         2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 *
    //         2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 *
    //         2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 *
    //         2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0;
    //
    //     time_increment_ms = ticks / TWO_POWER_32;
    //     time_increment_ls = ticks - time_increment_ms * TWO_POWER_32;
}

std::ostream& ProProfiler::outputStream()
{
    return outputStream_;
}

void ProProfiler::closeOutputStream()
{
    outputStream_.close();
}

void ProProfiler::calibrate(IProgressReporter* pReporter)
{
    uint32 startTicks[2]{};
    uint32 endTicks[2]{};

    const double calibrationTimeSeconds = 1.0;
    const size_t minClocksToWait = calibrationTimeSeconds * CLOCKS_PER_SEC;

    //  First wait for the clock to change to get a reasonably
    //  accurate base point.
    clock_t startClockValue = clock();

    while (clock() == startClockValue)
    {
        // Do nothing
    }

    startClockValue = clock();
    // RDTSC( startTicks );

    clock_t currentClock = 0;
    double nextReport = 0;
    do
    {
        currentClock = clock();

        if (pReporter && currentClock >= startClockValue + minClocksToWait * nextReport)
        {
            int done = 100 * nextReport;
            int total = 100;

            int nextCallAmount = pReporter->report(done, total);
            nextReport += static_cast<double>(nextCallAmount) / total;
        }
    } while (currentClock <= startClockValue + minClocksToWait);

    size_t endClockValue = clock();
    // RDTSC( endTicks );

    //  This is a fairly crude calculation but it's probably
    //  good enough for our purposes

    double startTicksDouble = startTicks[1];
    startTicksDouble *= 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0;
    startTicksDouble *= 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0;
    startTicksDouble *= 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0;
    startTicksDouble *= 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0;
    startTicksDouble += startTicks[0];

    double endTicksDouble = endTicks[1];
    endTicksDouble *= 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0;
    endTicksDouble *= 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0;
    endTicksDouble *= 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0;
    endTicksDouble *= 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0;
    endTicksDouble += endTicks[0];

    ticksPerSecond_ = endTicksDouble - startTicksDouble;
    ticksPerSecond_ /= ((endClockValue - startClockValue) / CLOCKS_PER_SEC);
}

void ProProfiler::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const ProProfiler& t)
{

    o << "ProProfiler " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "ProProfiler " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

extern "C" {
//  This is not a member function. By keeping it separate I can call
//  it from the appropriate assembler code.

void ProProfilerPrintCallStack()
{
    ProProfiler::instance().traceStack(ProProfiler::instance().outputStream(), false, 0, nullptr);
    if (ProProfiler::instance().crashOnPrint())
    {
        Diag::instance().forceCrash();
        //            raise( SIGILL );
        exit(1);
    }
}

} //  End extern "C"

void ProProfiler::registerAnchor(const char* anchorFunctionName)
{
    anchorFunctionName_ = _NEW_ARRAY(char, strlen(anchorFunctionName) + 1);
    strcpy(anchorFunctionName_, anchorFunctionName);

    anchorFunctionAddress_ = _REINTERPRET_CAST(void*, (*(&call_stack + count - 1)));

    // ProProfiler::instance().outputStream() << "[[[ " << anchorFunctionName_ << "  " <<
    outputStream() << "[[[ " << anchorFunctionName_ << "  " << anchorFunctionAddress_ << " ]]]" << std::endl;
}
/*
void    ProProfiler::writeStack( ostream& ostr ) const
{
    ostr << "[[[ " << anchorFunctionAddress_ << " :: " << anchorFunctionName_ << " ]]]" << endl;

    for( size_t i = 0; i < count; ++i )
    {
        void* p = _REINTERPRET_CAST( void*, (*(&call_stack + i)) );
        ostr << "[[ " << p << " ]]" << endl;
    }
}
*/
void addToUpdate(uint32[2]);
#pragma aux addToUpdate = " mov   eax, [esi]"                                                                          \
                          " add next_update_time_ls, eax"                                                              \
                          " mov   eax, [esi+4]"                                                                        \
                          " adc next_update_time_ms, eax" parm[esi] modify[eax];

void ProProfiler::nextTraceTime(double offsetSeconds)
{
    //  Get the current time

    uint32 currentTime[2];
    // RDTSC( currentTime );

    next_update_time_ls = currentTime[0];
    next_update_time_ms = currentTime[1];

    //  Now add on the time offset

    uint32 offset[2];

    doubleToUint64(offsetSeconds * ticksPerSecond_, &offset[0], &offset[1]);

    // addToUpdate( offset );
}

void ProProfiler::doubleToUint64(double value, uint32* pLs, uint32* pMs) const
{
    const double TWO_POWER_32 = 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0
        * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0 * 2.0;

    *pMs = value / TWO_POWER_32;
    *pLs = value - *pMs * TWO_POWER_32;
}

bool ProProfiler::isProfilingEnabled() const
{
    return profiling_enabled;
}

void ProProfiler::isMemoryProfilingEnabled(bool isEnabled)
{
    memoryProfilingOn_ = isEnabled;
}

bool ProProfiler::isMemoryProfilingEnabled() const
{
    return memoryProfilingOn_;
}

void traceStack(
    std::ostream& outStream,
    bool doTraceAnchor,
    size_t nStackFrames,
    const size_t* pCallStack,
    uint32_t lineNumber,
    const char* extraString);

void ProProfiler::traceStack(
    std::ostream& mystr,
    bool doTraceAnchor,
    size_t nStackFrames,
    const size_t* pCallStack,
    uint32_t lineNumber,
    const char* extraString)
{
    // This is necessary because of compiler rubbish
    std::ostream* pOstr = &mystr;
    if (isBufferingOutput_)
        pOstr = pMemoryBuffer_;
    std::ostream& ostr = *pOstr;

    // Do the left marker
    ostr << "[[[";

    // If required, write the anchor function name and address
    if (doTraceAnchor)
    {
        ostr << " " << anchorFunctionName_ << " " << anchorFunctionAddress_;
    }

    // Now the actual call stack
    for (size_t i = 0; i < nStackFrames; ++i)
    {
        void* p = _REINTERPRET_CAST(void*, (*(pCallStack + i)));
        ostr << " " << p;
    }

    // And the line number
    ostr << " [" << lineNumber << "]";

    // And any extra string
    if (extraString && extraString[0] != '\0')
        ostr << " { " << extraString << " }";

    // And the terminator
    ostr << " ]]]" << std::endl;
}

void ProProfiler::isBufferingOutput(bool is)
{
    // Check which way
    if (is && ! isBufferingOutput_)
    {
        // starting buffering
        isBufferingOutput_ = true;
        pMemoryBuffer_ = new BaseLogBuffer(256000);
    }
    else if (! is && isBufferingOutput_)
    {
        // starting buffering
        isBufferingOutput_ = false;
        delete pMemoryBuffer_;
        pMemoryBuffer_ = nullptr;
    }
}

bool ProProfiler::isBufferingOutput() const
{
    return isBufferingOutput_;
}

void ProProfiler::clearBuffer()
{
    PRE(isBufferingOutput());
    pMemoryBuffer_->clear();
}

void ProProfiler::writeBuffer(std::ostream& outStream)
{
    PRE(isBufferingOutput());
    outStream << *pMemoryBuffer_;
    pMemoryBuffer_->clear();
}

void ProProfiler::crashOnPrint(bool crash)
{
    crashOnPrint_ = crash;
}

bool ProProfiler::crashOnPrint()
{
    return crashOnPrint_;
}
// TODO:
extern "C" {
#ifdef _MSC_VER
void checkForProProfilerShutdown(void*)
#else
void* checkForProProfilerShutdown(void* dummy)
#endif // _MSC_VER
{
    unsigned attr;
    bool result = false;
    /*
    while ( not result )
    {
        if( _dos_getfileattr( "stop.now", &attr ) == 0 )
        {
            if( ( attr & _A_SUBDIR ) != _A_SUBDIR )
                result = true;
        }

        Sleep( 10000 );
    }
    */
    ProProfiler::instance().traceStack(ProProfiler::instance().outputStream(), true, 0, "User interrupt");
    ;
    ProProfiler::instance().closeOutputStream();
    exit(1);
}
}

const size_t* ProProfiler::pCallStack() const
{
    return &call_stack;
}

size_t ProProfiler::nCallStackEntries() const
{
    return count;
}

void ProProfiler::traceStack(std::ostream& outStream, bool doTraceAnchor, uint32_t lineNumber, const char* extraString)
{
    traceStack(outStream, doTraceAnchor, count, &call_stack, lineNumber, extraString);
}

/* End PROFILER.CPP *************************************************/
