// Copyright (c) 2011 Zeex
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CRASHDETECT_H
#define CRASHDETECT_H

#include <stack>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "amxdebuginfo.h"
#include "configreader.h"

#define AMX_EXEC_GDK (-10) // Compatibility with GDK

class crashdetect {
public:	
	static bool Load(void **ppPluginData);
	static void Unload();
	static int AmxLoad(AMX *amx);
	static int AmxUnload(AMX *amx);

	static int AMXAPI AmxDebug(AMX *amx);
	static int AMXAPI AmxCallback(AMX *amx, cell index, cell *result, cell *params);
	static int AMXAPI AmxExec(AMX *amx, cell *retval, int index);

	static void Crash();
	static void RuntimeError(AMX *amx, cell index, int error);
	static void Interrupt();
	static void ExitOnError();	

	void HandleCrash();
	void HandleNativeError(int index);
	void HandleRuntimeError(int index, int error);
	void HandleInterrupt();

	int HandleAmxDebug();
	int HandleAmxCallback(cell index, cell *result, cell *params);
	int HandleAmxExec(cell *retval, int index);

	void PrintCallStack() const;

private:
	explicit crashdetect(AMX *amx);

	// Helpers
	static void *GetJMPAbsoluteAddress(unsigned char *jmp);
	static std::string GetModuleNameBySymbol(void *symbol);
	static bool GetNativeInfo(AMX *amx, cell index, AMX_NATIVE_INFO &info);
	static const char *GetNativeName(AMX *amx, cell index);
	static AMX_NATIVE GetNativeAddress(AMX *amx, cell index);
	static ucell GetPublicAddress(AMX *amx, cell index);	

private:
	AMX         *amx_;
	AMX_HEADER  *amxhdr_;
	AMXDebugInfo debugInfo_;

	/// The path to the .amx file
	std::string amxPath_;
	/// The name of the .amx file (including extension)
	std::string amxName_;

	AMX_CALLBACK prevCallback_;
	AMX_DEBUG    prevDebugHook_;	

	class NativePublicCall {
	public:
		enum Type {
			NATIVE,
			PUBLIC
		};

		NativePublicCall(Type type, AMX *amx, cell index, ucell frm)
			: type_(type), amx_(amx), index_(index), frm_(frm) {}

		Type type() const 
			{ return type_; }
		AMX *amx() const 
			{ return amx_; }
		cell index() const 
			{ return index_; }
		ucell frm() const 
			{ return frm_; }

	private:
		Type type_;
		AMX *amx_;
		ucell frm_;
		cell index_;
	};

	/// Native/public calls
	static std::stack<NativePublicCall> npCalls_;

	/// Set to true on Runtime Error (to prevent double-reporting)
	static bool errorCaught_;

	/// Server config reader (server.cfg)
	static ConfigReader serverCfg;

	/// AMX* <=> crashdetect*
	static boost::unordered_map<AMX*, boost::shared_ptr<crashdetect> > instances_;
};

#endif // !CRASHDETECT_H
