//-----------------------------------------------------------------------------
// Created on: 25 March 2016
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

// Own include
#include <visu_surface_source.h>

// Visualization includes
#include <visu_utils.h>

// VTK includes
#include <vtkCellData.h>
#include <vtkDataObject.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

// OCCT includes
#include <GeomLProp_SLProps.hxx>

//-----------------------------------------------------------------------------
// Construction
//-----------------------------------------------------------------------------

vtkStandardNewMacro(visu_surface_source);

//! Default constructor.
visu_surface_source::visu_surface_source()
//
  : vtkPolyDataAlgorithm (),
    m_iSteps             (0),
    m_scalars            (Scalars_NoScalars),
    m_fMinScalar         (0.0),
    m_fMaxScalar         (0.0),
    m_fTrimU             (100.0),
    m_fTrimV             (100.0)
{
  this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                  // which has nothing to do with VTK pipeline
}

//! Destructor.
visu_surface_source::~visu_surface_source()
{
}

//-----------------------------------------------------------------------------
// Kernel methods
//-----------------------------------------------------------------------------

//! Initialize data source with a parametric surface.
//! \param surf [in] surface to visualize.
void visu_surface_source::SetInputSurface(const Handle(Geom_Surface)& surf)
{
  m_surf = surf;
}

//! Sets the number of sampling steps for surface discretization.
//! \param nSteps [in] number of steps to set.
void visu_surface_source::SetNumberOfSteps(const int nSteps)
{
  m_iSteps = nSteps;
  //
  this->Modified();
}

//! Sets type of scalars to associate with nodes.
//! \param scalars [in] scalars.
void visu_surface_source::SetScalars(const NodeScalars scalars)
{
  m_scalars = scalars;
  //
  this->Modified();
}

//! Sets trimming values for infinite surface domains.
//! \param uLimit [in] trimming value for U.
//! \param vLimit [in] trimming value for V.
void visu_surface_source::SetTrimValues(const double uLimit,
                                        const double vLimit)
{
  m_fTrimU = uLimit;
  m_fTrimV = vLimit;
  //
  this->Modified();
}

//-----------------------------------------------------------------------------

