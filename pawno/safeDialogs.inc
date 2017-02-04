/*
________________________________________________________________________________


								safeDialogs.inc
								    v1.0
								By Lordzy

LICENSE:
      This Source Code Form is subject to the
      terms of the Mozilla Public License, v.
      2.0. If a copy of the MPL was not
      distributed with this file, You can
      obtain one at
      http://mozilla.org/MPL/2.0/.


safeDialogs prevents the server from receiving fake or spoofed dialog responses.
This includes faking dialog ID, listitem or even the inputtext in case of lists.
Faking listitem or inputtext can lead to many risks of user breaking into server
security depending on how they're used. This include ensures that everything's
safe and also contains a callback that is triggered whenever player sends
spoofed dialog response.

IMPORTANT NOTE : Include this on every script that uses dialog feature!

Callbacks:

OnDialogSpoof(playerid, spooftype);
playerid = The player who sent spoofed dialog response.
spooftype = Spoof type.
	DIALOG_SPOOFTYPE_DIALOG_ID - If sending fake dialog response.
	DIALOG_SPOOFTYPE_LIST_ITEM - If sending fake list-item response.
	DIALOG_SPOOFTYPE_INPUT_TEXT - If sending fake input text.
										(in case of lists)
	
Returns : Return 0 to prevent server receiving falsified data. Returning 1
		  will allow receiving false data. In short, ALWAYS RETURN 0!

Functions :

GetPlayerDialogID(playerid) - Returns the current dialog ID of player.
GetPlayerDialogStyle(playerid) - Returns the current dialog style of player.
GetPlayerDialogInfo(playerid, dest[], size = sizeof(dest))
	- Stores the dialog info on "dest" array.


//For syntax highlighting:
native GetPlayerDialog(playerid);
native GetPlayerDialogStyle(playerid);
native GetPlayerDialogInfo(playerid, dest[], size = sizeof(dest));

Please consider checking the release topic on SA-MP forums for further info or
queries.

________________________________________________________________________________    */

#if defined _included_safeDialogs
	#endinput
#endif

#define _included_safeDialogs

#include <script_compatibility>

//Configuration (optional)
#if !defined MAX_DIALOG_STRING
	#define MAX_DIALOG_STRING   2048
#endif

#if !defined MAX_DIALOG_LISTITEM_LEN
	#define MAX_DIALOG_LISTITEM_LEN 256
#endif

//Include macros
#define g_LSafeDialogs_Player{%0}[e_L_SD_%1] \
	g_LSafeDialogs_%1{%0}

#define g_LSafeDialogs_player[%0][e_L_SD_%1] \
	g_LSafeDialogs_%1[%0]


//Enumerators
enum e_L_SAFEDIALOGS_PLAYER {

	e_L_SD_pDIALOG_ID,
	e_L_SD_pDIALOG_LISTITEMS
}

enum {

	DIALOG_SPOOFTYPE_DIALOG_ID,
	DIALOG_SPOOFTYPE_LIST_ITEM,
	DIALOG_SPOOFTYPE_INPUT_TEXT
}

static

	g_LSafeDialogs_Player[MAX_PLAYERS][e_L_SAFEDIALOGS_PLAYER],
	g_LSafeDialogs_pDIALOG_STYLE[MAX_PLAYERS char],
	g_LSafeDialogs_pDIALOG_STRING[MAX_PLAYERS][MAX_DIALOG_STRING char]
;

//Internal function
static stock LSafeDialogs_ResetVars(playerid) {

	g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_ID] = -1;
	g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_LISTITEMS] = -1;
	g_LSafeDialogs_Player{playerid}[e_L_SD_pDIALOG_STYLE] = 255;
	g_LSafeDialogs_player[playerid][e_L_SD_pDIALOG_STRING]{0} = '\0';

	return 1;
}

//Hooked ShowPlayerDialog
stock LSafeDialogs_ShowPlayerDialog(playerid, dialogid, style, caption[],
									 		info[], button1[], button2[]) {

	g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_ID] = dialogid;
	g_LSafeDialogs_Player{playerid}[e_L_SD_pDIALOG_STYLE] = style;

	if(style == DIALOG_STYLE_LIST || style == DIALOG_STYLE_TABLIST ||
		style == DIALOG_STYLE_TABLIST_HEADERS) {

	    for(new i = strlen(info); i != 0; i--) {

	        if(info[i-1] == '\n')
	            g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_LISTITEMS]++;
		}
		if(style != DIALOG_STYLE_TABLIST_HEADERS)
		    g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_LISTITEMS]++;
	}
	strpack((g_LSafeDialogs_player[playerid][e_L_SD_pDIALOG_STRING]{0} = '\0',
	    g_LSafeDialogs_player[playerid][e_L_SD_pDIALOG_STRING]), info,
	    MAX_DIALOG_STRING);
	    
	return ShowPlayerDialog(playerid, dialogid, style, caption,
	                        info, button1, button2);

}


//Other functions
stock GetPlayerDialogID(playerid)
	return g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_ID];
	
stock GetPlayerDialogStyle(playerid)
	return g_LSafeDialogs_Player{playerid}[e_L_SD_pDIALOG_STYLE];

