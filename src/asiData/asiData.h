//-----------------------------------------------------------------------------
// Created on: 25 September 2015
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

#ifndef asiData_h
#define asiData_h

#define asiData_NotUsed(x)

#ifdef _WIN32
  #ifdef asiData_EXPORTS
    #define asiData_EXPORT __declspec(dllexport)
  #else
    #define asiData_EXPORT __declspec(dllimport)
  #endif
#else
  #define asiData_EXPORT
#endif

// asiData includes
#include <asiData_ParameterFlags.h>

// Active Data includes
#include <ActAPI_IParameter.h>

//-----------------------------------------------------------------------------
// Custom Active Data Parameters
//-----------------------------------------------------------------------------

#define Parameter_AAG         Parameter_LASTFREE
#define Parameter_BVH         Parameter_LASTFREE + 1
#define Parameter_Naming      Parameter_LASTFREE + 2
#define Parameter_Function    Parameter_LASTFREE + 3
#define Parameter_Octree      Parameter_LASTFREE + 4
#define Parameter_UniformGrid Parameter_LASTFREE + 5
//
#define Parameter_LASTFREE_ASITUS Parameter_UniformGrid

#endif