//! This is called by the superclass. Creates VTK polygonal data set
//! from the input arrays.
//! \param request      [in] information about data object.
//! \param inputVector  [in] the input data. As a data source is the start
//!                          stage of the VTK pipeline, the Input Vector is
//!                          empty and not used (no input port).
//! \param outputVector [in] the pointer to output data, that is filled
//!                          in this method.
//! \return status.
int visu_surface_source::RequestData(vtkInformation*        request,
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector*  outputVector)
{
  if ( m_surf.IsNull() )
  {
    vtkErrorMacro( << "Invalid domain: NULL surface" );
    return 0;
  }

  if ( m_iSteps <= 0 )
  {
    vtkErrorMacro( << "Invalid domain: number of sampling steps must be positive" );
    return 0;
  }

  /* ==============================
   *  Prepare involved collections
   * ============================== */

  vtkPolyData* polyOutput = vtkPolyData::GetData(outputVector);
  polyOutput->Allocate();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  polyOutput->SetPoints(points);

  // Array for scalars
  vtkSmartPointer<vtkDoubleArray> curvature;
  //
  if ( m_scalars == Scalars_GaussianCurvature || m_scalars == Scalars_MeanCurvature )
  {
    vtkPointData* PD = polyOutput->GetPointData();
    curvature = visu_utils::InitDoubleArray(ARRNAME_SURF_CURVATURE);
    PD->SetScalars(curvature);
  }

  //---------------------------------------------------------------------------

  double uMin, uMax, vMin, vMax;
  m_surf->Bounds(uMin, uMax, vMin, vMax);
  //
  uMin = visu_utils::TrimInf(uMin, m_fTrimU);
  uMax = visu_utils::TrimInf(uMax, m_fTrimU);
  vMin = visu_utils::TrimInf(vMin, m_fTrimV);
  vMax = visu_utils::TrimInf(vMax, m_fTrimV);

  const double uStep = (uMax - uMin) / m_iSteps;
  const double vStep = (vMax - vMin) / m_iSteps;

  // Choose u values
  std::vector<double> U;
  double              u     = uMin;
  bool                uStop = false;
  //
  while ( !uStop )
  {
    if ( u > uMax )
    {
      u     = uMax;
      uStop = true;
    }

    U.push_back(u);
    u += uStep;
  }

  // Choose v values
  std::vector<double> V;
  double              v     = vMin;
  bool                vStop = false;
  //
  while ( !vStop )
  {
    if ( v > vMax )
    {
      v     = vMax;
      vStop = true;
    }

    V.push_back(v);
    v += vStep;
  }

  m_fMinScalar =  RealLast();
  m_fMaxScalar = -RealLast();

  // Register geometry (points)
  std::vector< std::vector<vtkIdType> > UV_ids;
  for ( size_t i = 0; i < U.size(); ++i )
  {
    std::vector<vtkIdType> uIso_ids;
    for ( size_t j = 0; j < V.size(); ++j )
    {
      const gp_Pnt P = m_surf->Value(U[i], V[j]);
      //
      uIso_ids.push_back( this->registerGridPoint(P, polyOutput) );

      // Associate scalars
      if ( m_scalars == Scalars_GaussianCurvature || m_scalars == Scalars_MeanCurvature )
      {
        GeomLProp_SLProps lProps( m_surf, U[i], V[j], 2, gp::Resolution() );
        double k = 0.0;
        //
        if ( m_scalars == Scalars_GaussianCurvature )
          k = lProps.GaussianCurvature();
        if ( m_scalars == Scalars_MeanCurvature )
          k = lProps.MeanCurvature();
        //
        if ( k < m_fMinScalar ) m_fMinScalar = k;
        if ( k > m_fMaxScalar ) m_fMaxScalar = k;
        //
        curvature->InsertNextValue(k);
      }
    }
    UV_ids.push_back(uIso_ids);
  }

  // Register topology (triangles)
  for ( size_t i = 1; i < U.size(); ++i )
  {
    for ( size_t j = 1; j < V.size(); ++j )
    {
      // First triangle
      {
        const vtkIdType n1 = UV_ids[i]    [j];
        const vtkIdType n2 = UV_ids[i]    [j - 1];
        const vtkIdType n3 = UV_ids[i - 1][j];
        //
        this->registerTriangle(n1, n2, n3, polyOutput);
      }

      // Opposite triangle
      {
        const vtkIdType n1 = UV_ids[i]    [j - 1];
        const vtkIdType n2 = UV_ids[i - 1][j - 1];
        const vtkIdType n3 = UV_ids[i - 1][j];
        //
        this->registerTriangle(n1, n2, n3, polyOutput);
      }
    }
  }

  if ( this->HasScalars() )
    if ( Abs(m_fMaxScalar - m_fMinScalar) < 1.0e-6 )
      m_fMinScalar = m_fMaxScalar; // Good for visualization

  //---------------------------------------------------------------------------

  return Superclass::RequestData(request, inputVector, outputVector);
}

//! Adds the given point to the passed polygonal data set.
//! \param point    [in]     point to add.
//! \param polyData [in/out] polygonal data set being populated.
//! \return ID of the just added VTK point.
vtkIdType visu_surface_source::registerGridPoint(const gp_Pnt& point,
                                                 vtkPolyData*  polyData)
{
  // Access necessary arrays
  vtkPoints* points = polyData->GetPoints();

  // Push the point into VTK data set
  vtkIdType pid = points->InsertNextPoint( point.X(),
                                           point.Y(),
                                           point.Z() );

  return pid;
}

//! Adds a triangle cell into the polygonal data set.
//! \param n1       [in]     index of the first node.
//! \param n2       [in]     index of the second node.
//! \param n3       [in]     index of the third node.
//! \param polyData [in/out] polygonal data set being populated.
//! \return ID of the just added VTK cell.
vtkIdType visu_surface_source::registerTriangle(const vtkIdType n1,
                                                const vtkIdType n2,
                                                const vtkIdType n3,
                                                vtkPolyData*    polyData)
{
  std::vector<vtkIdType> nodes;
  nodes.push_back(n1);
  nodes.push_back(n2);
  nodes.push_back(n3);
  //
  vtkIdType cellID = polyData->InsertNextCell(VTK_TRIANGLE, 3, &nodes[0]);
  //
  return cellID;
}
