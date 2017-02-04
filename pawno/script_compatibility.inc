/*
________________________________________________________________________________

						script_compatiblity - v1.0.1
					                By
								  Lordzy

This include allows you to build includes that are compatible with both
filterscripts and gamemode without any need for extra defines. You can also
run filterscripts as gamemode without any run-time errors using the init/exit
callbacks this include provides. In short, you no longer have to check
if FILTERSCRIPT is defined or not. This include contains two callbacks which
replaces the default Init/Exit callbacks.

LICENSE:
      This Source Code Form is subject to the
      terms of the Mozilla Public License, v.
      2.0. If a copy of the MPL was not
      distributed with this file, You can
      obtain one at
      http://mozilla.org/MPL/2.0/.


Changelog:

v1.0.1 - 2nd of February, 2017

- No more errors if any of the callback isn't defined by the user. It was first
meant to display error on purpose since using this include is required only if
needed to initialize run modes.

v1.0.0 - 1st of January, 2017

- Initial release.

Callbacks:
public OnScriptInit(); (replaces OnFilterScriptInit and OnGameModeInit)
public OnScriptExit(); (replaces OnFilterScriptExit and OnGameModeExit)

Function:
native GetInitMode();
//Returns SCRIPT_MODE_FS if running as filterscript.
//Returns SCRIPT_MODE_GM if running as gamemode.

Constants by their default values:

SCRIPT_MODE_FS = 1
SCRIPT_MODE_GM = 2

________________________________________________________________________________    */


#if defined _included_L_INIT
	#endinput
#endif

#define _included_L_INIT

#include <a_samp>

#if !defined SCRIPT_MODE_FS
	#define SCRIPT_MODE_FS 1
#endif

#if !defined SCRIPT_MODE_GM
	#define SCRIPT_MODE_GM 2
#endif


static
	L_INIT_MODE = 0
;

stock GetInitMode() {

	return L_INIT_MODE;
}

#if defined OnScriptInit
	forward OnScriptInit();
#endif

#if defined OnScriptExit
	forward OnScriptExit();
#endif

public OnFilterScriptInit() {

	if(!L_INIT_MODE) {

	    L_INIT_MODE = SCRIPT_MODE_FS;
	    #if defined OnScriptInit
			OnScriptInit();
		#endif
	}
	#if defined L_INIT_ONFS
	    return L_INIT_ONFS();
	#else
	    return 1;
	#endif
}

public OnGameModeInit() {

	if(!L_INIT_MODE) {

	    L_INIT_MODE = SCRIPT_MODE_GM;
	    #if defined OnScriptInit
	    	OnScriptInit();
		#endif
	}
	#if defined L_INIT_ONGM
	    return L_INIT_ONGM();
	#else
	    return 1;
	#endif
}

public OnFilterScriptExit() {

	if(L_INIT_MODE == SCRIPT_MODE_FS) {

		#if defined OnScriptExit
	    	OnScriptExit();
		#endif
	}
	#if defined L_EXIT_ONFS
	    return L_EXIT_ONFS();
	#else
	    return 1;
	#endif
}

public OnGameModeExit() {

	if(L_INIT_MODE == SCRIPT_MODE_GM) {
	
	    #if defined OnScriptExit
	    	OnScriptExit();
		#endif
	}
	#if defined L_EXIT_ONGM
	    return L_EXIT_ONGM();
	#else
	    return 1;
	#endif
}

//There's no problem if filterscript uses an entry point, it will simply be ignored.
main() {

	#if defined L_SCRIPT_main
	L_SCRIPT_main();
	#endif
}

#if defined _ALS_main
	#undef main
#else
	#define _ALS_main
#endif

#if defined _ALS_OnFilterScriptInit
	#undef OnFilterScriptInit
#else
	#define _ALS_OnFilterScriptInit
#endif

#if defined _ALS_OnGameModeInit
	#undef OnGameModeInit
#else
	#define _ALS_OnGameModeInit
#endif

#if defined _ALS_OnFilterScriptExit
	#undef OnFilterScriptExit
#else
	#define _ALS_OnFilterScriptExit
#endif

#if defined _ALS_OnGameModeExit
	#undef OnGameModeExit
#else
	#define _ALS_OnGameModeExit
#endif


#define main                L_SCRIPT_main
#define OnFilterScriptInit  L_INIT_ONFS
#define OnGameModeInit      L_INIT_ONGM
#define OnFilterScriptExit  L_EXIT_ONFS
#define OnGameModeExit      L_EXIT_ONGM

#if defined L_SCRIPT_main
	forward L_SCRIPT_main();
#endif

#if defined L_INIT_ONFS
	forward L_INIT_ONFS();
#endif

#if defined L_INIT_ONGM
	forward L_INIT_ONGM();
#endif

#if defined L_EXIT_ONFS
	forward L_EXIT_ONFS();
#endif

#if defined L_EXIT_ONGM
	forward L_EXIT_ONGM();
#endif

/*
public OnScriptInit() {

	print("Let's see if init is called twice");
	printf("Running as : %s", (GetInitMode() == SCRIPT_MODE_FS) ? ("FS") : ("GM"));

	return 1;
}

public OnScriptExit() {

	print("Let's see if exit is called twice");
	printf("Exiting as : %s", (GetInitMode() == SCRIPT_MODE_FS) ? ("FS") : ("GM"));

	return 1;
}*/

