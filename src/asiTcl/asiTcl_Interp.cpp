//-----------------------------------------------------------------------------
// Created on: 22 August 2017
// Created by: Sergey SLYADNEV
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

// Own include
#include <asiTcl_Interp.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// Tcl includes
#include <tcl.h>

// OCCT includes
#include <OSD_Path.hxx>

//-----------------------------------------------------------------------------
// Custom channel binding inspired by the code taken from tkConsole.c
//-----------------------------------------------------------------------------

/*
 * Prototypes for local procedures defined in this file:
 */

static int ConsoleClose(ClientData instanceData,
                        Tcl_Interp* interp);

static int ConsoleHandle(ClientData  instanceData,
                         int         direction,
                         ClientData* handlePtr);

static int ConsoleInput(ClientData instanceData,
                        char*      buf,
                        int        toRead,
                        int*       errorCode);

static int ConsoleOutput(ClientData  instanceData,
                         const char* buf,
                         int         toWrite,
                         int*        errorCode);

static void	ConsoleWatch(ClientData instanceData,
                         int        mask);

static const Tcl_ChannelType consoleChannelType = {
    "console",              /* Type name. */
    TCL_CHANNEL_VERSION_4,  /* v4 channel */
    ConsoleClose,           /* Close proc. */
    ConsoleInput,           /* Input proc. */
    ConsoleOutput,          /* Output proc. */
    nullptr,                   /* Seek proc. */
    nullptr,                   /* Set option proc. */
    nullptr,                   /* Get option proc. */
    ConsoleWatch,           /* Watch for events on console. */
    ConsoleHandle,          /* Get a handle from the device. */
    nullptr,                   /* close2proc. */
    nullptr,                   /* Always non-blocking.*/
    nullptr,                   /* flush proc. */
    nullptr,                   /* handler proc. */
    nullptr,                   /* wide seek proc */
    nullptr,                   /* thread action proc */
    nullptr
};

/*
 * Each console is associated with an instance of the ConsoleInfo struct.
 * A refCount permits the struct to be shared as instance data
 * by commands and by channels.
 */

struct ConsoleInfo
{
  Handle(asiTcl_Interp) asiInterp;
  int                   refCount;

  ConsoleInfo() : asiInterp(nullptr), refCount(0) {} //!< Default ctor.
};

/*
 * Each console channel holds an instance of the ChannelData struct as
 * its instance data.  It contains ConsoleInfo, so the channel can work
 * with the appropriate console window, and a type value to distinguish
 * the stdout channel from the stderr channel.
 */

struct ChannelData
{
  ConsoleInfo* info;
  int          type; /* TCL_STDOUT or TCL_STDERR */

  ChannelData() : info(nullptr), type(TCL_STDOUT) {} //!< Default ctor.
};

/*
 *----------------------------------------------------------------------
 *
 * ConsoleOutput
 *
 * Writes the given output on the IO channel. Returns count of how many
 * characters were actually written, and an error indication.
 *
 * Results:
 *    A count of how many characters were written is returned and an error
 *    indication is returned in an output argument.
 *
 * Side effects:
 *    Writes output on the actual channel.
 *
 *----------------------------------------------------------------------
 */

