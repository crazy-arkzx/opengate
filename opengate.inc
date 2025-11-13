/*
    OpenGate Include
    This include provides an Automatic Gate System
    Where, when approaching the gate,
    it opens without the need to type commands or press buttons.
    
    Creator: Crazy_ArKzX
    Contributors: Tommy
    
    CreateAutoGate(
    modelid, // Object Model
    Float:closeX, Float:closeY, Float:closeZ, Float:closeRX, Float:closeRY, Float:closeRZ // Closed Gate Position
    Float:openX, Float:openY, Float:openZ, Float:openRX, Float:openRY, Float:openRZ // Open Gate Position
    Float:range = 10.0 // Distance to Open the Gate
    speed = 2 // Gate Opening Speed
    autoclose_time = 5000 // Time to Automatically Close the Gate
    bool:auto_open = true) // Opens Automatically for Anyone (true) or (false) to open only if the player meets a specific condition or performs a certain action
*/

#if defined _opengate_included
    #endinput
#endif
#define _opengate_included

#if !defined MAX_GATES
    #define MAX_GATES 1000
#endif

#tryinclude <streamer>

#if !defined _streamer_included
    #error "Streamer Plugin Required!"
#endif

enum E_GATE_DATA {
    bool:g_Exists,
    g_ObjectID,
    g_AreaID,
    Float:g_ClosePos[6],
    Float:g_OpenPos[6],
    Float:g_Range,
    g_Speed,
    g_AutoCloseTime,
    bool:g_IsOpen,
    g_CloseTimer,
    bool:g_AutoOpen
}

static g_GateData[MAX_GATES][E_GATE_DATA];

stock Float:GetDistanceSq(Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2)
{
    new Float:dx = x2 - x1;
    new Float:dy = y2 - y1;
    new Float:dz = z2 - z1;
    return (dx * dx) + (dy * dy) + (dz * dz);
}

stock CreateAutoGate(modelid, Float:closeX, Float:closeY, Float:closeZ, Float:closeRX, Float:closeRY, Float:closeRZ, Float:openX, Float:openY, Float:openZ, Float:openRX, Float:openRY, Float:openRZ, Float:range = 10.0, speed = 2, autoclose_time = 5000, bool:auto_open = true)
{
    new slot = -1;
    for(new i = 0; i < MAX_GATES; i++)
    {
        if(!g_GateData[i][g_Exists])
        {
            slot = i;
            break;
        }
    }
    if(slot == -1) return -1;
    g_GateData[slot][g_ObjectID] = CreateDynamicObject(modelid, closeX, closeY, closeZ, closeRX, closeRY, closeRZ);
    if(g_GateData[slot][g_ObjectID] == INVALID_STREAMER_ID) return -1;
    g_GateData[slot][g_ClosePos][0] = closeX;
    g_GateData[slot][g_ClosePos][1] = closeY;
    g_GateData[slot][g_ClosePos][2] = closeZ;
    g_GateData[slot][g_ClosePos][3] = closeRX;
    g_GateData[slot][g_ClosePos][4] = closeRY;
    g_GateData[slot][g_ClosePos][5] = closeRZ;
    g_GateData[slot][g_OpenPos][0] = openX;
    g_GateData[slot][g_OpenPos][1] = openY;
    g_GateData[slot][g_OpenPos][2] = openZ;
    g_GateData[slot][g_OpenPos][3] = openRX;
    g_GateData[slot][g_OpenPos][4] = openRY;
    g_GateData[slot][g_OpenPos][5] = openRZ;
    g_GateData[slot][g_Range] = range;
    g_GateData[slot][g_Speed] = speed;
    g_GateData[slot][g_AutoCloseTime] = autoclose_time;
    g_GateData[slot][g_IsOpen] = false;
    g_GateData[slot][g_CloseTimer] = -1;
    g_GateData[slot][g_AutoOpen] = auto_open;
    g_GateData[slot][g_Exists] = true;
    g_GateData[slot][g_AreaID] = CreateDynamicCircle(closeX, closeY, range);
    Streamer_SetIntData(STREAMER_TYPE_AREA, g_GateData[slot][g_AreaID], E_STREAMER_EXTRA_ID, slot);
    return slot;
}

