/*
________________________________________________________________________________


								safeDialogs.inc
								    v1.0.3
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

Functions : (These can be used only under OnDialogResponse & OnDialogSpoof)

GetPlayerDialogID(playerid) - Returns the current dialog ID of player.
GetPlayerDialogStyle(playerid) - Returns the current dialog style of player.
GetPlayerDialogInfo(playerid, dest[], size = sizeof(dest))
	- Stores the dialog info on "dest" array.
GetPlayerDialogItem(playerid, listitem, dest[], bool:filter = false, size = sizeof(dest))
	- Stores the string/data of corresponding listitem.
	- If filter is set to true, listitem text will be filtered
	  (ie : Color embedding will be removed. Text will be displayed like how-
	        clients see in game.)
   - If filter is set to false (default), listitem text will be as per the
   	 script.


//For syntax highlighting:
native GetPlayerDialog(playerid);
native GetPlayerDialogStyle(playerid);
native GetPlayerDialogInfo(playerid, dest[], size = sizeof(dest));
native GetPlayerDialogItem(playerid, listitem, dest[], bool:filter = false, size = sizeof(dest));

Please consider checking the release topic on SA-MP forums for further info,
change-logs or queries.

________________________________________________________________________________    */

#if defined _included_safeDialogs
	#endinput
#endif

#define _included_safeDialogs

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
	g_LSafeDialogs_pDIALOG_STRING[MAX_PLAYERS][MAX_DIALOG_STRING char],
	bool:g_LSafeDialogs_Init = false,
	g_LSafeDialogs_bigString[MAX_DIALOG_STRING] //To use less heap space.
;

//Internal functions
static stock LSafeDialogs_ResetVars(playerid) {

	g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_ID] = -1;
	g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_LISTITEMS] = -1;
	g_LSafeDialogs_Player{playerid}[e_L_SD_pDIALOG_STYLE] = 255;
	g_LSafeDialogs_player[playerid][e_L_SD_pDIALOG_STRING]{0} = '\0';

	return 1;
}

static stock LSafeDialogs_Init() {

	for(new i = GetPlayerPoolSize(); i != -1; i--) {
	    if(!IsPlayerConnected(i))
	        continue;

		LSafeDialogs_ResetVars(i);
	}
	return 1;
}

