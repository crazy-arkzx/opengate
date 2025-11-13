/*
    OpenGate Include
    This include provides an Automatic Gate System
    Where, when approaching the gate,
    it opens without the need to type commands or press buttons.
    
    Creator: Crazy_ArKzX
    Contributors: ddx60hz, HaShira_Caos, PortalSamp (utils)
    
    CreateAutoGate(
    modelid, // Object Model
    Float:closeX, Float:closeY, Float:closeZ, Float:closeRX, Float:closeRY, Float:closeRZ // Closed Gate Position
    Float:openX, Float:openY, Float:openZ, Float:openRX, Float:openRY, Float:openRZ // Open Gate Position
    Float:range = 10.0 // Distance to Open the Gate
    speed = 2 // Gate Opening Speed
    autoclose_time = 5000 // Time to Automatically Close the Gate
    bool:auto_open = true) // Opens Automatically for Anyone (true) or (false) to open only if the player meets a specific condition or performs a certain action
*/

#if defined _open_gate_included
    #endinput
#endif
#define _open_gate_included

#if !defined MAX_GATES
    #define MAX_GATES 1000
#endif
#if !defined GATE_CHECK
    #define GATE_CHECK 1000
#endif

enum E_GATE_DATA {
    bool:g_Exists,
    g_ObjectID,
    Float:g_ClosePos[6],
    Float:g_OpenPos[6],
    Float:g_Range,
    Float:g_RangeSq,
    g_Speed,
    g_AutoCloseTime,
    bool:g_IsOpen,
    g_CloseTimer,
    g_LastCheck,
    bool:g_AutoOpen
}

static g_GateData[MAX_GATES][E_GATE_DATA];
static g_GateCheckTimer = -1;
static g_ActiveGates = 0;

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
    g_GateData[slot][g_ObjectID] = CreateObject(modelid, closeX, closeY, closeZ, closeRX, closeRY, closeRZ);    
    if(g_GateData[slot][g_ObjectID] == INVALID_OBJECT_ID) return -1;
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
    g_GateData[slot][g_RangeSq] = range * range;
    g_GateData[slot][g_Speed] = speed;
    g_GateData[slot][g_AutoCloseTime] = autoclose_time;
    g_GateData[slot][g_IsOpen] = false;
    g_GateData[slot][g_CloseTimer] = -1;
    g_GateData[slot][g_LastCheck] = 0;
    g_GateData[slot][g_AutoOpen] = auto_open;
    g_GateData[slot][g_Exists] = true;    
    g_ActiveGates++;
    if(g_ActiveGates == 1 && g_GateCheckTimer == -1)
    {
        g_GateCheckTimer = SetTimer("AutoGateCheck", GATE_CHECK, true);
    }    
    return slot;
}

stock DestroyAutoGate(gateid)
{
    if(gateid < 0 || gateid >= MAX_GATES || !g_GateData[gateid][g_Exists]) return 0;    
    DestroyObject(g_GateData[gateid][g_ObjectID]);    
    if(g_GateData[gateid][g_CloseTimer] != -1)
    {
        KillTimer(g_GateData[gateid][g_CloseTimer]);
        g_GateData[gateid][g_CloseTimer] = -1;
    }  
    g_GateData[gateid][g_Exists] = false;
    g_ActiveGates--;
    if(g_ActiveGates == 0 && g_GateCheckTimer != -1)
    {
        KillTimer(g_GateCheckTimer);
        g_GateCheckTimer = -1;
    }    
    return 1;
}

stock OpenAutoGate(gateid)
{
    if(gateid < 0 || gateid >= MAX_GATES || !g_GateData[gateid][g_Exists] || g_GateData[gateid][g_IsOpen]) return 0;   
    MoveObject(g_GateData[gateid][g_ObjectID], g_GateData[gateid][g_OpenPos][0], g_GateData[gateid][g_OpenPos][1], g_GateData[gateid][g_OpenPos][2], g_GateData[gateid][g_Speed], g_GateData[gateid][g_OpenPos][3], g_GateData[gateid][g_OpenPos][4], g_GateData[gateid][g_OpenPos][5]);    
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
    if(distSq > g_GateData[gateid][g_RangeSq]) return 0; // Player muito longe
    return OpenAutoGate(gateid);
}

