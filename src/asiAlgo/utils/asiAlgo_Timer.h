//-----------------------------------------------------------------------------
// Created on: 23 March 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef asiAlgo_Timer_h
#define asiAlgo_Timer_h

// asiAlgo includes
#include <asiAlgo_MemChecker.h>

// OCCT includes
#pragma warning(push, 0)
#include <OSD_Timer.hxx>
#include <TCollection_AsciiString.hxx>
#pragma warning(pop)

/************************************************************************
                           MEASURING PERFORMANCE
 ************************************************************************/

//! Example:
//!
//! #ifdef TIMER_NEW
//!   TIMER_NEW
//!   TIMER_RESET
//!   TIMER_GO
//! #endif
//!
//! ... YOUR AD_ALGO GOES HERE ...
//!
//! #ifdef TIMER_NEW
//!   TIMER_FINISH
//!   TIMER_COUT_RESULT
//! #endif

#define TIMER_NEW \
  OSD_Timer __aux_debug_Timer; \
  double __aux_debug_Seconds, __aux_debug_CPUTime; \
  int __aux_debug_Minutes, __aux_debug_Hours; \
  int __aux_debug_memcheck_before, __aux_debug_memcheck_after;

#define TIMER_RESET \
  __aux_debug_Seconds = __aux_debug_CPUTime = 0.0; \
  __aux_debug_Minutes = __aux_debug_Hours = 0; \
  __aux_debug_Timer.Reset();

#define TIMER_GO \
  MEMCHECK_COUNT_MIB(__aux_debug_memcheck_before) \
  __aux_debug_Timer.Start();

#define TIMER_FINISH \
  MEMCHECK_COUNT_MIB(__aux_debug_memcheck_after) \
  __aux_debug_Timer.Stop(); \
  __aux_debug_Timer.Show(__aux_debug_Seconds, __aux_debug_Minutes, __aux_debug_Hours, __aux_debug_CPUTime);

#define TIMER_COUT_RESULT \
  { \
    TIMER_COUT_RESULT_MSG(""); \
  }

#define TIMER_COUT_RESULT_MSG(Msg) \
  { \
    std::cout << "\n=============================================" << std::endl; \
    TCollection_AsciiString ascii_msg(Msg); \
    if ( !ascii_msg.IsEmpty() ) \
    { \
      std::cout << Msg                                             << std::endl; \
      std::cout << "---------------------------------------------" << std::endl; \
    } \
    std::cout << "Seconds:  " << __aux_debug_Seconds               << std::endl; \
    std::cout << "Minutes:  " << __aux_debug_Minutes               << std::endl; \
    std::cout << "Hours:    " << __aux_debug_Hours                 << std::endl; \
    std::cout << "CPU time: " << __aux_debug_CPUTime               << std::endl; \
    std::cout << "=============================================\n" << std::endl; \
  }

#define TIMER_COUT_RESULT_NOTIFIER(Notifier, Msg) \
  { \
    const int __aux_memcheck_delta = __aux_debug_memcheck_after - __aux_debug_memcheck_before; \
    TCollection_AsciiString __aux_memcheck_delta_str; \
    if ( __aux_memcheck_delta > 0 ) __aux_memcheck_delta_str = "+"; \
    __aux_memcheck_delta_str += __aux_memcheck_delta; \
    Notifier.SendLogMessage(LogInfo(Normal) << "============================================="); \
    Notifier.SendLogMessage(LogInfo(Normal) << "%1" << Msg); \
    Notifier.SendLogMessage(LogInfo(Normal) << "---------------------------------------------"); \
    Notifier.SendLogMessage(LogInfo(Normal) << "\tElapsed time (seconds):  %1"     << __aux_debug_Seconds); \
    Notifier.SendLogMessage(LogInfo(Normal) << "\tElapsed time (minutes):  %1"     << __aux_debug_Minutes); \
    Notifier.SendLogMessage(LogInfo(Normal) << "\tElapsed time (hours):    %1"     << __aux_debug_Hours); \
    Notifier.SendLogMessage(LogInfo(Normal) << "\tElapsed time (CPU time): %1"     << __aux_debug_CPUTime); \
    Notifier.SendLogMessage(LogInfo(Normal) << "\tMemory before:           %1 MiB" << __aux_debug_memcheck_before); \
    Notifier.SendLogMessage(LogInfo(Normal) << "\tMemory after [delta]:    %1 MiB [%2]" \
                                            << __aux_debug_memcheck_after \
                                            << __aux_memcheck_delta_str); \
    Notifier.SendLogMessage(LogInfo(Normal) << "... Finished."); \
  }

#endif
