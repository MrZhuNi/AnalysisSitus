//-----------------------------------------------------------------------------
// Created on: 16 May 2016
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

// Own include
#include <asiUI_DialogSTEP.h>

// asiAlgo includes
#include <asiAlgo_STEP.h>

// asiEngine includes
#include <asiEngine_Part.h>

// asiUI includes
#include <asiUI_Common.h>
#include <asiUI_DialogSTEPDelegate.h>

// OCCT includes
#include <Interface_Static.hxx>
#include <STEPControl_Controller.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QApplication>
#include <QHeaderView>
#pragma warning(pop)

//-----------------------------------------------------------------------------

#define BTN_MIN_WIDTH 120

//-----------------------------------------------------------------------------
// Construction & destruction
//-----------------------------------------------------------------------------

//! Constructor.
//! \param model  [in] Data Model instance.
//! \param part_n [in] Part Node.
//! \param mode   [in] interoperability mode.
//! \param parent [in] parent widget.
asiUI_DialogSTEP::asiUI_DialogSTEP(const Handle(ActAPI_IModel)&    model,
                                   const Handle(asiData_PartNode)& part_n,
                                   const Mode                      mode,
                                   QWidget*                        parent)
: QDialog (parent),
  m_model (model),
  m_part  (part_n),
  m_mode  (mode)
{
  // Main layout
  m_pMainLayout = new QVBoxLayout();

  // Widgets
  m_widgets.pOptions = new QTableWidget();
  m_widgets.pProceed = new QPushButton("Proceed");
  //
  m_widgets.pProceed -> setMinimumWidth(BTN_MIN_WIDTH);

  // Configure table with options
  QStringList headers;
  headers.append("Name");
  headers.append("Value");
  //
  m_widgets.pOptions->setColumnCount( headers.size() );
  m_widgets.pOptions->setHorizontalHeaderLabels( headers );
  m_widgets.pOptions->horizontalHeader()->setStretchLastSection( true );
  m_widgets.pOptions->verticalHeader()->setVisible( false );
  m_widgets.pOptions->setSelectionMode( QAbstractItemView::SingleSelection );
  m_widgets.pOptions->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_widgets.pOptions->setItemDelegateForColumn( 1, new asiUI_DialogSTEPDelegate(this) );

  // Set layout
  m_pMainLayout->setSpacing(10);
  //
  m_pMainLayout->addWidget(m_widgets.pOptions);
  m_pMainLayout->addWidget(m_widgets.pProceed);
  //
  m_pMainLayout->setAlignment(Qt::AlignTop);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);
  //
  this->setLayout(m_pMainLayout);

  // Initialize table
  this->initialize();

  // Connect signals to slots
  connect( m_widgets.pOptions, SIGNAL ( itemChanged(QTableWidgetItem*) ),
           this,               SLOT   ( onVarChanged(QTableWidgetItem*) ) );
  //
  connect( m_widgets.pProceed, SIGNAL( clicked() ), SLOT( onProceed() ) );

  // Set good initial size
  this->setMinimumSize( QSize(400, 600) );
}

