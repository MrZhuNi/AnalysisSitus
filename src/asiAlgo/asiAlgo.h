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

#ifndef asiAlgo_h
#define asiAlgo_h

#define asiAlgo_NotUsed(x)

#ifdef _WIN32
  #ifdef asiAlgo_EXPORTS
    #define asiAlgo_EXPORT __declspec(dllexport)
  #else
    #define asiAlgo_EXPORT __declspec(dllimport)
  #endif
#else
  #define asiAlgo_EXPORT
#endif

//-----------------------------------------------------------------------------

#define ASI_TEST_DATA    "ASI_TEST_DATA"
#define ASI_TEST_DUMPING "ASI_TEST_DUMPING"
#define ASI_TEST_DESCR   "ASI_TEST_DESCR"

//-----------------------------------------------------------------------------

#define ASI_FILE_EXT "situ"

//-----------------------------------------------------------------------------

typedef int t_topoId; // Domain (1-based face ID).

#endif
