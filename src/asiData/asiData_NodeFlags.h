//-----------------------------------------------------------------------------
// Created on: 25 January 2019
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2019-present, Sergey Slyadnev
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

#ifndef asiData_NodeFlags_HeaderFile
#define asiData_NodeFlags_HeaderFile

//-----------------------------------------------------------------------------

//! User flags for Nodes. The items of this enumeration are used
//! for additional customization of CAF Data Objects with information
//! required by GUI. The kind of such information is representation-dependent,
//! so other GUI services might want to use some other set of user flags.
enum asiData_NodeFlags
{
  //! Indicates whether the Data Node can be presented in 3D viewer tailored
  //! to parts.
  NodeFlag_IsPresentedInPartView = 0x001,

  //! Indicates whether the Data Node can be presented in 3D viewer tailored
  //! to the host geometry.
  NodeFlag_IsPresentedInHostView = 0x002,

  //! Indicates whether the Data Node can be presented in 3D viewer tailored
  //! to the parameteric domain.
  NodeFlag_IsPresentedInDomainView = 0x004,

  //! Indicates that 3D Presentation is currently visible.
  NodeFlag_IsPresentationVisible = 0x008,

  //! Indicates that Data Node should not be displayed in Object Browser
  //! even if its Tree Node connectivity allows it.
  NodeFlag_IsHiddenInBrowser = 0x010,

  //! Indicates whether this Data Node is structural or not. We say that
  //! Data Node is structural if it is an essential (and so immutable)
  //! part of the Project tree.
  NodeFlag_IsStructural = 0x020,

};

#endif