//! Destructor.
asiUI_DialogSTEP::~asiUI_DialogSTEP()
{
  delete m_pMainLayout;
  m_widgets.Release();
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------

//! Initializes table of variables.
void asiUI_DialogSTEP::initialize()
{
  // Activate static variables of STEP translation tool
  STEPControl_Controller::Init();

  // Collect variables
  NCollection_Sequence<asiAlgo_Variable> vars;
  //
  if ( m_mode == Mode_Read )
    this->initialize_Read(vars);
  else
    this->initialize_Write(vars);

  // Prepare properties to access the item
  const Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  // Populate table
  int current_row = 0;
  for ( int v = 1; v <= vars.Length(); ++v )
  {
    const asiAlgo_Variable& var = vars(v);

    // Insert table row
    m_widgets.pOptions->insertRow(current_row);

    // Table item for name
    QTableWidgetItem* pNameItem = new QTableWidgetItem( AsciiStr2QStr(var.Name) );
    pNameItem->setFlags(flags);
    pNameItem->setToolTip( AsciiStr2QStr(var.Description) );
    m_widgets.pOptions->setItem(current_row, 0, pNameItem);

    // Get variable value from statics if it is available there
    QString valueStr;
    Standard_CString cname = var.Name.ToCString();
    //
    if ( Interface_Static::IsPresent(cname) )
    {
      TCollection_AsciiString strVal;
      if ( var.Type == var_INTEGER )
      {
        strVal = Interface_Static::IVal(cname);
      }
      else if ( var.Type == var_REAL )
      {
        strVal = Interface_Static::RVal(cname);
      }
      else
      {
        strVal = Interface_Static::CVal(cname);
      }
      valueStr = AsciiStr2QStr(strVal);
    }
    else
    {
      valueStr = "UNDEFINED";
    }

    // Table widget item for value with the flag for editing
    QTableWidgetItem *pValueItem = new QTableWidgetItem(valueStr);
    pValueItem->setData( asiUI_DialogSTEPDelegate::WidgetRole, QVariant(var.Type) );
    pValueItem->setFlags( flags | Qt::ItemIsEditable );
    m_widgets.pOptions->setItem(current_row, 1, pValueItem);

    // Switch to the next row
    current_row++;
  }

  // Choose good ratio of column sizes
  m_widgets.pOptions->resizeColumnsToContents();
}

//! Initializes variable for reading.
//! \param vars [out] initialized variables.
void asiUI_DialogSTEP::initialize_Read(NCollection_Sequence<asiAlgo_Variable>& vars)
{
  vars.Append( asiAlgo_Variable( var_STRING,  xstep_cascade_unit,           xstep_cascade_unit_descr           ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_step_product_mode,       read_step_product_mode_descr       ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_step_product_context,    read_step_product_context_descr    ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_step_shape_repr,         read_step_shape_repr_descr         ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_step_assembly_level,     read_step_assembly_level_descr     ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_step_shape_relationship, read_step_shape_relationship_descr ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_step_shape_aspect,       read_step_shape_aspect_descr       ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_precision_mode,          read_precision_mode_descr          ) );
  vars.Append( asiAlgo_Variable( var_REAL,    read_precision_val,           read_precision_val_descr           ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_maxprecision_mode,       read_maxprecision_mode_descr       ) );
  vars.Append( asiAlgo_Variable( var_REAL,    read_maxprecision_val,        read_maxprecision_val_descr        ) );
  vars.Append( asiAlgo_Variable( var_STRING,  read_step_resource_name,      read_step_sequence_descr           ) );
  vars.Append( asiAlgo_Variable( var_STRING,  read_step_sequence,           read_step_sequence_descr           ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_surfacecurve_mode,       read_surfacecurve_mode_descr       ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_stdsameparameter_mode,   read_stdsameparameter_mode_descr   ) );
  vars.Append( asiAlgo_Variable( var_REAL,    read_encoderegularity_angle,  read_encoderegularity_angle_descr  ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, read_step_nonmanifold,        read_step_nonmanifold_descr        ) );
}

//! Initializes variable for writing.
//! \param vars [out] initialized variables.
void asiUI_DialogSTEP::initialize_Write(NCollection_Sequence<asiAlgo_Variable>& vars)
{
  vars.Append( asiAlgo_Variable( var_INTEGER, write_step_schema,        write_step_schema_descr       ) );
  vars.Append( asiAlgo_Variable( var_STRING,  write_step_product_name,  write_step_product_name_descr ) );
  vars.Append( asiAlgo_Variable( var_STRING,  write_step_unit,          write_step_unit_descr         ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, write_step_assembly,      write_step_assembly_descr     ) );
  vars.Append( asiAlgo_Variable( var_STRING,  write_step_resource_name, write_step_sequence_descr     ) );
  vars.Append( asiAlgo_Variable( var_STRING,  write_step_sequence,      write_step_sequence_descr     ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, write_surfacecurve_mode,  write_surfacecurve_mode_descr ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, write_precision_mode,     write_precision_mode_descr    ) );
  vars.Append( asiAlgo_Variable( var_REAL,    write_precision_val,      write_precision_val_descr     ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, write_step_vertex_mode,   write_step_vertex_mode_descr  ) );
  vars.Append( asiAlgo_Variable( var_INTEGER, write_step_nonmanifold,   write_step_nonmanifold_descr  ) );
}

//-----------------------------------------------------------------------------
// Save variables
//-----------------------------------------------------------------------------

//! Saves variable values.
void asiUI_DialogSTEP::saveVars()
{
  if ( !m_widgets.pOptions )
    return;

  // Get the actual variables from the table
  for ( int row = 0; row < m_widgets.pOptions->rowCount(); ++row )
  {
    QTableWidgetItem* pNameItem  = m_widgets.pOptions->item(row, 0);
    QTableWidgetItem* pValueItem = m_widgets.pOptions->item(row, 1);

    // Get current name and value
    QString nameStr = pNameItem->text();
    QString valueStr = pValueItem->text();
    //
    TCollection_AsciiString asciiName = QStr2AsciiStr(nameStr);
    TCollection_AsciiString asciiValue = QStr2AsciiStr(valueStr);

    // Get variable type
    const int var_type = pValueItem->data(asiUI_DialogSTEPDelegate::WidgetRole).toInt();

    // Set interface value
    if ( var_type == var_INTEGER )
      Interface_Static::SetIVal( asciiName.ToCString(), asciiValue.IntegerValue() );
    else if ( var_type == var_REAL )
      Interface_Static::SetRVal( asciiName.ToCString(), asciiValue.RealValue() );
    else
      Interface_Static::SetCVal( asciiName.ToCString(), asciiValue.ToCString() );
  }
}

//-----------------------------------------------------------------------------
// Write
//-----------------------------------------------------------------------------

//! Writes STEP.
void asiUI_DialogSTEP::proceed_Write()
{
  QString filename = asiUI_Common::selectSTEPFile(asiUI_Common::OpenSaveAction_Save);

  // Check Geometry Node
  if ( m_part.IsNull() || !m_part->IsWellFormed() )
    return;

  // Shape to save
  TopoDS_Shape targetShape = m_part->GetShape();
  if ( targetShape.IsNull() )
  {
    std::cout << "Error: target shape is null" << std::endl;
    return;
  }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  // Save
  if ( !asiAlgo_STEP::Write( targetShape, QStr2AsciiStr(filename) ) )
  {
    std::cout << "Error: cannot save shape" << std::endl;
  }

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
// Read
//-----------------------------------------------------------------------------

//! Reads STEP.
void asiUI_DialogSTEP::proceed_Read()
{
  QString filename = asiUI_Common::selectSTEPFile(asiUI_Common::OpenSaveAction_Open);

  // Check Geometry Node
  if ( m_part.IsNull() || !m_part->IsWellFormed() )
    return;

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  // Read STEP
  TopoDS_Shape shape;
  if ( !asiAlgo_STEP::Read(QStr2AsciiStr(filename), false, shape) )
  {
    std::cout << "Error: cannot read STEP file" << std::endl;
    QApplication::restoreOverrideCursor();
    return;
  }

  // Update part
  Handle(asiEngine_Model) M = Handle(asiEngine_Model)::DownCast(m_model);
  //
  M->OpenCommand(); // tx start
  {
    asiEngine_Part(M, NULL).Update(shape);
  }
  M->CommitCommand(); // tx commit

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
// Slots
//-----------------------------------------------------------------------------

//! On changing variable value.
//! \param pItem [in] table item being changed.
void asiUI_DialogSTEP::onVarChanged(QTableWidgetItem* pItem)
{
  Q_UNUSED(pItem);

  this->saveVars();
}

//! On proceed.
void asiUI_DialogSTEP::onProceed()
{
  if ( m_mode == Mode_Read )
    this->proceed_Read();
  else
    this->proceed_Write();

  this->close();
}
