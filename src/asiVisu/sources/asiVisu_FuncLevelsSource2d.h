//-----------------------------------------------------------------------------
// Created on: 28 April 2017
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

#ifndef asiVisu_FuncLevelsSource2d_h
#define asiVisu_FuncLevelsSource2d_h

// asiVisu includes
#include <asiVisu_FuncLevelsSourceBase.h>

#undef COUT_DEBUG

//! Source of polygonal data representing a function levels.
template <typename T_VARIABLE>
class asiVisu_FuncLevelsSource2d : public asiVisu_FuncLevelsSourceBase<T_VARIABLE>
{
// RTTI and construction:
public:

  vtkTypeMacro(asiVisu_FuncLevelsSource2d, asiVisu_FuncLevelsSourceBase);

  static asiVisu_FuncLevelsSource2d<T_VARIABLE>* New()
  {
    return new asiVisu_FuncLevelsSource2d<T_VARIABLE>();
  }

protected:

  //! This is called by the superclass. Creates VTK polygonal data set
  //! from the input data.
  //! \param request      [in] information about data object.
  //! \param inputVector  [in] the input data. As a data source is the start
  //!                          stage of the VTK pipeline, the Input Vector is
  //!                          empty and not used (no input port).
  //! \param outputVector [in] the pointer to output data, that is filled
  //!                          in this method.
  //! \return status.
  virtual int RequestData(vtkInformation*        request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector*  outputVector)
  {
    if ( m_func.IsNull() )
    {
      vtkErrorMacro( << "Invalid domain: nullptr function" );
      return 0;
    }

    if ( T_VARIABLE::num_coordinates() != 2 )
    {
      vtkErrorMacro( << "Invalid domain: only two-dimensional domains are supported" );
      return 0;
    }

    if ( m_iNumIntervals < 1 )
    {
      vtkErrorMacro( << "Invalid number of sampling intervals" );
      return 0;
    }

    /* ==============================
     *  Prepare involved collections
     * ============================== */

    vtkPolyData* polyOutput = vtkPolyData::GetData(outputVector);
    polyOutput->Allocate();

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    polyOutput->SetPoints(points);

    /* ===============
     *  Sample domain
     * =============== */

    const int numRanges_x0 = m_iNumIntervals;
    const int numRanges_x1 = m_iNumIntervals;
    //
    const double x0_step = ( m_maxCorner.Coord(0) - m_minCorner.Coord(0) ) / numRanges_x0;
    const double x1_step = ( m_maxCorner.Coord(1) - m_minCorner.Coord(1) ) / numRanges_x1;

    // Function values
    std::vector<double> funcValues;

    // Sample
    for ( int i = 0; i <= numRanges_x0; ++i )
    {
      double x0 = m_minCorner.Coord(0) + i*x0_step;
      //
      if ( x0 > m_maxCorner.Coord(0) )
        x0 = m_maxCorner.Coord(0);

      for ( int j = 0; j <= numRanges_x1; ++j )
      {
        double x1 = m_minCorner.Coord(1) + j*x1_step;
        //
        if ( x1 > m_maxCorner.Coord(1) )
          x1 = m_maxCorner.Coord(1);

        // Register point and cell
        gp_Pnt P( x0, x1, 0.0 );
        vtkIdType pointIndex = this->registerGridPoint(P, polyOutput);
        this->registerVertex( pointIndex, polyOutput );

        // Evaluate function
        const double f = m_func->Value( T_VARIABLE(x0, x1) );
        //
        if ( f < m_fFuncMin ) m_fFuncMin = f;
        if ( f > m_fFuncMax ) m_fFuncMax = f;
        //
        funcValues.push_back(f);
      }
    }

    /* ====================================================================
     *  Associate colors to the domain points according to function values
     * ==================================================================== */

    // Create the color map
    vtkSmartPointer<vtkLookupTable>
      colorLookupTable = vtkSmartPointer<vtkLookupTable>::New();
    //
    colorLookupTable->SetTableRange(m_fFuncMin, m_fFuncMax);
    colorLookupTable->Build();

    // Generate the colors for each point based on the color map
    vtkSmartPointer<vtkUnsignedCharArray>
      colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    //
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");

    // Set color for each point
    for ( int i = 0; i < polyOutput->GetNumberOfPoints(); ++i )
    {
      double p[3];
      polyOutput->GetPoint(i, p);
 
      double dcolor[3];
      colorLookupTable->GetColor(funcValues[i], dcolor);

      unsigned char color[3];
      for ( unsigned int j = 0; j < 3; ++j )
      {
        color[j] = static_cast<unsigned char>(255.0 * dcolor[j]);
      }

  #if defined COUT_DEBUG
      std::cout << "color: "
                << (int) color[0] << " "
                << (int) color[1] << " "
                << (int) color[2] << std::endl;
  #endif
 
      colors->InsertNextTypedTuple(color);
    }

    // Set scalars
    polyOutput->GetPointData()->SetScalars(colors);

    return Superclass::RequestData(request, inputVector, outputVector);
  }

protected:

  //! Default constructor.
  asiVisu_FuncLevelsSource2d()
  {
    this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                    // which has nothing to do with VTK pipeline
  }

  //! Destructor.
  ~asiVisu_FuncLevelsSource2d() {}

private:

  asiVisu_FuncLevelsSource2d(const asiVisu_FuncLevelsSource2d&);
  asiVisu_FuncLevelsSource2d& operator=(const asiVisu_FuncLevelsSource2d&);

};

#endif