stock GetPlayerDialogInfo(playerid, dest[], size = sizeof(dest)) {

	strunpack(dest,
	    g_LSafeDialogs_player[playerid][e_L_SD_pDIALOG_STRING],
	    size);

	return dest;
}

public OnScriptInit() {

	for(new i = GetPlayerPoolSize(); i != -1; i--) {
	    if(!IsPlayerConnected(i))
	        continue;

		OnPlayerConnect(i);
	}
	#if defined LSafeDialogs_OnScriptInit
	    return LSafeDialogs_OnScriptInit();
	#else
	    return 1;
	#endif
}


public OnPlayerConnect(playerid) {

	LSafeDialogs_ResetVars(playerid);

	#if defined LSafeDialogs_OnPlayerConnect
	    return LSafeDialogs_OnPlayerConnect(playerid);
	#else
	    return 1;
	#endif
}


public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]) {

	if(g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_ID] != dialogid) {

	    LSafeDialogs_ResetVars(playerid);
	    #if defined OnDialogSpoof
			if(!OnDialogSpoof(playerid, DIALOG_SPOOFTYPE_DIALOG_ID))
			    return 1;
		#else
  			return 1;
		#endif
	}
	new
	    inputtextLen = strlen(inputtext);

	if(g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_LISTITEMS] != -1) {

	    if(listitem < 0 ||
   		listitem > g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_LISTITEMS]) {

			LSafeDialogs_ResetVars(playerid);
			#if defined OnDialogSpoof
			    if(!OnDialogSpoof(playerid, DIALOG_SPOOFTYPE_LIST_ITEM))
			        return 1;
			#else
				return 1;
			#endif
		}
		new
		    initPoint = 0,
		    endPoint,
		    tempCounts = (g_LSafeDialogs_Player{playerid}[e_L_SD_pDIALOG_STYLE]
				== DIALOG_STYLE_TABLIST_HEADERS) ? -2 : -1,
			tempDString[MAX_DIALOG_STRING],
			tempDListString[MAX_DIALOG_LISTITEM_LEN],
		    tempDStrLen
		;
		
		strunpack(tempDString,
			g_LSafeDialogs_player[playerid][e_L_SD_pDIALOG_STRING],
		    sizeof(tempDString)
		);
		tempDStrLen = strlen(tempDString);
		for(endPoint = 0; endPoint < tempDStrLen; endPoint++) {
		
		    if(tempDString[endPoint] == '\n') {
		    
		        if(++tempCounts == listitem)
		            break;
                initPoint = endPoint;
			}
		}
		if(initPoint != 0)
		    initPoint++;
		strmid(tempDListString, tempDString, initPoint, endPoint,
		    sizeof(tempDListString));
		    
		if(inputtextLen > 0 && tempDStrLen > 0) {
		
		    if(strcmp(inputtext, tempDListString, false)) {
		    
		        LSafeDialogs_ResetVars(playerid);
		        #if defined OnDialogSpoof
		        	if(!OnDialogSpoof(playerid, DIALOG_SPOOFTYPE_INPUT_TEXT))
		            	return 1;
				#else
				    return 1;
				#endif
			}
		}
		else {
		
		    LSafeDialogs_ResetVars(playerid);
		    #if defined OnDialogSpoof
		        if(!OnDialogSpoof(playerid, DIALOG_SPOOFTYPE_INPUT_TEXT))
		            return 1;
			#else
			    return 1;
			#endif
		}
	}
	else {

		//Filtering inputtext.
	    for(new i = 0; i< inputtextLen; i++) {

	        if(inputtext[i] == '%')
	            inputtext[i] = '#';
		}
	}
	LSafeDialogs_ResetVars(playerid);
	#if defined LSafeDialogs_OnDialogResp
	    return LSafeDialogs_OnDialogResp(playerid, dialogid, response,
	        listitem, inputtext);
	#else
	    return 1;
	#endif
}

//Hooks
#if defined _ALS_OnScriptInit
	#undef OnScriptInit
#else
	#define _ALS_OnScriptInit
#endif

#if defined LSafeDialogs_OnScriptInit
	forward LSafeDialogs_OnScriptInit();
#endif


#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif

#if defined LSafeDialogs_OnPlayerConnect
	forward LSafeDialogs_OnPlayerConnect(playerid);
#endif

#if defined _ALS_OnDialogResponse
	#undef OnDialogResponse
#else
	#define _ALS_OnDialogResponse
#endif

#if defined LSafeDialogs_OnDialogResp
	forward LSafeDialogs_OnDialogResp(playerid, dialogid, response, listitem,
	    inputtext[]);
#endif

#if defined _ALS_ShowPlayerDialog
	#undef ShowPlayerDialog
#else
	#define _ALS_ShowPlayerDialog
#endif

#if defined OnDialogSpoof
	forward OnDialogSpoof(playerid, spooftype);
#endif

#define ShowPlayerDialog 		LSafeDialogs_ShowPlayerDialog
#define OnDialogResponse 		LSafeDialogs_OnDialogResp
#define OnPlayerConnect 		LSafeDialogs_OnPlayerConnect
#define OnScriptInit 			LSafeDialogs_OnScriptInit
