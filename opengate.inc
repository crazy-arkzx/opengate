/*
    OpenGate Include
    Automatic Gate System
    Where, When Approaching the Gate, it Open Automatically 
    
    Creators: Crazy_ArKzX & Tommy
    
    CreateAutoGate(
        modelid, // Object Model 
        Float:closeX, Float:closeY, Float:closeZ, Float:closeRX, Float:closeRY, Float:closeRZ, // Gate Position
        move[], // Direction of Movement: "left", "right", "up", "down", "forward", "backward"
        Float:movement, // How Many Meters Will the Gate Move 
        Float:range = 10.0, // Distance to Open the Gate 
        speed = 2, // Opening Speed
        autoclose_time = 5000, // Time to Auto Close
        bool:auto_open = true // Automatically Open to Anyone (true) or (false) Only With Condition 
    )
    
    USE DestroyAllAutoGates(); in OnGameModeExit!!! 
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

#define GATE_MOVE_UP        0
#define GATE_MOVE_DOWN      1
#define GATE_MOVE_LEFT      2
#define GATE_MOVE_RIGHT     3
#define GATE_MOVE_FORWARD   4
#define GATE_MOVE_BACKWARD  5

enum GateData {
    bool:gateExists,
    gateObjectID,
    gateAreaID,
    Float:gateClosedPos[6],
    Float:gateOpenPos[6],
    Float:gateRange,
    gateSpeed,
    gateAutoCloseTime,
    bool:gateIsOpen,
    gateTimerID,
    bool:gateAutoOpen
}

static GateInfo[MAX_GATES][GateData];

stock Float:GetDistanceSquared(Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2) 
{
    new Float:dx = x2 - x1;
    new Float:dy = y2 - y1;
    new Float:dz = z2 - z1;
    return (dx * dx) + (dy * dy) + (dz * dz);
}

stock GetMoveDirection(const move[]) 
{
    if(!strcmp(move, "up", true)) return GATE_MOVE_UP;
    if(!strcmp(move, "down", true)) return GATE_MOVE_DOWN;
    if(!strcmp(move, "left", true)) return GATE_MOVE_LEFT;
    if(!strcmp(move, "right", true)) return GATE_MOVE_RIGHT;
    if(!strcmp(move, "forward", true)) return GATE_MOVE_FORWARD;
    if(!strcmp(move, "backward", true)) return GATE_MOVE_BACKWARD;
    return GATE_MOVE_UP;
}

stock CalculateOpenPosition(Float:closeX, Float:closeY, Float:closeZ, Float:closeRZ, direction, Float:movement, &Float:openX, &Float:openY, &Float:openZ) 
{
    openX = closeX;
    openY = closeY;
    openZ = closeZ;   
    switch(direction) 
    {
        case GATE_MOVE_UP: 
        {
            openZ += movement;
        }
        case GATE_MOVE_DOWN: 
        {
            openZ -= movement;
        }
        case GATE_MOVE_LEFT: 
        {
            new Float:angle = closeRZ - 90.0;
            openX += movement * floatcos(angle, degrees);
            openY += movement * floatsin(angle, degrees);
        }
        case GATE_MOVE_RIGHT: 
        {
            new Float:angle = closeRZ + 90.0;
            openX += movement * floatcos(angle, degrees);
            openY += movement * floatsin(angle, degrees);
        }
        case GATE_MOVE_FORWARD: 
        {
            openX += movement * floatcos(closeRZ, degrees);
            openY += movement * floatsin(closeRZ, degrees);
        }
        case GATE_MOVE_BACKWARD: 
        {
            openX -= movement * floatcos(closeRZ, degrees);
            openY -= movement * floatsin(closeRZ, degrees);
        }
    }
}

stock CreateAutoGate(modelid, Float:closeX, Float:closeY, Float:closeZ, Float:closeRX, Float:closeRY, Float:closeRZ, const move[], Float:movement, Float:range = 10.0, speed = 2, autoclose_time = 5000, bool:auto_open = true) 
{
    new gateID = -1;
    for(new i = 0; i < MAX_GATES; i++) 
    {
        if(!GateInfo[i][gateExists]) 
        {
            gateID = i;
            break;
        }
    }    
    if(gateID == -1) return -1;
    GateInfo[gateID][gateObjectID] = CreateDynamicObject(modelid, closeX, closeY, closeZ, closeRX, closeRY, closeRZ);
    if(GateInfo[gateID][gateObjectID] == INVALID_STREAMER_ID) return -1;
    GateInfo[gateID][gateClosedPos][0] = closeX;
    GateInfo[gateID][gateClosedPos][1] = closeY;
    GateInfo[gateID][gateClosedPos][2] = closeZ;
    GateInfo[gateID][gateClosedPos][3] = closeRX;
    GateInfo[gateID][gateClosedPos][4] = closeRY;
    GateInfo[gateID][gateClosedPos][5] = closeRZ;
    new Float:openX, Float:openY, Float:openZ;
    new direction = GetMoveDirection(move);
    CalculateOpenPosition(closeX, closeY, closeZ, closeRZ, direction, movement, openX, openY, openZ);    
    GateInfo[gateID][gateOpenPos][0] = openX;
    GateInfo[gateID][gateOpenPos][1] = openY;
    GateInfo[gateID][gateOpenPos][2] = openZ;
    GateInfo[gateID][gateOpenPos][3] = closeRX;
    GateInfo[gateID][gateOpenPos][4] = closeRY;
    GateInfo[gateID][gateOpenPos][5] = closeRZ;
    GateInfo[gateID][gateRange] = range;
    GateInfo[gateID][gateSpeed] = speed;
    GateInfo[gateID][gateAutoCloseTime] = autoclose_time;
    GateInfo[gateID][gateIsOpen] = false;
    GateInfo[gateID][gateTimerID] = -1;
    GateInfo[gateID][gateAutoOpen] = auto_open;
    GateInfo[gateID][gateExists] = true;
    GateInfo[gateID][gateAreaID] = CreateDynamicCircle(closeX, closeY, range);
    Streamer_SetIntData(STREAMER_TYPE_AREA, GateInfo[gateID][gateAreaID], E_STREAMER_EXTRA_ID, gateID);   
    return gateID;
}

stock DestroyAutoGate(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gateExists]) return 0;   
    DestroyDynamicObject(GateInfo[gateID][gateObjectID]);
    DestroyDynamicArea(GateInfo[gateID][gateAreaID]);   
    if(GateInfo[gateID][gateTimerID] != -1) 
    {
        KillTimer(GateInfo[gateID][gateTimerID]);
        GateInfo[gateID][gateTimerID] = -1;
    }   
    GateInfo[gateID][gateExists] = false;
    return 1;
}

stock OpenAutoGate(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gateExists] || GateInfo[gateID][gateIsOpen]) return 0;  
    MoveDynamicObject(GateInfo[gateID][gateObjectID],GateInfo[gateID][gateOpenPos][0],GateInfo[gateID][gateOpenPos][1],GateInfo[gateID][gateOpenPos][2],GateInfo[gateID][gateSpeed],GateInfo[gateID][gateOpenPos][3],GateInfo[gateID][gateOpenPos][4],GateInfo[gateID][gateOpenPos][5]);    
    GateInfo[gateID][gateIsOpen] = true;   
    if(GateInfo[gateID][gateTimerID] != -1) 
    {
        KillTimer(GateInfo[gateID][gateTimerID]);
    }    
    GateInfo[gateID][gateTimerID] = SetTimerEx("AutoGateCloseTimer", GateInfo[gateID][gateAutoCloseTime], false, "i", gateID);
    return 1;
}

stock OpenManualGate(gateID, playerid) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gateExists]) return 0;
    if(GateInfo[gateID][gateIsOpen]) return 0;    
    new Float:pX, Float:pY, Float:pZ;
    GetPlayerPos(playerid, pX, pY, pZ);   
    new Float:distSq = GetDistanceSquared(pX, pY, pZ, GateInfo[gateID][gateClosedPos][0], GateInfo[gateID][gateClosedPos][1], GateInfo[gateID][gateClosedPos][2]);
    new Float:rangeSq = GateInfo[gateID][gateRange] * GateInfo[gateID][gateRange];    
    if(distSq > rangeSq) return 0;   
    return OpenAutoGate(gateID);
}

stock CloseAutoGate(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gateExists] || !GateInfo[gateID][gateIsOpen]) return 0;    
    MoveDynamicObject(GateInfo[gateID][gateObjectID],GateInfo[gateID][gateClosedPos][0],GateInfo[gateID][gateClosedPos][1],GateInfo[gateID][gateClosedPos][2],GateInfo[gateID][gateSpeed],GateInfo[gateID][gateClosedPos][3],GateInfo[gateID][gateClosedPos][4],GateInfo[gateID][gateClosedPos][5]);    
    GateInfo[gateID][gateIsOpen] = false;    
    if(GateInfo[gateID][gateTimerID] != -1) 
    {
        KillTimer(GateInfo[gateID][gateTimerID]);
        GateInfo[gateID][gateTimerID] = -1;
    }    
    return 1;
}

stock IsAutoGateOpen(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gateExists]) return 0;
    return GateInfo[gateID][gateIsOpen];
}

forward AutoGateCloseTimer(gateID);
public AutoGateCloseTimer(gateID) 
{
    GateInfo[gateID][gateTimerID] = -1;    
    new bool:playerInArea = false;
    foreach(new i : Player) 
    {
        if(IsPlayerInDynamicArea(i, GateInfo[gateID][gateAreaID])) 
        {
            playerInArea = true;
            break;
        }
    }   
    if(!playerInArea) 
    {
        CloseAutoGate(gateID);
    } else {
        GateInfo[gateID][gateTimerID] = SetTimerEx("AutoGateCloseTimer", GateInfo[gateID][gateAutoCloseTime], false, "i", gateID);
    }    
    return 1;
}

#if defined _ALS_OnPlayerEnterDynamicArea
    #undef OnPlayerEnterDynamicArea
#else
    #define _ALS_OnPlayerEnterDynamicArea
#endif
#define OnPlayerEnterDynamicArea OG_OnPlrEnterDynArea
#if defined OG_OnPlrEnterDynArea
    forward OG_OnPlrEnterDynArea(playerid, areaid);
#endif
#if defined _ALS_OnPlayerLeaveDynamicArea
    #undef OnPlayerLeaveDynamicArea
#else
    #define _ALS_OnPlayerLeaveDynamicArea
#endif
#define OnPlayerLeaveDynamicArea OG_OnPlrLeaveDynArea
#if defined OG_OnPlrLeaveDynArea
    forward OG_OnPlrLeaveDynArea(playerid, areaid);
#endif

public OnPlayerEnterDynamicArea(playerid, areaid) 
{
    new gateID = Streamer_GetIntData(STREAMER_TYPE_AREA, areaid, E_STREAMER_EXTRA_ID);    
    if(gateID >= 0 && gateID < MAX_GATES && GateInfo[gateID][gateExists]) 
    {
        if(GateInfo[gateID][gateAutoOpen]) 
        {
            if(!GateInfo[gateID][gateIsOpen]) 
            {
                OpenAutoGate(gateID);
            }
        }
    }    
    #if defined OG_OnPlrEnterDynArea 
        return OG_OnPlrEnterDynArea(playerid, areaid);
    #else
        return 1;
    #endif
}

public OnPlayerLeaveDynamicArea(playerid, areaid) 
{
    new gateID = Streamer_GetIntData(STREAMER_TYPE_AREA, areaid, E_STREAMER_EXTRA_ID);    
    if(gateID >= 0 && gateID < MAX_GATES && GateInfo[gateID][gateExists]) 
    {
        new bool:otherPlayerInArea = false;        
        foreach(new i : Player) 
        {
            if(i != playerid && IsPlayerInDynamicArea(i, areaid)) 
            {
                otherPlayerInArea = true;
                break;
            }
        }        
        if(!otherPlayerInArea && GateInfo[gateID][gateIsOpen] && GateInfo[gateID][gateTimerID] == -1) 
        {
            GateInfo[gateID][gateTimerID] = SetTimerEx("AutoGateCloseTimer", GateInfo[gateID][gateAutoCloseTime], false, "i", gateID);
        }
    }    
    #if defined OG_OnPlrLeaveDynArea
        return OG_OnPlrLeaveDynArea(playerid, areaid);
    #else
        return 1;
    #endif
}

stock DestroyAllAutoGates() 
{
    for(new i = 0; i < MAX_GATES; i++) 
    {
        if(GateInfo[i][gateExists]) DestroyAutoGate(i);
    }
    return 1;
}