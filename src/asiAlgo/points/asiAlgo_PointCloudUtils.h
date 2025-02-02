//-----------------------------------------------------------------------------
// Created on: 02 December 2016
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

#ifndef asiAlgo_PointCloudUtils_h
#define asiAlgo_PointCloudUtils_h

// asiAlgo includes
#include <asiAlgo_BaseCloud.h>

//-----------------------------------------------------------------------------

//! Point cloud processing utilities.
namespace asiAlgo_PointCloudUtils
{
  asiAlgo_EXPORT Handle(TColStd_HArray1OfReal)
    AsRealArray(const Handle(asiAlgo_BaseCloud<double>)& pointCloud);

  asiAlgo_EXPORT Handle(TColStd_HArray1OfReal)
    AsRealArray(const Handle(asiAlgo_BaseCloud<float>)& pointCloud);

  asiAlgo_EXPORT Handle(asiAlgo_BaseCloud<double>)
    AsCloudd(const Handle(TColStd_HArray1OfReal)& arr);

  asiAlgo_EXPORT Handle(asiAlgo_BaseCloud<float>)
    AsCloudf(const Handle(TColStd_HArray1OfReal)& arr);

  asiAlgo_EXPORT Handle(asiAlgo_BaseCloud<double>)
    CloudfAsCloudd(const Handle(asiAlgo_BaseCloud<float>)& pointCloud);
}

#endif
