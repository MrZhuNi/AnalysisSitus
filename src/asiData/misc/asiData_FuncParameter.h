//-----------------------------------------------------------------------------
// Created on: 05 May 2017
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

#ifndef asiData_FuncParameter_h
#define asiData_FuncParameter_h

// asiData includes
#include <asiData_FuncAttr.h>

// Active Data includes
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>
#include <ActData_ParameterFactory.h>
#include <ActData_UserParameter.h>
#include <ActData_Utils.h>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Function Parameter without any OCAF connectivity.
template <typename T_VARIABLE>
class asiData_FuncDTO : public ActData_ParameterDTO
{
public:

  //! Constructor accepting GID.
  //! \param GID [in] GID.
  asiData_FuncDTO(const ActAPI_ParameterGID& GID) : ActData_ParameterDTO(GID, Parameter_UNDEFINED) {}

public:

  Handle(asiAlgo_Function<T_VARIABLE>) Func; //!< Function object.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Node Parameter representing a Function object in Active Data framework.
template <typename T_VARIABLE>
class asiData_FuncParameter : public ActData_UserParameter
{
public:

  //! Ensures correct construction of the Parameter object, e.g. prevents
  //! allocating it in stack memory.
  //! \return Parameter instance.
  static Handle(asiData_FuncParameter<T_VARIABLE>) Instance()
  {
    return new asiData_FuncParameter<T_VARIABLE>();
  }

public:

  //! Sets function object.
  //! \param function        [in] function object to set.
  //! \param MType           [in] modification type.
  //! \param doResetValidity [in] indicates whether to reset validity flag.
  //! \param doResetPending  [in] indicates whether this Parameter must lose its
  //!                             PENDING (or out-dated) property.
  void SetFunction(const Handle(asiAlgo_Function<T_VARIABLE>)& func,
                   const ActAPI_ModificationType               MType           = MT_Touched,
                   const bool                                  doResetValidity = true,
                   const bool                                  doResetPending  = true)
  {
    if ( this->IsDetached() )
      Standard_ProgramError::Raise("Cannot access detached data");

    // Settle down an attribute an populate it with data
    TDF_Label                            dataLab = ActData_Utils::ChooseLabelByTag(m_label, DS_Func, true);
    Handle(asiData_FuncAttr<T_VARIABLE>) attr    = asiData_FuncAttr<T_VARIABLE>::Set(dataLab);
    //
    attr->SetFunction(func);

    // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
    SPRING_INTO_FUNCTION(MType)
    // Reset Parameter's validity flag if requested
    RESET_VALIDITY(doResetValidity)
    // Reset Parameter's PENDING property
    RESET_PENDING(doResetPending)
  }

  //! Accessor for the stored function object.
  //! \return stored function object.
  Handle(asiAlgo_Function<T_VARIABLE>) GetFunction()
  {
    if ( !this->IsWellFormed() )
      Standard_ProgramError::Raise("Data inconsistent");

    // Choose a data label ensuring not to create it
    TDF_Label dataLab = ActData_Utils::ChooseLabelByTag(m_label, DS_Func, false);

    // Get function attribute
    Handle(asiData_FuncAttr<T_VARIABLE>) attr;
    dataLab.FindAttribute(asiData_FuncAttr<T_VARIABLE>::GUID(), attr);
    //
    if ( attr.IsNull() )
      return nullptr;

    return attr->GetFunction();
  }

protected:

  //! Default constructor.
  asiData_FuncParameter() : ActData_UserParameter() {}

private:

  //! Checks if this Parameter object is mapped onto CAF data structure in a
  //! correct way.
  //! \return true if the object is well-formed, false -- otherwise.
  virtual bool isWellFormed() const
  {
    if ( !ActData_Utils::CheckLabelAttr( m_label, DS_Func,
                                         asiData_FuncAttr<T_VARIABLE>::GUID() ) )
      return false;

    return true;
  }

  //! Returns Parameter type.
  //! \return Parameter type.
  virtual int parameterType() const
  {
    return Parameter_Function;
  }

private:

  //! Populates Parameter from the passed DTO.
  //! \param DTO             [in] DTO to source data from.
  //! \param MType           [in] modification type.
  //! \param doResetValidity [in] indicates whether validity flag must be
  //!                             reset or not.
  //! \param doResetPending  [in] indicates whether pending flag must be reset
  //!                             or not.
  virtual void
    setFromDTO(const Handle(ActData_ParameterDTO)& DTO,
               const ActAPI_ModificationType       MType = MT_Touched,
               const bool                          doResetValidity = true,
               const bool                          doResetPending = true)
  {
    Handle(asiData_FuncDTO<T_VARIABLE>) MyDTO = Handle(asiData_FuncDTO<T_VARIABLE>)::DownCast(DTO);
    this->SetFunction(MyDTO->Func, MType, doResetValidity, doResetPending);
  }

  //! Creates and populates DTO.
  //! \param GID [in] ready-to-use GID for DTO.
  //! \return constructed DTO instance.
  virtual Handle(ActData_ParameterDTO)
    createDTO(const ActAPI_ParameterGID& GID)
  {
    Handle(asiData_FuncDTO<T_VARIABLE>) res = new asiData_FuncDTO<T_VARIABLE>(GID);
    res->Func = this->GetFunction();
    return res;
  }

protected:

  //! Tags for the underlying CAF Labels.
  enum Datum
  {
    DS_Func = ActData_UserParameter::DS_DatumLast,
    DS_DatumLast = DS_Func + RESERVED_DATUM_RANGE
  };

};

#endif
