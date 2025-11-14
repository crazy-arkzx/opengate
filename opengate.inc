/*
    OpenGate Include
    Automatic Gate System with Performance Improvements
    
    Creator: Crazy_ArKzX
    
    CreateAutoGate(
        modelid,
        Float:closeX, Float:closeY, Float:closeZ, Float:closeRX, Float:closeRY, Float:closeRZ,
        move[],
        Float:movement,
        Float:range = 10.0,
        speed = 2,
        autoclose_time = 5000,
        bool:auto_open = true
    )
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
#define INVALID_GATE_ID     -1

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
static gateFreeSlots[MAX_GATES];
static gateFreeCount = 0;
static gateUsedCount = 0;
static gateAreaMap[MAX_GATES];

stock OG_InitGateSystem() 
{
    for(new i = 0; i < MAX_GATES; i++) 
    {
        gateFreeSlots[i] = i;
        gateAreaMap[i] = INVALID_GATE_ID;
    }
    gateFreeCount = MAX_GATES;
    gateUsedCount = 0;
}

stock OG_AllocateGateSlot() 
{
    if(gateFreeCount <= 0) return INVALID_GATE_ID;
    gateFreeCount--;
    gateUsedCount++;
    return gateFreeSlots[gateFreeCount];
}

stock OG_FreeGateSlot(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES) return 0;
    gateFreeSlots[gateFreeCount] = gateID;
    gateFreeCount++;
    gateUsedCount--;
    return 1;
}

#if !defined GetDistanceSquared
stock Float:OG_GetDistanceSquared(Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2) 
{
    new Float:dx = x2 - x1;
    new Float:dy = y2 - y1;
    new Float:dz = z2 - z1;
    return (dx * dx) + (dy * dy) + (dz * dz);
}
#endif

stock OG_GetMoveDirection(const move[]) 
{
    switch(move[0]) 
    {
        case 'u', 'U': return GATE_MOVE_UP;
        case 'd', 'D': return GATE_MOVE_DOWN;
        case 'l', 'L': return GATE_MOVE_LEFT;
        case 'r', 'R': return GATE_MOVE_RIGHT;
        case 'f', 'F': return GATE_MOVE_FORWARD;
        case 'b', 'B': return GATE_MOVE_BACKWARD;
    }
    return GATE_MOVE_UP;
}

stock OG_CalculateOpenPosition(Float:closeX, Float:closeY, Float:closeZ, Float:closeRZ, direction, Float:movement, &Float:openX, &Float:openY, &Float:openZ) 
{
    openX = closeX;
    openY = closeY;
    openZ = closeZ;   
    new Float:angle;
    switch(direction)
    {
        case GATE_MOVE_UP: openZ += movement;
        case GATE_MOVE_DOWN: openZ -= movement;
        case GATE_MOVE_LEFT: 
        {
            angle = closeRZ - 90.0;
            openX += movement * floatcos(angle, degrees);
            openY += movement * floatsin(angle, degrees);
        }
        case GATE_MOVE_RIGHT: 
        {
            angle = closeRZ + 90.0;
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
    new gateID = OG_AllocateGateSlot();
    if(gateID == INVALID_GATE_ID) return INVALID_GATE_ID;    
    GateInfo[gateID][gateObjectID] = CreateDynamicObject(modelid, closeX, closeY, closeZ, closeRX, closeRY, closeRZ);
    if(GateInfo[gateID][gateObjectID] == INVALID_STREAMER_ID) 
    {
        OG_FreeGateSlot(gateID);
        return INVALID_GATE_ID;
    }
    GateInfo[gateID][gateClosedPos][0] = closeX;
    GateInfo[gateID][gateClosedPos][1] = closeY;
    GateInfo[gateID][gateClosedPos][2] = closeZ;
    GateInfo[gateID][gateClosedPos][3] = closeRX;
    GateInfo[gateID][gateClosedPos][4] = closeRY;
    GateInfo[gateID][gateClosedPos][5] = closeRZ;
    new Float:openX, Float:openY, Float:openZ;
    new direction = OG_GetMoveDirection(move);
    OG_CalculateOpenPosition(closeX, closeY, closeZ, closeRZ, direction, movement, openX, openY, openZ);    
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
    if(GateInfo[gateID][gateAreaID] < MAX_GATES) 
    {
        gateAreaMap[GateInfo[gateID][gateAreaID]] = gateID;
    }    
    return gateID;
}

stock DestroyAutoGate(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gateExists]) return 0;
    new areaID = GateInfo[gateID][gateAreaID];
    if(areaID >= 0 && areaID < MAX_GATES) 
    {
        gateAreaMap[areaID] = INVALID_GATE_ID;
    }
    if(GateInfo[gateID][gateObjectID] != INVALID_STREAMER_ID) 
    {
        DestroyDynamicObject(GateInfo[gateID][gateObjectID]);
    }
    if(areaID != INVALID_STREAMER_ID) 
    {
        DestroyDynamicArea(areaID);
    }
    if(GateInfo[gateID][gateTimerID] != -1) 
    {
        KillTimer(GateInfo[gateID][gateTimerID]);
        GateInfo[gateID][gateTimerID] = -1;
    }
    GateInfo[gateID][gateExists] = false;
    GateInfo[gateID][gateObjectID] = INVALID_STREAMER_ID;
    GateInfo[gateID][gateAreaID] = INVALID_STREAMER_ID;
    GateInfo[gateID][gateIsOpen] = false;
    GateInfo[gateID][gateAutoOpen] = false;
    OG_FreeGateSlot(gateID);
    return 1;
}

stock OpenAutoGate(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gateExists] || GateInfo[gateID][gateIsOpen]) return 0;    
    MoveDynamicObject(GateInfo[gateID][gateObjectID],GateInfo[gateID][gateOpenPos][0], GateInfo[gateID][gateOpenPos][1], GateInfo[gateID][gateOpenPos][2],GateInfo[gateID][gateSpeed],GateInfo[gateID][gateOpenPos][3], GateInfo[gateID][gateOpenPos][4], GateInfo[gateID][gateOpenPos][5]);  
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
    #if defined GetDistanceSquared
        new Float:distSq = GetDistanceSquared(pX, pY, pZ, GateInfo[gateID][gateClosedPos][0], GateInfo[gateID][gateClosedPos][1], GateInfo[gateID][gateClosedPos][2]);
    #else
        new Float:distSq = OG_GetDistanceSquared(pX, pY, pZ, GateInfo[gateID][gateClosedPos][0], GateInfo[gateID][gateClosedPos][1], GateInfo[gateID][gateClosedPos][2]);
    #endif
    new Float:rangeSq = GateInfo[gateID][gateRange] * GateInfo[gateID][gateRange];    
    if(distSq > rangeSq) return 0;
    return OpenAutoGate(gateID);
}

stock CloseAutoGate(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gateExists] || !GateInfo[gateID][gateIsOpen]) return 0;    
    MoveDynamicObject(GateInfo[gateID][gateObjectID],GateInfo[gateID][gateClosedPos][0], GateInfo[gateID][gateClosedPos][1], GateInfo[gateID][gateClosedPos][2],GateInfo[gateID][gateSpeed],GateInfo[gateID][gateClosedPos][3], GateInfo[gateID][gateClosedPos][4], GateInfo[gateID][gateClosedPos][5]);    
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

stock GetGateCount() 
{
    return gateUsedCount;
}

stock DestroyAllAutoGates() 
{
    for(new i = 0; i < MAX_GATES; i++) 
    {
        if(GateInfo[i][gateExists]) 
        {
            DestroyAutoGate(i);
        }
    }    
    OG_InitGateSystem();
    return 1;
}

forward AutoGateCloseTimer(gateID);
public AutoGateCloseTimer(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gateExists]) return 0;    
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

#if defined OG_OnGameModeInit
    forward OG_OnGameModeInit();
#endif
public OnGameModeInit()
{
    OG_InitGateSystem();
    #if defined OG_OnGameModeInit
        return OG_OnGameModeInit();
    #else
        return 1;
    #endif
}
#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif
#define OnGameModeInit OG_OnGameModeInit
#if defined OG_OnGameModeExit
    forward OG_OnGameModeExit();
#endif
public OnGameModeExit()
{
    DestroyAllAutoGates();
    #if defined OG_OnGameModeExit
        return OG_OnGameModeExit();
    #else
        return 1;
    #endif
}
#if defined _ALS_OnGameModeExit
    #undef OnGameModeExit
#else
    #define _ALS_OnGameModeExit
#endif
#define OnGameModeExit OG_OnGameModeExit
#if defined OG_OnPlrEnterDynArea
    forward OG_OnPlrEnterDynArea(playerid, areaid);
#endif
public OnPlayerEnterDynamicArea(playerid, areaid) 
{
    new gateID = INVALID_GATE_ID;
    if(areaid >= 0 && areaid < MAX_GATES) 
    {
        gateID = gateAreaMap[areaid];
    }    
    if(gateID != INVALID_GATE_ID && GateInfo[gateID][gateExists]) 
    {
        if(GateInfo[gateID][gateAutoOpen] && !GateInfo[gateID][gateIsOpen]) 
        {
            OpenAutoGate(gateID);
        }
    }    
    #if defined OG_OnPlrEnterDynArea
        return OG_OnPlrEnterDynArea(playerid, areaid);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnPlayerEnterDynamicArea
    #undef OnPlayerEnterDynamicArea
#else
    #define _ALS_OnPlayerEnterDynamicArea
#endif
#define OnPlayerEnterDynamicArea OG_OnPlrEnterDynArea
#if defined OG_OnPlrLeaveDynArea
    forward OG_OnPlrLeaveDynArea(playerid, areaid);
#endif
public OnPlayerLeaveDynamicArea(playerid, areaid) 
{
    new gateID = INVALID_GATE_ID;
    if(areaid >= 0 && areaid < MAX_GATES) 
    {
        gateID = gateAreaMap[areaid];
    }    
    if(gateID != INVALID_GATE_ID && GateInfo[gateID][gateExists]) 
    {
        if(GateInfo[gateID][gateIsOpen] && GateInfo[gateID][gateTimerID] == -1) 
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
            if(!otherPlayerInArea) 
            {
                GateInfo[gateID][gateTimerID] = SetTimerEx("AutoGateCloseTimer", GateInfo[gateID][gateAutoCloseTime], false, "i", gateID);
            }
        }
    }   
    #if defined OG_OnPlrLeaveDynArea
        return OG_OnPlrLeaveDynArea(playerid, areaid);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnPlayerLeaveDynamicArea
    #undef OnPlayerLeaveDynamicArea
#else
    #define _ALS_OnPlayerLeaveDynamicArea
#endif
#define OnPlayerLeaveDynamicArea OG_OnPlrLeaveDynArea