//Hooked ShowPlayerDialog
stock LSafeDialogs_ShowPlayerDialog(playerid, dialogid, style, caption[],
									 		info[], button1[], button2[]) {

	g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_ID] = dialogid;
	g_LSafeDialogs_Player{playerid}[e_L_SD_pDIALOG_STYLE] = style;

	if(style == DIALOG_STYLE_LIST || style == DIALOG_STYLE_TABLIST ||
		style == DIALOG_STYLE_TABLIST_HEADERS) {

		new
		    i = strlen(info);

		if(info[i-1] == '\n') //Why a new empty line?
		    info[i-1] = '\0';

	    for(; i != 0; i--) {

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

	strunpack((dest[0] = '\0', dest),
	    g_LSafeDialogs_player[playerid][e_L_SD_pDIALOG_STRING],
	    size);

	return dest;
}

stock GetPlayerDialogItem(playerid, listitem, dest[], bool:filter = false,
							size = sizeof(dest)) {

    new
        initPoint,
        endPoint,
        tempCounts,
        dLen
    ;

    tempCounts = (GetPlayerDialogStyle(playerid) == DIALOG_STYLE_TABLIST_HEADERS) ? -2 : -1;

	GetPlayerDialogInfo(playerid, g_LSafeDialogs_bigString,
		sizeof(g_LSafeDialogs_bigString));

    dLen = strlen(g_LSafeDialogs_bigString);

    for(endPoint = 0; endPoint < dLen; endPoint++) {

        if(g_LSafeDialogs_bigString[endPoint] == '\n') {

            if(++tempCounts == listitem)
                break;
            initPoint = endPoint;
        }
    }
    if(initPoint != 0)
        initPoint++;

    strmid(dest, g_LSafeDialogs_bigString, initPoint, endPoint, size);

	if(filter) {

		dLen = strlen(dest);
		for(endPoint = 0; endPoint < dLen; endPoint++) {

		    if(dest[endPoint] == '{') {

		        initPoint = endPoint;
		        endPoint += 7;
		        if(endPoint < dLen) {

		            if(dest[endPoint] == '}') {

		                for(tempCounts = 1; tempCounts < 7; tempCounts++) {

		                    if(!(tolower(dest[initPoint+tempCounts]) >= 97
		                    && tolower(dest[initPoint+tempCounts]) <= 102)
		                    && !(dest[initPoint+tempCounts] >= 48
		                    && dest[initPoint+tempCounts] <= 57))
		                        break;
						}
						if(tempCounts != 7)
						    continue;

						strdel(dest, initPoint, ++endPoint);
						dLen = strlen(dest);
						endPoint -= 8;
					}
				}
			}
		}
	}
    return 1;
}

public OnFilterScriptInit() {

	if(!g_LSafeDialogs_Init) {

	    LSafeDialogs_Init();
	    g_LSafeDialogs_Init = true;
	}
	#if defined LSafeDialogs_OnFSInit
	    return LSafeDialogs_OnFSInit();
	#else
	    return 1;
	#endif
}

public OnGameModeInit() {

	if(!g_LSafeDialogs_Init) {

		LSafeDialogs_Init();
		g_LSafeDialogs_Init = true;
	}
	#if defined LSafeDialogs_OnGMInit
	    return LSafeDialogs_OnGMInit();
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

	    #if defined OnDialogSpoof
			if(!OnDialogSpoof(playerid, DIALOG_SPOOFTYPE_DIALOG_ID)) {
			    LSafeDialogs_ResetVars(playerid);
			    return 1;
			}
		#else
		    LSafeDialogs_ResetVars(playerid);
  			return 1;
		#endif
	}
 	new
	    inputtextLen = strlen(inputtext);

	if(g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_LISTITEMS] != -1) {

	    if(listitem < 0 ||
   		listitem > g_LSafeDialogs_Player[playerid][e_L_SD_pDIALOG_LISTITEMS]) {

			#if defined OnDialogSpoof
			    if(!OnDialogSpoof(playerid, DIALOG_SPOOFTYPE_LIST_ITEM)) {

			        LSafeDialogs_ResetVars(playerid);
			        return 1;
				}
			#else
			    LSafeDialogs_ResetVars(playerid);
				return 1;
			#endif
		}
		new
		    initPoint = 0,
		    endPoint,
		    tempCounts = (g_LSafeDialogs_Player{playerid}[e_L_SD_pDIALOG_STYLE]
				== DIALOG_STYLE_TABLIST_HEADERS) ? -2 : -1,
			tempDListString[MAX_DIALOG_LISTITEM_LEN],
		    tempDStrLen
		;

		strunpack((g_LSafeDialogs_bigString[0]='\0', g_LSafeDialogs_bigString),
			g_LSafeDialogs_player[playerid][e_L_SD_pDIALOG_STRING],
		    sizeof(g_LSafeDialogs_bigString)
		);
		tempDStrLen = strlen(g_LSafeDialogs_bigString);
		for(endPoint = 0; endPoint < tempDStrLen; endPoint++) {

		    if(g_LSafeDialogs_bigString[endPoint] == '\n') {

		        if(++tempCounts == listitem)
		            break;
                initPoint = endPoint;
			}
		}
		if(initPoint != 0)
		    initPoint++;
		strmid(tempDListString, g_LSafeDialogs_bigString, initPoint, endPoint,
		    sizeof(tempDListString));

		tempDStrLen = strlen(tempDListString);

		initPoint = 0;
		//Filtering the list string to avoid false calls.
		for(endPoint = 0; endPoint< tempDStrLen; endPoint++) {

		    if(tempDListString[endPoint] == '{') {

				initPoint = endPoint;
				endPoint += 7;
				if(endPoint < tempDStrLen) {

				    if(tempDListString[endPoint] == '}') {

						for(tempCounts = 1; tempCounts < 7; tempCounts++) {

						    if(!(tolower(tempDListString[initPoint+tempCounts])\
							>= 97
						    && tolower(tempDListString[initPoint+tempCounts])\
							 <= 102)
							&& !(tempDListString[initPoint+tempCounts] >= 48
							&& tempDListString[initPoint+tempCounts] <= 57))
							    break;
						}
						if(tempCounts != 7)
						    continue;

						strdel(tempDListString, initPoint, ++endPoint);
						tempDStrLen = strlen(tempDListString);
						endPoint -= 8;
					}
				}
			}
		}

		if(inputtextLen > 0 && tempDStrLen > 0) {

			tempDStrLen = (g_LSafeDialogs_Player{playerid}[e_L_SD_pDIALOG_STYLE]
			    == DIALOG_STYLE_TABLIST_HEADERS ||
				g_LSafeDialogs_Player{playerid}[e_L_SD_pDIALOG_STYLE]
				== DIALOG_STYLE_TABLIST) ? inputtextLen : cellmax;

		    if(strcmp(inputtext, tempDListString, false, tempDStrLen)) {

		        #if defined OnDialogSpoof
		        	if(!OnDialogSpoof(playerid, DIALOG_SPOOFTYPE_INPUT_TEXT)) {

						LSafeDialogs_ResetVars(playerid);
		            	return 1;
					}
				#else
				    LSafeDialogs_ResetVars(playerid);
				    return 1;
				#endif
			}
		}
		else {

		    #if defined OnDialogSpoof
		        if(!OnDialogSpoof(playerid, DIALOG_SPOOFTYPE_INPUT_TEXT)) {

		            LSafeDialogs_ResetVars(playerid);
		            return 1;
				}
			#else
			    LSafeDialogs_ResetVars(playerid);
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
	#if defined LSafeDialogs_OnDialogResp
	    //inputtextLen is used as return value of further OnDialogResponse.
	    inputtextLen = LSafeDialogs_OnDialogResp(playerid, dialogid, response,
	        listitem, inputtext);
		LSafeDialogs_ResetVars(playerid); //Resetting only at last.
		return inputtextLen;
	#else
	    LSafeDialogs_ResetVars(playerid);
	    return 0;
	#endif
}

//Hooks
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

#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif

#if defined _ALS_OnDialogResponse
	#undef OnDialogResponse
#else
	#define _ALS_OnDialogResponse
#endif

#if defined _ALS_ShowPlayerDialog
	#undef ShowPlayerDialog
#else
	#define _ALS_ShowPlayerDialog
#endif

#define ShowPlayerDialog 		LSafeDialogs_ShowPlayerDialog
#define OnDialogResponse 		LSafeDialogs_OnDialogResp
#define OnPlayerConnect 		LSafeDialogs_OnPlayerConnect
#define OnFilterScriptInit      LSafeDialogs_OnFSInit
#define OnGameModeInit          LSafeDialogs_OnGMInit

#if defined LSafeDialogs_OnFSInit
	forward LSafeDialogs_OnFSInit();
#endif

#if defined LSafeDialogs_OnGMInit
	forward LSafeDialogs_OnGMInit();
#endif

#if defined LSafeDialogs_OnPlayerConnect
	forward LSafeDialogs_OnPlayerConnect(playerid);
#endif

#if defined LSafeDialogs_OnDialogResp
	forward LSafeDialogs_OnDialogResp(playerid, dialogid, response, listitem,
	    inputtext[]);
#endif

#if defined OnDialogSpoof
	forward OnDialogSpoof(playerid, spooftype);
#endif

