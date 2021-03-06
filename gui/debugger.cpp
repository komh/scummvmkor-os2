/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/debug.h"
#include "common/system.h"

#include "gui/debugger.h"
#if USE_CONSOLE
	#include "gui/console.h"
#endif

namespace GUI {

Debugger::Debugger() {
	_frame_countdown = 0;
	_detach_now = false;
	_isAttached = false;
	_errStr = NULL;
	_firstTime = true;
#if USE_CONSOLE
	_debuggerDialog = new GUI::ConsoleDialog(1.0f, 0.67f);
	_debuggerDialog->setInputCallback(debuggerInputCallback, this);
	_debuggerDialog->setCompletionCallback(debuggerCompletionCallback, this);
#endif

	//DCmd_Register("continue",			WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("exit",				WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("quit",				WRAP_METHOD(Debugger, Cmd_Exit));

	DCmd_Register("help",				WRAP_METHOD(Debugger, Cmd_Help));

	DCmd_Register("debugflag_list",		WRAP_METHOD(Debugger, Cmd_DebugFlagsList));
	DCmd_Register("debugflag_enable",	WRAP_METHOD(Debugger, Cmd_DebugFlagEnable));
	DCmd_Register("debugflag_disable",	WRAP_METHOD(Debugger, Cmd_DebugFlagDisable));
}

Debugger::~Debugger() {
#if USE_CONSOLE
	delete _debuggerDialog;
#endif
}


// Initialisation Functions
int Debugger::DebugPrintf(const char *format, ...) {
	va_list	argptr;

	va_start(argptr, format);
	int count;
#if USE_CONSOLE
	count = _debuggerDialog->vprintf(format, argptr);
#else
	count = ::vprintf(format, argptr);
#endif
	va_end (argptr);
	return count;
}

void Debugger::attach(const char *entry) {

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	if (entry) {
		_errStr = strdup(entry);
	}

	_frame_countdown = 1;
	_detach_now = false;
	_isAttached = true;
}

void Debugger::detach() {
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	_detach_now = false;
	_isAttached = false;
}

// Temporary execution handler
void Debugger::onFrame() {
	if (_frame_countdown == 0)
		return;
	--_frame_countdown;

	if (!_frame_countdown) {

		preEnter();
		enter();
		postEnter();

		// Detach if we're finished with the debugger
		if (_detach_now)
			detach();
	}
}

// Main Debugger Loop
void Debugger::enter() {
#if USE_CONSOLE
	if (_firstTime) {
		DebugPrintf("Debugger started, type 'exit' to return to the game.\n");
		DebugPrintf("Type 'help' to see a little list of commands and variables.\n");
		_firstTime = false;
	}

	if (_errStr) {
		DebugPrintf("ERROR: %s\n\n", _errStr);
		free(_errStr);
		_errStr = NULL;
	}

	_debuggerDialog->runModal();
#else
	// TODO: compared to the console input, this here is very bare bone.
	// For example, no support for tab completion and no history. At least
	// we should re-add (optional) support for the readline library.
	// Or maybe instead of choosing between a console dialog and stdio,
	// we should move that choice into the ConsoleDialog class - that is,
	// the console dialog code could be #ifdef'ed to not print to the dialog
	// but rather to stdio. This way, we could also reuse the command history
	// and tab completion of the console. It would still require a lot of
	// work, but at least no dependency on a 3rd party library...

	printf("Debugger entered, please switch to this console for input.\n");

	int i;
	char buf[256];

	do {
		printf("debug> ");
		if (!fgets(buf, sizeof(buf), stdin))
			return;

		i = strlen(buf);
		while (i > 0 && buf[i - 1] == '\n')
			buf[--i] = 0;

		if (i == 0)
			continue;
	} while (parseCommand(buf));

#endif
}

bool Debugger::handleCommand(int argc, const char **argv, bool &result) {
	if (_cmds.contains(argv[0])) {
		Debuglet *debuglet = _cmds[argv[0]].get();
		assert(debuglet);
		result = (*debuglet)(argc, argv);
		return true;
	}

	return false;
}

// Command execution loop
bool Debugger::parseCommand(const char *inputOrig) {
	int num_params = 0;
	const char *param[256];
	char *input = strdup(inputOrig);	// One of the rare occasions using strdup is OK (although avoiding strtok might be more elegant here).

	// Parse out any params
	char *tok = strtok(input, " ");
	if (tok) {
		do {
			param[num_params++] = tok;
		} while ((tok = strtok(NULL, " ")) != NULL);
	} else {
		param[num_params++] = input;
	}

	// Handle commands first
	bool result;
	if (handleCommand(num_params, param, result)) {
		free(input);
		return result;
	}

	// It's not a command, so things get a little tricky for variables. Do fuzzy matching to ignore things like subscripts.
	for (uint i = 0; i < _dvars.size(); i++) {
		if (!strncmp(_dvars[i].name.c_str(), param[0], _dvars[i].name.size())) {
			if (num_params > 1) {
				// Alright, we need to check the TYPE of the variable to deref and stuff... the array stuff is a bit ugly :)
				switch (_dvars[i].type) {
				// Integer
				case DVAR_BYTE:
					*(byte *)_dvars[i].variable = atoi(param[1]);
					DebugPrintf("byte%s = %d\n", param[0], *(byte *)_dvars[i].variable);
					break;
				case DVAR_INT:
					*(int32 *)_dvars[i].variable = atoi(param[1]);
					DebugPrintf("(int)%s = %d\n", param[0], *(int32 *)_dvars[i].variable);
					break;
				// Integer Array
				case DVAR_INTARRAY: {
					char *chr = (char *)strchr(param[0], '[');
					if (!chr) {
						DebugPrintf("You must access this array as %s[element]\n", param[0]);
					} else {
						int element = atoi(chr+1);
						int32 *var = *(int32 **)_dvars[i].variable;
						if (element >= _dvars[i].optional) {
							DebugPrintf("%s is out of range (array is %d elements big)\n", param[0], _dvars[i].optional);
						} else {
							var[element] = atoi(param[1]);
							DebugPrintf("(int)%s = %d\n", param[0], var[element]);
						}
					}
					}
					break;
				default:
					DebugPrintf("Failed to set variable %s to %s - unknown type\n", _dvars[i].name.c_str(), param[1]);
					break;
				}
			} else {
				// And again, type-dependent prints/defrefs. The array one is still ugly.
				switch (_dvars[i].type) {
				// Integer
				case DVAR_BYTE:
					DebugPrintf("(byte)%s = %d\n", param[0], *(const byte *)_dvars[i].variable);
					break;
				case DVAR_INT:
					DebugPrintf("(int)%s = %d\n", param[0], *(const int32 *)_dvars[i].variable);
					break;
				// Integer array
				case DVAR_INTARRAY: {
					const char *chr = strchr(param[0], '[');
					if (!chr) {
						DebugPrintf("You must access this array as %s[element]\n", param[0]);
					} else {
						int element = atoi(chr+1);
						const int32 *var = *(const int32 **)_dvars[i].variable;
						if (element >= _dvars[i].optional) {
							DebugPrintf("%s is out of range (array is %d elements big)\n", param[0], _dvars[i].optional);
						} else {
							DebugPrintf("(int)%s = %d\n", param[0], var[element]);
						}
					}
				}
				break;
				// String
				case DVAR_STRING:
					DebugPrintf("(string)%s = %s\n", param[0], ((Common::String *)_dvars[i].variable)->c_str());
					break;
				default:
					DebugPrintf("%s = (unknown type)\n", param[0]);
					break;
				}
			}

			free(input);
			return true;
		}
	}

	DebugPrintf("Unknown command or variable\n");
	free(input);
	return true;
}

// returns true if something has been completed
// completion has to be delete[]-ed then
bool Debugger::tabComplete(const char *input, Common::String &completion) const {
	// very basic tab completion
	// for now it just supports command completions

	// adding completions of command parameters would be nice (but hard) :-)
	// maybe also give a list of possible command completions?
	//   (but this will require changes to console)

	if (strchr(input, ' '))
		return false; // already finished the first word

	const uint inputlen = strlen(input);

	completion.clear();

	CommandsMap::const_iterator i, e = _cmds.end();
	for (i = _cmds.begin(); i != e; ++i) {
		if (i->_key.hasPrefix(input)) {
			uint commandlen = i->_key.size();
			if (commandlen == inputlen) { // perfect match, so no tab completion possible
				return false;
			}
			if (commandlen > inputlen) { // possible match
				// no previous match
				if (completion.empty()) {
					completion = i->_key.c_str() + inputlen;
				} else {
					// take common prefix of previous match and this command
					for (uint j = 0; j < completion.size(); j++) {
						if (inputlen + j >= i->_key.size() ||
								completion[j] != i->_key[inputlen + j]) {
							completion = Common::String(completion.begin(), completion.begin() + j);
							// If there is no unambiguous completion, abort
							if (completion.empty())
								return false;
							break;
						}
					}
				}
			}
		}
	}
	if (completion.empty())
		return false;

	return true;
}

// Variable registration function
void Debugger::DVar_Register(const Common::String &varname, void *pointer, int type, int optional) {
	// TODO: Filter out duplicates
	// TODO: Sort this list? Then we can do binary search later on when doing lookups.
	assert(pointer);

	DVar tmp;
	tmp.name = varname;
	tmp.type = type;
	tmp.variable = pointer;
	tmp.optional = optional;

	_dvars.push_back(tmp);
}

// Command registration function
void Debugger::DCmd_Register(const Common::String &cmdname, Debuglet *debuglet) {
	assert(debuglet && debuglet->isValid());
	_cmds[cmdname] = Common::SharedPtr<Debuglet>(debuglet);
}


// Detach ("exit") the debugger
bool Debugger::Cmd_Exit(int argc, const char **argv) {
	_detach_now = true;
	return false;
}

// Print a list of all registered commands (and variables, if any),
// nicely word-wrapped.
bool Debugger::Cmd_Help(int argc, const char **argv) {
#if USE_CONSOLE
	const int charsPerLine = _debuggerDialog->getCharsPerLine();
#else
	const int charsPerLine = 80;
#endif
	int width, size;
	uint i;

	DebugPrintf("Commands are:\n");

	// Obtain a list of sorted command names
	Common::StringList cmds;
	CommandsMap::const_iterator iter, e = _cmds.end();
	for (iter = _cmds.begin(); iter != e; ++iter) {
		cmds.push_back(iter->_key);
	}
	sort(cmds.begin(), cmds.end());

	// Print them all
	width = 0;
	for (i = 0; i < cmds.size(); i++) {
		size = cmds[i].size() + 1;

		if ((width + size) >= charsPerLine) {
			DebugPrintf("\n");
			width = size;
		} else
			width += size;

		DebugPrintf("%s ", cmds[i].c_str());
	}
	DebugPrintf("\n");

	if (!_dvars.empty()) {
		DebugPrintf("\n");
		DebugPrintf("Variables are:\n");
		width = 0;
		for (i = 0; i < _dvars.size(); i++) {
			size = _dvars[i].name.size() + 1;

			if ((width + size) >= charsPerLine) {
				DebugPrintf("\n");
				width = size;
			} else
				width += size;

			DebugPrintf("%s ", _dvars[i].name.c_str());
		}
		DebugPrintf("\n");
	}

	return true;
}

bool Debugger::Cmd_DebugFlagsList(int argc, const char **argv) {
	const Common::DebugChannelList &debugLevels = Common::listDebugChannels();

	DebugPrintf("Engine debug levels:\n");
	DebugPrintf("--------------------\n");
	if (debugLevels.empty()) {
		DebugPrintf("No engine debug levels\n");
		return true;
	}
	for (Common::DebugChannelList::const_iterator i = debugLevels.begin(); i != debugLevels.end(); ++i) {
		DebugPrintf("%c%s - %s (%s)\n", i->enabled ? '+' : ' ',
				i->name.c_str(), i->description.c_str(),
				i->enabled ? "enabled" : "disabled");
	}
	DebugPrintf("\n");
	return true;
}

bool Debugger::Cmd_DebugFlagEnable(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("debugflag_enable <flag>\n");
	} else {
		if (Common::enableDebugChannel(argv[1])) {
			DebugPrintf("Enabled debug flag '%s'\n", argv[1]);
		} else {
			DebugPrintf("Failed to enable debug flag '%s'\n", argv[1]);
		}
	}
	return true;
}

bool Debugger::Cmd_DebugFlagDisable(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("debugflag_disable <flag>\n");
	} else {
		if (Common::disableDebugChannel(argv[1])) {
			DebugPrintf("Disabled debug flag '%s'\n", argv[1]);
		} else {
			DebugPrintf("Failed to disable debug flag '%s'\n", argv[1]);
		}
	}
	return true;
}

// Console handler
#if USE_CONSOLE
bool Debugger::debuggerInputCallback(GUI::ConsoleDialog *console, const char *input, void *refCon) {
	Debugger *debugger = (Debugger *)refCon;

	return debugger->parseCommand(input);
}


bool Debugger::debuggerCompletionCallback(GUI::ConsoleDialog *console, const char *input, Common::String &completion, void *refCon) {
	Debugger *debugger = (Debugger *)refCon;

	return debugger->tabComplete(input, completion);
}

#endif

}	// End of namespace GUI