stock CloseAutoGate(gateid)
{
    if(gateid < 0 || gateid >= MAX_GATES || !g_GateData[gateid][g_Exists] || !g_GateData[gateid][g_IsOpen]) return 0;    
    MoveObject(g_GateData[gateid][g_ObjectID], g_GateData[gateid][g_ClosePos][0], g_GateData[gateid][g_ClosePos][1], g_GateData[gateid][g_ClosePos][2], g_GateData[gateid][g_Speed], g_GateData[gateid][g_ClosePos][3], g_GateData[gateid][g_ClosePos][4], g_GateData[gateid][g_ClosePos][5]);    
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

forward AutoGateCheck();
public AutoGateCheck()
{
    if(g_ActiveGates == 0) return 1;    
    new Float:pX, Float:pY, Float:pZ;
    new tick = GetTickCount();
    for(new i = 0; i < MAX_GATES; i++)
    {
        if(!g_GateData[i][g_Exists]) continue;
        if(!g_GateData[i][g_AutoOpen]) continue;        
        new bool:playerNear = false;
        foreach(new playerid : Player)
        {
            if(GetPlayerVirtualWorld(playerid) != 0) continue;         
            GetPlayerPos(playerid, pX, pY, pZ);
            new Float:distSq = GetDistanceSq(pX, pY, pZ, g_GateData[i][g_ClosePos][0], g_GateData[i][g_ClosePos][1], g_GateData[i][g_ClosePos][2]);            
            if(distSq <= g_GateData[i][g_RangeSq])
            {
                playerNear = true;               
                if(!g_GateData[i][g_IsOpen])
                {
                    OpenAutoGate(i);
                }
                break;
            }
        }
        if(!playerNear && g_GateData[i][g_IsOpen] && g_GateData[i][g_CloseTimer] == -1)
        {
            g_GateData[i][g_CloseTimer] = SetTimerEx("AutoGateCloseTimer", g_GateData[i][g_AutoCloseTime], false, "i", i);
        }       
        g_GateData[i][g_LastCheck] = tick;
    }    
    return 1;
}

#if !defined foreach
forward AutoGateCheckFallback();
public AutoGateCheckFallback()
{
    if(g_ActiveGates == 0) return 1;   
    new Float:pX, Float:pY, Float:pZ;
    new maxplayers = GetMaxPlayers();    
    for(new i = 0; i < MAX_GATES; i++)
    {
        if(!g_GateData[i][g_Exists]) continue;
        if(!g_GateData[i][g_AutoOpen]) continue;        
        new bool:playerNear = false;       
        for(new playerid = 0; playerid < maxplayers; playerid++)
        {
            if(!IsPlayerConnected(playerid)) continue;          
            GetPlayerPos(playerid, pX, pY, pZ);         
            new Float:distSq = GetDistanceSq(pX, pY, pZ, g_GateData[i][g_ClosePos][0], g_GateData[i][g_ClosePos][1], g_GateData[i][g_ClosePos][2]);           
            if(distSq <= g_GateData[i][g_RangeSq])
            {
                playerNear = true;                
                if(!g_GateData[i][g_IsOpen])
                {
                    OpenAutoGate(i);
                }
                break;
            }
        }       
        if(!playerNear && g_GateData[i][g_IsOpen] && g_GateData[i][g_CloseTimer] == -1)
        {
            g_GateData[i][g_CloseTimer] = SetTimerEx("AutoGateCloseTimer", g_GateData[i][g_AutoCloseTime], false, "i", i);
        }
    }   
    return 1;
}
#endif

forward AutoGateCloseTimer(gateid);
public AutoGateCloseTimer(gateid)
{
    g_GateData[gateid][g_CloseTimer] = -1;
    CloseAutoGate(gateid);
    return 1;
}

public OnGameModeExit()
{
    if(g_GateCheckTimer != -1)
    {
        KillTimer(g_GateCheckTimer);
        g_GateCheckTimer = -1;
    }    
    for(new i = 0; i < MAX_GATES; i++)
    {
        if(g_GateData[i][g_Exists]) DestroyAutoGate(i);
    }   
    #if defined AutoGate_OnGameModeExit
        return AutoGate_OnGameModeExit();
    #else
        return 1;
    #endif
}
#if defined _ALS_OnGameModeExit
    #undef OnGameModeExit
#else
    #define _ALS_OnGameModeExit
#endif
#define OnGameModeExit AutoGate_OnGameModeExit
#if defined AutoGate_OnGameModeExit
    forward AutoGate_OnGameModeExit();
#endif