static int ConsoleOutput(ClientData  instanceData, /* Indicates which device to use. */
                         const char* buf,          /* The data buffer. */
                         int         toWrite,      /* How many bytes to write? */
                         int*        errorCode)    /* Where to store error code. */
{
  ChannelData *data = (ChannelData*) instanceData;
  ConsoleInfo *info = data->info;

  *errorCode = 0;
  Tcl_SetErrno(0);

  TCollection_AsciiString bufStr(buf);

  if ( info->asiInterp )
    if ( bufStr != "\n" )
      info->asiInterp->GetProgress().SendLogMessage(LogInfo(Normal) << bufStr);

  return toWrite;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleInput
 *
 *  Reads input from the console. Not currently implemented.
 *
 * Results:
 *    Always returns EOF.
 *
 * Side effects:
 *    None.
 *
 *----------------------------------------------------------------------
 */

static int ConsoleInput(ClientData asiTcl_NotUsed(instanceData), /* Unused. */
                        char*      asiTcl_NotUsed(buf),          /* Where to store data read. */
                        int        asiTcl_NotUsed(bufSize),      /* How much space is available in the buffer? */
                        int*       asiTcl_NotUsed(errorCode))    /* Where to store error code. */
{
  return 0; /* Always return EOF. */
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleClose
 *
 *  Closes the IO channel.
 *
 * Results:
 *    Always returns 0 (success).
 *
 * Side effects:
 *    Frees the dummy file associated with the channel.
 *
 *----------------------------------------------------------------------
 */

static int ConsoleClose(ClientData  instanceData,
                        Tcl_Interp* asiTcl_NotUsed(interp))
{
  ChannelData *data = (ChannelData*) instanceData;
  ConsoleInfo *info = data->info;

  if ( info )
  {
    if ( --info->refCount <= 0 )
    {
      delete info;
    }
  }
  delete data;
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleWatch
 *
 *  Called by the notifier to set up the console device so that events
 *  will be noticed. Since there are no events on the console, this
 *  routine just returns without doing anything.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    None.
 *
 *----------------------------------------------------------------------
 */

static void ConsoleWatch(ClientData asiTcl_NotUsed(instanceData), /* Device ID for the channel. */
                         int        asiTcl_NotUsed(mask))         /* OR-ed combination of TCL_READABLE,
                                                                   * TCL_WRITABLE and TCL_EXCEPTION, for the
                                                                   * events we are interested in. */
{}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleHandle
 *
 *  Invoked by the generic IO layer to get a handle from a channel.
 *  Because console channels are not devices, this function always fails.
 *
 * Results:
 *    Always returns TCL_ERROR.
 *
 * Side effects:
 *    None.
 *
 *----------------------------------------------------------------------
 */

static int ConsoleHandle(ClientData  asiTcl_NotUsed(instanceData), /* Device ID for the channel. */
                         int         asiTcl_NotUsed(direction),    /* TCL_READABLE or TCL_WRITABLE to indicate
                                                                    * which direction of the channel is being
                                                                    * requested. */
                         ClientData* asiTcl_NotUsed(handlePtr))    /* Where to store handle */
{
  return TCL_ERROR;
}

//-----------------------------------------------------------------------------

//! Function to pass to Tcl_CreateCommand as an entry point to any user-defined
//! command.
//! \param[in] pClientData callback.
//! \param[in] pInterpTcl  Tcl interpreter.
//! \param[in] argc        number of command line arguments.
//! \param[ni] argv        command line arguments.
//! \return Tcl error code.
static int TclProcInvoke(void*       pClientData,
                         Tcl_Interp* pInterpTcl,
                         int         argc,
                         const char* argv[])
{
  static int code;
  code = TCL_OK;
  asiTcl_Interp::t_tcl_callback*
    pCallback = (asiTcl_Interp::t_tcl_callback*) pClientData;

  // Invoke
  try
  {
    int fres = pCallback->Invoke(argc, argv);

    if ( fres != TCL_OK )
      code = TCL_ERROR;
  }
  catch ( ... )
  {
    std::cerr << "Error: exception occurred" << std::endl;
    code = TCL_ERROR;
    //
    std::cerr << "\t*** " << Tcl_GetStringResult(pInterpTcl) << std::endl;
  }
  return code;
}

//-----------------------------------------------------------------------------

static void TclProcDelete(void* pClientData)
{
  asiTcl_Interp::t_tcl_callback*
    pCallback = (asiTcl_Interp::t_tcl_callback*) pClientData;

  delete pCallback;
}

//-----------------------------------------------------------------------------

static void OverrideTclChannel(const Handle(asiTcl_Interp)& interp,
                               const int                    channelType)
{
  ConsoleInfo* info = new ConsoleInfo;
  info->asiInterp   = interp;

  ChannelData* data = new ChannelData;
  data->info        = info;
  data->type        = channelType;
  data->info->refCount++;

  // Unregister standard channel (if any).
  Tcl_Channel nativeChannel = Tcl_GetStdChannel(channelType);
  //
  if ( nativeChannel != nullptr )
    Tcl_UnregisterChannel(interp->GetTclInterp(), nativeChannel);

  // Create custom channel.
  Tcl_Channel customChannel = Tcl_CreateChannel(&consoleChannelType,
                                               "console0",
                                                data,
                                                TCL_READABLE | TCL_WRITABLE);

  if ( customChannel != nullptr )
  {
    Tcl_SetChannelOption(nullptr, customChannel, "-translation", "lf");
    Tcl_SetChannelOption(nullptr, customChannel, "-buffering", "none");
    Tcl_SetChannelOption(nullptr, customChannel, "-encoding", "utf-8");

    Tcl_SetStdChannel(customChannel, channelType);
    Tcl_RegisterChannel(interp->GetTclInterp(), customChannel);
  }
}

//-----------------------------------------------------------------------------

asiTcl_Interp::asiTcl_Interp(ActAPI_ProgressEntry progress,
                             ActAPI_PlotterEntry  plotter)
: Standard_Transient (),
  m_bNotifierOn      (true),
  m_pInterp          (nullptr),
  m_progress         (progress),
  m_plotter          (plotter)
{}

//-----------------------------------------------------------------------------

asiTcl_Interp::~asiTcl_Interp()
{
  if ( m_pInterp )
    Tcl_DeleteInterp(m_pInterp);
}

//-----------------------------------------------------------------------------

void asiTcl_Interp::Init(const bool overrideChannels)
{
  if ( m_pInterp )
    Tcl_DeleteInterp(m_pInterp);

  m_pInterp = Tcl_CreateInterp();

  /* ================================
   *  Override standard Tcl channels
   * ================================ */

  if ( overrideChannels )
  {
    OverrideTclChannel(this, TCL_STDOUT);
    OverrideTclChannel(this, TCL_STDERR);
  }

  // Output available channels.
  Tcl_GetChannelNames(m_pInterp);
  //
  std::string channels = Tcl_GetStringResult(m_pInterp);
  //
  this->GetProgress().SendLogMessage( LogInfo(Normal) << "Available channels: %1."
                                                      << (channels.size() ? channels.c_str() : "none") );
}

//-----------------------------------------------------------------------------

void asiTcl_Interp::PrintLastError()
{
  std::string errStr = Tcl_GetStringResult(m_pInterp);

  m_progress.SendLogMessage( LogErr(Normal) << "Tcl last error: %1"
                                            << ( errStr.empty() ? "undefined." : errStr.c_str() ) );
}

//-----------------------------------------------------------------------------

int asiTcl_Interp::Eval(const TCollection_AsciiString& cmd)
{
  if ( !m_pInterp )
  {
    this->GetProgress().SendLogMessage(LogErr(Normal) << "Tcl command has finished with TCL_ERROR.");
    return TCL_ERROR;
  }

  // Evaluate.
  int ret = Tcl_Eval( m_pInterp, cmd.ToCString() );
  //
  if ( ret == TCL_ERROR )
    this->PrintLastError();

  return ret;
}

//-----------------------------------------------------------------------------

bool asiTcl_Interp::AddCommand(const TCollection_AsciiString& name,
                               const TCollection_AsciiString& help,
                               const TCollection_AsciiString& filename,
                               const TCollection_AsciiString& group,
                               t_user_func                    func)
{
  t_tcl_callback* pCallback = new t_tcl_callback(this, func);
  return this->addCommand(name, help, filename, group, pCallback);
}

//-----------------------------------------------------------------------------

bool asiTcl_Interp::DeleteCommand(const TCollection_AsciiString& name,
                                  const TCollection_AsciiString& group)
{
  return this->deleteCommand(name, group);
}

//-----------------------------------------------------------------------------

void asiTcl_Interp::GetAvailableCommands(std::vector<asiTcl_CommandInfo>& commands) const
{
  for ( size_t p = 0; p < m_plugins.size(); ++p )
  {
    std::string
      commandsInGroupStr = Tcl_GetVar2(m_pInterp, "asi_Groups", m_plugins[p].ToCString(),
                                       TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG);

    std::vector<std::string> commandsInGroup;
    asiAlgo_Utils::Str::Split(commandsInGroupStr, " ", commandsInGroup);

    for ( size_t c = 0; c < commandsInGroup.size(); ++c )
    {
      asiTcl_CommandInfo cmdInfo;
      cmdInfo.Group = m_plugins[p].ToCString();
      cmdInfo.Name  = commandsInGroup[c];

      // Help
      std::string
        commandHelp = Tcl_GetVar2(m_pInterp, "asi_Help", cmdInfo.Name.c_str(),
                                  TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG);
      //
      cmdInfo.Help = commandHelp;

      // Source
      std::string
        commandSrc = Tcl_GetVar2(m_pInterp, "asi_Files", cmdInfo.Name.c_str(),
                                 TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG);
      //
      cmdInfo.Filename = commandSrc;

      // Set to output
      commands.push_back(cmdInfo);
    }
  }
}

//-----------------------------------------------------------------------------

int asiTcl_Interp::ErrorOnWrongArgs(const char* cmd)
{
  std::cerr << "Error: Wrong number of arguments in command " << cmd << ".";
  this->GetProgress().SendLogMessage(LogErr(Normal) << "Wrong number of arguments in command %1." << cmd);

  return TCL_ERROR;
}

//-----------------------------------------------------------------------------

void asiTcl_Interp::GetIndicesOfKeys(const int                   argc,
                                     const char**                argv,
                                     TColStd_PackedMapOfInteger& ids) const
{
  for ( int k = 1; k < argc; ++k )
    if ( this->IsKeyword(argv[k]) )
      ids.Add(k);
}

//-----------------------------------------------------------------------------

bool asiTcl_Interp::IsKeyword(const TCollection_AsciiString& opt,
                              const TCollection_AsciiString& key) const
{
  if ( opt.IsEmpty() )
    return false;

  // Compose the reference key (to check with).
  TCollection_AsciiString key2check("-");
  if ( !key.IsEmpty() ) key2check += key;

  // If the passed reference key is not empty, let's do an equality check.
  if ( !key.IsEmpty() )
  {
    if ( opt == key2check )
      return true;
  }
  else // If the reference key is empty, let's check the '-' prefix only.
  {
    if ( opt.StartsWith(key2check) )
      return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiTcl_Interp::HasKeyword(const int                      argc,
                               const char**                   argv,
                               const TCollection_AsciiString& key) const
{
  int idx; // Not used.
  return HasKeyword(argc, argv, key, idx);
}

//-----------------------------------------------------------------------------

bool asiTcl_Interp::HasKeyword(const int                      argc,
                               const char**                   argv,
                               const TCollection_AsciiString& key,
                               int&                           idx) const
{
  for ( int k = 1; k < argc; ++k )
  {
    if ( this->IsKeyword(argv[k], key) )
    {
      idx = k;
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------

bool asiTcl_Interp::GetKeyValue(const int                      argc,
                                const char**                   argv,
                                const TCollection_AsciiString& key,
                                TCollection_AsciiString&       value) const
{
  for ( int k = 1; k < argc - 1; ++k )
  {
    if ( this->IsKeyword(argv[k], key) )
    {
      value = argv[k+1];
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------

void asiTcl_Interp::SetVar(const char* name,
                           const char* val)
{
  Tcl_UnsetVar (m_pInterp, name, 0);
  Tcl_SetVar   (m_pInterp, name, val, 0);
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::Append(const char* str)
{
  // Convert string to UTF-8 format for Tcl.
  Tcl_DString TclString;
  Tcl_ExternalToUtfDString(nullptr, str, -1, &TclString);
  Tcl_AppendResult(m_pInterp, Tcl_DStringValue(&TclString), (const char*) 0);
  Tcl_DStringFree(&TclString);
  //
  return *this;
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::operator<<(const char* str)
{
  return this->Append(str);
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::Append(const TCollection_AsciiString& str)
{
  return this->Append( str.ToCString() );
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::operator<<(const TCollection_AsciiString& str)
{
  return this->Append(str);
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::Append(const TCollection_ExtendedString& str)
{
  // Convert string to UTF-8 format for Tcl
  char *buff = new char[str.LengthOfCString() + 1];
  str.ToUTF8CString(buff);
  Tcl_AppendResult(m_pInterp, buff, (const char*) 0);
  delete[] buff;
  //
  return *this;
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::operator<<(const TCollection_ExtendedString& str)
{
  return this->Append(str);
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::Append(const std::string& str)
{
  return this->Append( str.c_str() );
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::operator<<(const std::string& str)
{
  return this->Append(str);
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::Append(const int value)
{
  char c[100];
  Sprintf(c, "%d", value);
  Tcl_AppendResult(m_pInterp, c, (const char*) 0);
  return *this;
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::operator<<(const int value)
{
  return this->Append(value);
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::Append(const double value)
{
  char s[100];
  Sprintf(s, "%.17g", value);
  Tcl_AppendResult(m_pInterp, s, (const char*) 0);
  return *this;
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::operator<<(const double value)
{
  return this->Append(value);
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::Append(const TColStd_PackedMapOfInteger& mask)
{
  int iter = 0;
  for ( TColStd_MapIteratorOfPackedMapOfInteger mit(mask); mit.More(); mit.Next() )
  {
    if ( iter++ )
      this->Append(" ");

    this->Append( mit.Key() );
  }

  return *this;
}

//-----------------------------------------------------------------------------

asiTcl_Interp& asiTcl_Interp::operator<<(const TColStd_PackedMapOfInteger& mask)
{
  return this->Append(mask);
}

//-----------------------------------------------------------------------------

bool asiTcl_Interp::addCommand(const TCollection_AsciiString& name,
                               const TCollection_AsciiString& help,
                               const TCollection_AsciiString& filename,
                               const TCollection_AsciiString& group,
                               t_tcl_callback*                callback)
{
  if ( !m_pInterp )
    return false;

  Tcl_Command cmd = Tcl_CreateCommand(m_pInterp,
                                      name.ToCString(),
                                      TclProcInvoke,
                                      (void*) callback,
                                      TclProcDelete);

  if ( cmd == nullptr )
  {
    this->GetProgress().SendLogMessage(LogErr(Normal) << "Tcl_CreateCommand has returned nullptr.");
    return false;
  }

  // Each custom Tcl command of Analysis Situs is supported with the following
  // additional information:
  //
  // - help string;
  // - filename of the source file where the command is implemented;
  // - group name to organize Tcl commands logically.
  //
  // All this is stored by means of dedicated Tcl variables (correspondingly):
  //
  // - asi_Help
  // - asi_Files
  // - asi_Groups

  // Help line
  Tcl_SetVar2(m_pInterp, "asi_Help", name.ToCString(), help.ToCString(),
              TCL_GLOBAL_ONLY);

  // Source filename
  if ( !filename.IsEmpty() )
  {
    // The following code for manipulation with paths is taken from
    // OpenCascade's Draw package.

    OSD_Path path(filename);
    for ( int i = 2; i < path.TrekLength(); ++i )
    {
      path.RemoveATrek(1);
    }
    path.SetDisk("");
    path.SetNode("");
    //
    TCollection_AsciiString srcPath;
    path.SystemName(srcPath);
    //
    if ( srcPath.Value(1) == '/' )
      srcPath.Remove(1);

    // Filename
    Tcl_SetVar2(m_pInterp, "asi_Files", name.ToCString(), srcPath.ToCString(),
                TCL_GLOBAL_ONLY);
  }

  // Group
  Tcl_SetVar2(m_pInterp, "asi_Groups", group.ToCString(), name.ToCString(),
              TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);

  return true;
}

//-----------------------------------------------------------------------------

bool
  asiTcl_Interp::deleteCommand(const TCollection_AsciiString& name,
                               const TCollection_AsciiString& group)
{
  if ( !m_pInterp )
    return false;

  /* Delete command itself. */

  Tcl_DeleteCommand( m_pInterp, name.ToCString() );

  /* Remove the command name from the asi_Groups array of lists. We do so
     by reassembling the list of command names and moving out the element
     which equals to the passed name. Is there a better way to do this
     using Tcl C API?
   */

  std::string
    commandsInGroupStr = Tcl_GetVar2(m_pInterp, "asi_Groups", group.ToCString(),
                                     TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG),
    newCommandsInGroupStr;

  std::vector<std::string> commandsInGroup;
  asiAlgo_Utils::Str::Split(commandsInGroupStr, " ", commandsInGroup);

  for ( size_t c = 0; c < commandsInGroup.size(); ++c )
  {
    if ( commandsInGroup[c] != name.ToCString() )
    {
      newCommandsInGroupStr += commandsInGroup[c];

      if ( c < commandsInGroup.size() - 1 )
        newCommandsInGroupStr += " ";
    }
  }

  Tcl_SetVar2(m_pInterp, "asi_Groups", group.ToCString(), newCommandsInGroupStr.c_str(),
              TCL_GLOBAL_ONLY);

  /* Remove the command name from the asi_Files array. */

  int ok1 = Tcl_UnsetVar2( m_pInterp, "asi_Files", name.ToCString(), TCL_GLOBAL_ONLY );
  //
  if ( ok1 == TCL_ERROR )
  {
    this->GetProgress().SendLogMessage(LogErr(Normal) << "Tcl_UnsetVar2 error on asi_Files.");
  }

  /* Remove the command name from the asi_Help array. */

  int ok2 = Tcl_UnsetVar2( m_pInterp, "asi_Help", name.ToCString(), TCL_GLOBAL_ONLY );
  //
  if ( ok2 == TCL_ERROR )
  {
    this->GetProgress().SendLogMessage(LogErr(Normal) << "Tcl_UnsetVar2 error on asi_Help.");
  }

  /* Finalize. */

  const bool isOk = ( (ok1 == TCL_OK) && (ok2 == TCL_OK) );

  if ( isOk )
  {
    this->GetProgress().SendLogMessage(LogInfo(Normal) << "The command %1 (%2) was unregistered."
                                                       << name << group);
  }

  return isOk;
}