stock DestroyAutoGate(gateid)
{
    if(gateid < 0 || gateid >= MAX_GATES || !g_GateData[gateid][g_Exists]) return 0;
    DestroyDynamicObject(g_GateData[gateid][g_ObjectID]);
    DestroyDynamicArea(g_GateData[gateid][g_AreaID]);
    if(g_GateData[gateid][g_CloseTimer] != -1)
    {
        KillTimer(g_GateData[gateid][g_CloseTimer]);
        g_GateData[gateid][g_CloseTimer] = -1;
    }
    g_GateData[gateid][g_Exists] = false;
    return 1;
}

stock OpenAutoGate(gateid)
{
    if(gateid < 0 || gateid >= MAX_GATES || !g_GateData[gateid][g_Exists] || g_GateData[gateid][g_IsOpen]) return 0;
    MoveDynamicObject(g_GateData[gateid][g_ObjectID], g_GateData[gateid][g_OpenPos][0], g_GateData[gateid][g_OpenPos][1], g_GateData[gateid][g_OpenPos][2], g_GateData[gateid][g_Speed], g_GateData[gateid][g_OpenPos][3], g_GateData[gateid][g_OpenPos][4], g_GateData[gateid][g_OpenPos][5]);
    g_GateData[gateid][g_IsOpen] = true;
    if(g_GateData[gateid][g_CloseTimer] != -1)
    {
        KillTimer(g_GateData[gateid][g_CloseTimer]);
    }
    g_GateData[gateid][g_CloseTimer] = SetTimerEx("AutoGateCloseTimer", g_GateData[gateid][g_AutoCloseTime], false, "i", gateid);
    return 1;
}

stock OpenManualGate(gateid, playerid)
{
    if(gateid < 0 || gateid >= MAX_GATES || !g_GateData[gateid][g_Exists]) return 0;
    if(g_GateData[gateid][g_IsOpen]) return 0;
    new Float:pX, Float:pY, Float:pZ;
    GetPlayerPos(playerid, pX, pY, pZ);
    new Float:distSq = GetDistanceSq(pX, pY, pZ, g_GateData[gateid][g_ClosePos][0], g_GateData[gateid][g_ClosePos][1], g_GateData[gateid][g_ClosePos][2]);
    new Float:rangeSq = g_GateData[gateid][g_Range] * g_GateData[gateid][g_Range];   
    if(distSq > rangeSq) return 0;
    return OpenAutoGate(gateid);
}

stock CloseAutoGate(gateid)
{
    if(gateid < 0 || gateid >= MAX_GATES || !g_GateData[gateid][g_Exists] || !g_GateData[gateid][g_IsOpen]) return 0;
    MoveDynamicObject(g_GateData[gateid][g_ObjectID], g_GateData[gateid][g_ClosePos][0], g_GateData[gateid][g_ClosePos][1], g_GateData[gateid][g_ClosePos][2], g_GateData[gateid][g_Speed], g_GateData[gateid][g_ClosePos][3], g_GateData[gateid][g_ClosePos][4], g_GateData[gateid][g_ClosePos][5]);
    g_GateData[gateid][g_IsOpen] = false;
    if(g_GateData[gateid][g_CloseTimer] != -1)
    {
        KillTimer(g_GateData[gateid][g_CloseTimer]);
        g_GateData[gateid][g_CloseTimer] = -1;
    }
    return 1;
}

stock IsAutoGateOpen(gateid)
{
    if(gateid < 0 || gateid >= MAX_GATES || !g_GateData[gateid][g_Exists]) return 0;
    return g_GateData[gateid][g_IsOpen];
}

