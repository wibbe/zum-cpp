
#include "Tcl.h"
#include "ScriptingLib.tcl.h"
#include "Editor.h"
#include "Log.h"

#include "bx/bx.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cassert>
#include <map>

extern "C" {
  int Jim_clockInit(Jim_Interp * interp);
  int Jim_regexpInit(Jim_Interp * interp);
}

namespace tcl {

  // -- Globals --

  static Jim_Interp * interpreter_ = nullptr;

  // -- Variable --

  static std::vector<Variable *> & buildInVariables()
  {
    static std::vector<Variable *> vars;
    return vars;
  }

  Variable::Variable(const char * name, const char * defaultValue)
    : name_(name),
      type_(ValueType::STRING),
      defaultStrValue_(defaultValue)
  {
    buildInVariables().push_back(this);
  }

  Variable::Variable(const char * name, int defaultValue)
    : name_(name),
      type_(ValueType::INT),
      defaultIntValue_(defaultValue)
  {
    buildInVariables().push_back(this);
  }

  Variable::Variable(const char * name, bool defaultValue)
    : name_(name),
      type_(ValueType::BOOL),
      defaultBoolValue_(defaultValue)
  {
    buildInVariables().push_back(this);
  }

  Jim_Obj * Variable::value() const
  {
    assert(interpreter_);
    return Jim_GetGlobalVariableStr(interpreter_, name_, 0);
  }

  Jim_Obj * Variable::defaultValue() const
  {
    assert(interpreter_);

    switch (type_)
    {
      case ValueType::STRING:
        return Jim_NewStringObj(interpreter_, defaultStrValue_, std::strlen(defaultStrValue_));

      case ValueType::INT:
        return Jim_NewIntObj(interpreter_, defaultIntValue_);

      case ValueType::BOOL:
        return Jim_NewIntObj(interpreter_, defaultBoolValue_ ? 1 : 0);
    }

    return nullptr;
  }

  std::string Variable::toStr() const
  {
    return std::string(Jim_String(value()));
  }

  bool Variable::toBool() const
  {
    return toInt() > 0;
  }

  int Variable::toInt() const
  {
    long val;
    if (Jim_GetLong(interpreter_, value(), &val) != JIM_OK)
      return 0;

    return val;
  }

  // -- BuiltInProc --

  static std::vector<std::pair<BuiltInProc *, Jim_CmdProc *>> & builtInProcs()
  {
    static std::vector<std::pair<BuiltInProc *, Jim_CmdProc *>> procs;
    return procs;
  }

  BuiltInProc::BuiltInProc(const char * name, Jim_CmdProc * proc)
    : name_(name)
  {
    builtInProcs().push_back({this, proc});
  }

  BuiltInProc::BuiltInProc(const char * name, const char * args, Jim_CmdProc * proc)
    : name_(name),
      args_(args)
  {
    builtInProcs().push_back({this, proc});
  }

  BuiltInProc::BuiltInProc(const char * name, const char * args, const char * desc, Jim_CmdProc * proc)
    : name_(name),
      args_(args),
      desc_(desc)
  {
    builtInProcs().push_back({this, proc});
  }

  // -- Interface --

  static const std::string CONFIG_FILE = "/.zumrc";

  void initialize()
  {
    interpreter_ = Jim_CreateInterp();
    Jim_RegisterCoreCommands(interpreter_);

    // Register extensions
    ::Jim_clockInit(interpreter_);
    ::Jim_regexpInit(interpreter_);

    // Register build in commands
    for (auto const& it : builtInProcs())
      Jim_CreateCommand(interpreter_, it.first->name_, it.second, it.first, nullptr);

    // Register build in variables
    for (auto * var : buildInVariables())
      Jim_SetGlobalVariableStr(interpreter_, var->name(), var->defaultValue());

    Jim_EvalSource(interpreter_, __FILE__, __LINE__, std::string((char *)&ScriptingLib[0], BX_COUNTOF(ScriptingLib)).c_str());

    logInfo("Loading ~/.zumrc");
    // Try to load the config file
    char * home = getenv("HOME");
    if (home)
      Jim_EvalFileGlobal(interpreter_, (std::string(home) + CONFIG_FILE).c_str());
    else
      Jim_EvalFileGlobal(interpreter_, ("~" + CONFIG_FILE).c_str());

/*
    evaluate(Str(std::string((char *)&ScriptingLib_tcl[0], ScriptingLib_tcl_len).c_str()));


    logInfo("Evaluating ~/.zumrc");
    // Try to load the config file
    char * home = getenv("HOME");
    if (home)
      execFile(std::string(home) + CONFIG_FILE);
    else
      execFile("~" + CONFIG_FILE);
*/
  }

  void shutdown()
  {
    Jim_FreeInterp(interpreter_);
    interpreter_ = nullptr;
  }

  bool evaluate(std::string const& code)
  {
    const bool ok = Jim_EvalGlobal(interpreter_, code.c_str()) == JIM_OK;
    if (!ok)
      logError(result());

    return ok;
  }

  std::string result()
  {
    return std::string(Jim_String(Jim_GetResult(interpreter_)));
  }

  std::vector<std::string> findMatches(std::string const& name)
  {
    std::vector<std::string> result;

    // Search for matching procs
    Jim_HashTableIterator * it = Jim_GetHashTableIterator(&interpreter_->commands);
    Jim_HashEntry * entry = nullptr;

    while ((entry = Jim_NextHashEntry(it)) != nullptr)
    {
      const std::string cmdName((const char *)Jim_GetHashEntryKey(entry));
      Jim_Cmd * cmd = (Jim_Cmd *)Jim_GetHashEntryVal(entry);

      if (cmdName.find(name) == 0)
        result.push_back(cmdName);
    }

    Jim_Free(it);

    // Search for matching global variables
    it = Jim_GetHashTableIterator(&interpreter_->topFramePtr->vars);

    while ((entry = Jim_NextHashEntry(it)) != nullptr)
    {
      const std::string varName((const char *)Jim_GetHashEntryKey(entry));
      //Jim_Cmd * cmd = (Jim_Cmd *)Jim_GetHashEntryVal(entry);

      if (varName.find(name) == 0)
        result.push_back(varName);
    }

    Jim_Free(it);

    return result;
  }

  TCL_FUNC(puts, "string ?string ...?")
  {
    TCL_CHECK_ARGS(2, 1000);

    std::string log;
    for (uint32_t i = 1; i < argc; ++i)
      log += Jim_String(argv[i]) + std::string((i + 1) == argc ? "" : " ");

    logInfo(log);
    flashMessage(log);

    return JIM_OK;
  }
}
