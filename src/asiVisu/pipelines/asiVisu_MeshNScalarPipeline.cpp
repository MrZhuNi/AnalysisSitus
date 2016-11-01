//-----------------------------------------------------------------------------
// Created on: 13 November 2015
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

// Own include
#include <asiVisu_MeshNScalarPipeline.h>

// Visualization includes
#include <asiVisu_MeshNScalarFilter.h>
#include <asiVisu_MeshResultUtils.h>
#include <asiVisu_MeshSource.h>
#include <asiVisu_MeshUtils.h>
#include <asiUI_NodeInfo.h>

// Active Data includes
#include <ActData_MeshParameter.h>
#include <ActData_ParameterFactory.h>

// OCCT includes
#include <Precision.hxx>
#include <Standard_ProgramError.hxx>

// VTK includes
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>

//-----------------------------------------------------------------------------
// Pipeline
//-----------------------------------------------------------------------------

//! Creates new Pipeline instance.
asiVisu_MeshNScalarPipeline::asiVisu_MeshNScalarPipeline()
  : asiVisu_Pipeline( vtkSmartPointer<vtkPolyDataMapper>::New(),
                   vtkSmartPointer<vtkActor>::New() )
{
  /* ========================
   *  Prepare custom filters
   * ======================== */

  // Allocate filter populating scalar arrays
  vtkSmartPointer<asiVisu_MeshNScalarFilter>
    aScFilter = vtkSmartPointer<asiVisu_MeshNScalarFilter>::New();

  // Allocate Normals Calculation filter
  vtkSmartPointer<vtkPolyDataNormals>
    aNormalsFilter = vtkSmartPointer<vtkPolyDataNormals>::New();

  /* =========================
   *  Register custom filters
   * ========================= */

  m_filterMap.Bind(Filter_NScalar, aScFilter);
  m_filterMap.Bind(Filter_Normals, aNormalsFilter);

  // Append custom filters to the pipeline
  this->append( m_filterMap.Find(Filter_NScalar) );
  this->append( m_filterMap.Find(Filter_Normals) );
}

//! Sets input data for the pipeline accepting Active Data entities.
//! Actually this method performs translation of DOMAIN data to VTK POLYGONAL
//! DATA.
//! \param theDataProvider [in] Data Provider.
void asiVisu_MeshNScalarPipeline::SetInput(const Handle(asiVisu_DataProvider)& theDataProvider)
{
  Handle(asiVisu_MeshNScalarDataProvider)
    aMeshPrv = Handle(asiVisu_MeshNScalarDataProvider)::DownCast(theDataProvider);

  /* ============================
   *  Prepare polygonal data set
   * ============================ */

  if ( aMeshPrv->MustExecute( this->GetMTime() ) )
  {
    vtkSmartPointer<asiVisu_MeshSource> aMeshSource = vtkSmartPointer<asiVisu_MeshSource>::New();
    aMeshSource->SetInputMesh( aMeshPrv->GetMeshDS() );

    asiVisu_MeshNScalarFilter*
      aScFilter = asiVisu_MeshNScalarFilter::SafeDownCast( m_filterMap.Find(Filter_NScalar) );

    aScFilter->SetScalarArrays( aMeshPrv->GetNodeIDs(),
                                aMeshPrv->GetNodeScalars() );

    // Complete pipeline
    this->SetInputConnection( aMeshSource->GetOutputPort() );

    // Bind actor to owning Node ID. Thus we set back reference from VTK
    // entity to data object
    asiUI_NodeInfo::Store( aMeshPrv->GetNodeID(), this->Actor() );
  }

  // Update modification timestamp
  this->Modified();
}

//! Callback for AddToRenderer base routine. Good place to adjust visualization
//! properties of the pipeline's actor.
void asiVisu_MeshNScalarPipeline::addToRendererCallback(vtkRenderer*)
{
  this->Actor()->GetProperty()->SetInterpolationToGouraud();
}

//! Callback for RemoveFromRenderer base routine.
void asiVisu_MeshNScalarPipeline::removeFromRendererCallback(vtkRenderer*)
{
}

//! Callback for Update routine.
void asiVisu_MeshNScalarPipeline::updateCallback()
{
  asiVisu_MeshNScalarFilter*
    aScFilter = asiVisu_MeshNScalarFilter::SafeDownCast( m_filterMap.Find(Filter_NScalar) );
  aScFilter->Update();
  double aMinScalar = aScFilter->GetMinScalar(),
                aMaxScalar = aScFilter->GetMaxScalar();

  if ( Abs(aMinScalar) == VTK_FLOAT_MAX && Abs(aMaxScalar) == VTK_FLOAT_MAX )
    aMinScalar = aMaxScalar = 0.0;

  bool doScalarInterpolation;
  if ( Abs(aMaxScalar - aMinScalar) < Precision::Confusion() )
    doScalarInterpolation = false;
  else
    doScalarInterpolation = true;

  asiVisu_MeshResultUtils::InitPointScalarMapper(m_mapper, ARRNAME_MESH_N_SCALARS,
                                                aMinScalar, aMaxScalar,
                                                doScalarInterpolation);
}