forward AutoGateCloseTimer(gateid);
public AutoGateCloseTimer(gateid)
{
    g_GateData[gateid][g_CloseTimer] = -1;
    new bool:hasPlayers = false;
    foreach(new playerid : Player)
    {
        if(IsPlayerInDynamicArea(playerid, g_GateData[gateid][g_AreaID]))
        {
            hasPlayers = true;
            break;
        }
    }
    if(!hasPlayers)
    {
        CloseAutoGate(gateid);
    }
    else
    {
        g_GateData[gateid][g_CloseTimer] = SetTimerEx("AutoGateCloseTimer", g_GateData[gateid][g_AutoCloseTime], false, "i", gateid);
    }    
    return 1;
}

public OnPlayerEnterDynamicArea(playerid, areaid)
{
    new gateid = Streamer_GetIntData(STREAMER_TYPE_AREA, areaid, E_STREAMER_EXTRA_ID);    
    if(gateid >= 0 && gateid < MAX_GATES && g_GateData[gateid][g_Exists])
    {
        if(g_GateData[gateid][g_AutoOpen])
        {
            if(!g_GateData[gateid][g_IsOpen])
            {
                OpenAutoGate(gateid);
            }
        }
    }
    #if defined Gate_OnPlayerEnterDynamicArea
        return Gate_OnPlayerEnterDynamicArea(playerid, areaid);
    #else
        return 1;
    #endif
}

public OnPlayerLeaveDynamicArea(playerid, areaid)
{
    new gateid = Streamer_GetIntData(STREAMER_TYPE_AREA, areaid, E_STREAMER_EXTRA_ID);    
    if(gateid >= 0 && gateid < MAX_GATES && g_GateData[gateid][g_Exists])
    {
        new bool:hasOtherPlayers = false;
        foreach(new i : Player)
        {
            if(i != playerid && IsPlayerInDynamicArea(i, areaid))
            {
                hasOtherPlayers = true;
                break;
            }
        }
        if(!hasOtherPlayers && g_GateData[gateid][g_IsOpen] && g_GateData[gateid][g_CloseTimer] == -1)
        {
            g_GateData[gateid][g_CloseTimer] = SetTimerEx("AutoGateCloseTimer", g_GateData[gateid][g_AutoCloseTime], false, "i", gateid);
        }
    }
    #if defined Gate_OnPlayerLeaveDynamicArea
        return Gate_OnPlayerLeaveDynamicArea(playerid, areaid);
    #else
        return 1;
    #endif
}

public OnGameModeExit()
{
    for(new i = 0; i < MAX_GATES; i++)
    {
        if(g_GateData[i][g_Exists]) DestroyAutoGate(i);
    }
    #if defined Gate_OnGameModeExit
        return Gate_OnGameModeExit();
    #else
        return 1;
    #endif
}

#if defined _ALS_OnPlayerEnterDynamicArea
    #undef OnPlayerEnterDynamicArea
#else
    #define _ALS_OnPlayerEnterDynamicArea
#endif
#define OnPlayerEnterDynamicArea Gate_OnPlayerEnterDynamicArea
#if defined Gate_OnPlayerEnterDynamicArea
    forward Gate_OnPlayerEnterDynamicArea(playerid, areaid);
#endif
#if defined _ALS_OnPlayerLeaveDynamicArea
    #undef OnPlayerLeaveDynamicArea
#else
    #define _ALS_OnPlayerLeaveDynamicArea
#endif
#define OnPlayerLeaveDynamicArea Gate_OnPlayerLeaveDynamicArea
#if defined Gate_OnPlayerLeaveDynamicArea
    forward Gate_OnPlayerLeaveDynamicArea(playerid, areaid);
#endif
#if defined _ALS_OnGameModeExit
    #undef OnGameModeExit
#else
    #define _ALS_OnGameModeExit
#endif
#define OnGameModeExit Gate_OnGameModeExit
#if defined Gate_OnGameModeExit
    forward Gate_OnGameModeExit();
#endif