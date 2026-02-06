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

#tryinclude <open.mp>

#if !defined _INC_open_mp
	#include <a_samp>
#endif

#if !defined _streamer_included
    #tryinclude <streamer>
    #if !defined _streamer_included
        #error "Streamer Plugin Required! Please install streamer.inc"
    #endif
#endif

#if !defined MAX_GATES
    #define MAX_GATES 1000
#endif

#if !defined GATE_SYNC_DISTANCE
    #define GATE_SYNC_DISTANCE 10.0
#endif

#define GATE_MOVE_UP        0
#define GATE_MOVE_DOWN      1
#define GATE_MOVE_LEFT      2
#define GATE_MOVE_RIGHT     3
#define GATE_MOVE_FORWARD   4
#define GATE_MOVE_BACKWARD  5
#define GATE_ROTATE_X       6
#define GATE_ROTATE_Y       7
#define GATE_ROTATE_Z       8
#define INVALID_GATE_ID     -1

forward OnGateRequestAccess(playerid, gateid);
forward OnGateOpened(gateid);
forward OnGateClosed(gateid);

enum E_GATE_DATA {
    bool:gExists,
    gObjectID,
    gAreaID,
    Float:gClosedPos[6],
    Float:gOpenPos[6],
    Float:gRange,
    Float:gSpeed,
    gAutoCloseTime,
    bool:gIsOpen,
    gTimerID,
    bool:gAutoOpen,
    gPlayersInArea,
    gSyncedGates[MAX_GATES],
    gSyncedCount
}

static GateInfo[MAX_GATES][E_GATE_DATA];
static gateUsedCount = 0;
static OG_TIMER_FUNC[] = "OG_CloseTimer";

stock OG_InitGateSystem() 
{
    for(new i = 0; i < MAX_GATES; i++) 
    {
        GateInfo[i][gExists] = false;
        GateInfo[i][gTimerID] = -1;
        GateInfo[i][gPlayersInArea] = 0;
        GateInfo[i][gSyncedCount] = 0;
        for(new j = 0; j < MAX_GATES; j++)
        {
            GateInfo[i][gSyncedGates][j] = INVALID_GATE_ID;
        }
    }
    gateUsedCount = 0;
}

static stock OG_AllocateGateSlot() 
{
    for(new i = 0; i < MAX_GATES; i++)
    {
        if(!GateInfo[i][gExists])
        {
            gateUsedCount++;
            return i;
        }
    }
    printf("[OpenGate] Error: No free slots available (Max: %d)", MAX_GATES);
    return INVALID_GATE_ID;
}

static stock OG_FreeGateSlot(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES) return 0;
    if(GateInfo[gateID][gExists])
    {
        gateUsedCount--;
    }
    return 1;
}

static stock OG_GetMoveDirection(const move[]) 
{
    if(!strcmp(move, "rotx", true)) return GATE_ROTATE_X;
    if(!strcmp(move, "roty", true)) return GATE_ROTATE_Y;
    if(!strcmp(move, "rotz", true)) return GATE_ROTATE_Z;
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

static stock OG_CalculateOpenPosition(Float:cX, Float:cY, Float:cZ, Float:cRZ, Float:cRX, Float:cRY, dir, Float:mov, &Float:oX, &Float:oY, &Float:oZ, &Float:oRX, &Float:oRY, &Float:oRZ) 
{
    oX = cX; 
    oY = cY; 
    oZ = cZ;   
    oRX = cRX; 
    oRY = cRY; 
    oRZ = cRZ;
    new Float:targetAngle;
    new bool:isMovementXY = false;
    switch(dir)
    {
        case GATE_MOVE_UP: 
        {
            oZ += mov;
        }
        case GATE_MOVE_DOWN: 
        {
            oZ -= mov;
        }
        case GATE_MOVE_FORWARD: 
        {
            targetAngle = cRZ;
            isMovementXY = true;
        }
        case GATE_MOVE_BACKWARD: 
        {
            targetAngle = cRZ + 180.0;
            isMovementXY = true;
        }
        case GATE_MOVE_LEFT: 
        {
            targetAngle = cRZ - 90.0;
            isMovementXY = true;
        }
        case GATE_MOVE_RIGHT: 
        {
            targetAngle = cRZ + 90.0;
            isMovementXY = true;
        }
        case GATE_ROTATE_X: 
        { 
            oRX += mov; 
            oZ += 0.004; 
        }
        case GATE_ROTATE_Y: 
        { 
            oRY += mov; 
            oZ += 0.004; 
        }
        case GATE_ROTATE_Z: 
        { 
            oRZ += mov; 
            oZ += 0.004; 
        }
    }
    if(isMovementXY)
    {
        oX += mov * floatsin(-targetAngle, degrees);
        oY += mov * floatcos(-targetAngle, degrees);
    }
}

static stock OG_CallRequestAccess(playerid, gateid)
{
    if(funcidx("OnGateRequestAccess") != -1) 
    {
        return CallLocalFunction("OnGateRequestAccess", "ii", playerid, gateid);
    }
    return 1;
}

static stock OG_CallOnGateOpened(gateid)
{
    if(funcidx("OnGateOpened") != -1) 
    {
        return CallLocalFunction("OnGateOpened", "i", gateid);
    }
    return 1;
}

static stock OG_CallOnGateClosed(gateid)
{
    if(funcidx("OnGateClosed") != -1) 
    {
        return CallLocalFunction("OnGateClosed", "i", gateid);
    }
    return 1;
}

static bool:IsAnyPlayerInGateArea(gateID)
{
    return (GateInfo[gateID][gPlayersInArea] > 0);
}

static stock OG_DetectNearbyGates(gateID)
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gExists]) return 0;   
    new Float:x1, Float:y1, Float:z1;
    x1 = GateInfo[gateID][gClosedPos][0];
    y1 = GateInfo[gateID][gClosedPos][1];
    z1 = GateInfo[gateID][gClosedPos][2];  
    GateInfo[gateID][gSyncedCount] = 0;    
    for(new i = 0; i < MAX_GATES; i++)
    {
        if(i == gateID || !GateInfo[i][gExists]) continue;       
        new Float:x2, Float:y2, Float:z2;
        x2 = GateInfo[i][gClosedPos][0];
        y2 = GateInfo[i][gClosedPos][1];
        z2 = GateInfo[i][gClosedPos][2];       
        new Float:distance = VectorSize(x1 - x2, y1 - y2, z1 - z2);        
        if(distance <= GATE_SYNC_DISTANCE)
        {
            GateInfo[gateID][gSyncedGates][GateInfo[gateID][gSyncedCount]] = i;
            GateInfo[gateID][gSyncedCount]++;
            new bool:alreadyLinked = false;
            for(new j = 0; j < GateInfo[i][gSyncedCount]; j++)
            {
                if(GateInfo[i][gSyncedGates][j] == gateID)
                {
                    alreadyLinked = true;
                    break;
                }
            }           
            if(!alreadyLinked && GateInfo[i][gSyncedCount] < MAX_GATES)
            {
                GateInfo[i][gSyncedGates][GateInfo[i][gSyncedCount]] = gateID;
                GateInfo[i][gSyncedCount]++;
            }
        }
    }   
    return GateInfo[gateID][gSyncedCount];
}

static stock OG_OpenSyncedGates(gateID, bool:smooth = true)
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gExists]) return 0;    
    for(new i = 0; i < GateInfo[gateID][gSyncedCount]; i++)
    {
        new syncedGate = GateInfo[gateID][gSyncedGates][i];
        if(syncedGate >= 0 && syncedGate < MAX_GATES && GateInfo[syncedGate][gExists])
        {
            if(!GateInfo[syncedGate][gIsOpen])
            {
                OpenAutoGate(syncedGate, smooth);
            }
        }
    }
    return 1;
}

static stock OG_CloseSyncedGates(gateID, bool:smooth = true)
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gExists]) return 0;   
    for(new i = 0; i < GateInfo[gateID][gSyncedCount]; i++)
    {
        new syncedGate = GateInfo[gateID][gSyncedGates][i];
        if(syncedGate >= 0 && syncedGate < MAX_GATES && GateInfo[syncedGate][gExists])
        {
            if(GateInfo[syncedGate][gIsOpen])
            {
                if(!IsAnyPlayerInGateArea(syncedGate))
                {
                    CloseAutoGate(syncedGate, smooth);
                }
            }
        }
    }
    return 1;
}

stock CreateAutoGate(modelid, Float:cX, Float:cY, Float:cZ, Float:cRX, Float:cRY, Float:cRZ, const move[], Float:movement, Float:range = 10.0, Float:speed = 2.0, autoclose_time = 5000, bool:auto_open = true) 
{
    new gateID = OG_AllocateGateSlot();
    if(gateID == INVALID_GATE_ID) return INVALID_GATE_ID;
    GateInfo[gateID][gObjectID] = CreateDynamicObject(modelid, cX, cY, cZ, cRX, cRY, cRZ);
    if(GateInfo[gateID][gObjectID] == INVALID_STREAMER_ID) 
    {
        OG_FreeGateSlot(gateID);
        return INVALID_GATE_ID;
    }
    GateInfo[gateID][gClosedPos][0] = cX;
    GateInfo[gateID][gClosedPos][1] = cY;
    GateInfo[gateID][gClosedPos][2] = cZ;
    GateInfo[gateID][gClosedPos][3] = cRX;
    GateInfo[gateID][gClosedPos][4] = cRY;
    GateInfo[gateID][gClosedPos][5] = cRZ;
    new Float:oX, Float:oY, Float:oZ, Float:oRX, Float:oRY, Float:oRZ;  
    OG_CalculateOpenPosition(cX, cY, cZ, cRZ, cRX, cRY, OG_GetMoveDirection(move), movement, oX, oY, oZ, oRX, oRY, oRZ);    
    GateInfo[gateID][gOpenPos][0] = oX; 
    GateInfo[gateID][gOpenPos][1] = oY; 
    GateInfo[gateID][gOpenPos][2] = oZ;
    GateInfo[gateID][gOpenPos][3] = oRX; 
    GateInfo[gateID][gOpenPos][4] = oRY; 
    GateInfo[gateID][gOpenPos][5] = oRZ;
    GateInfo[gateID][gRange] = range;
    GateInfo[gateID][gSpeed] = speed;
    GateInfo[gateID][gAutoCloseTime] = autoclose_time;
    GateInfo[gateID][gAutoOpen] = auto_open;
    GateInfo[gateID][gIsOpen] = false;
    GateInfo[gateID][gTimerID] = -1;
    GateInfo[gateID][gExists] = true;
    GateInfo[gateID][gPlayersInArea] = 0;
    GateInfo[gateID][gSyncedCount] = 0;
    GateInfo[gateID][gAreaID] = CreateDynamicCircle(cX, cY, range);
    if(GateInfo[gateID][gAreaID] != INVALID_STREAMER_ID) 
    {
        Streamer_SetIntData(STREAMER_TYPE_AREA, GateInfo[gateID][gAreaID], E_STREAMER_EXTRA_ID, gateID);
    }
    OG_DetectNearbyGates(gateID);    
    return gateID;
}

stock DestroyAutoGate(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gExists]) return 0;
    for(new i = 0; i < MAX_GATES; i++)
    {
        if(i == gateID || !GateInfo[i][gExists]) continue;       
        for(new j = 0; j < GateInfo[i][gSyncedCount]; j++)
        {
            if(GateInfo[i][gSyncedGates][j] == gateID)
            {
                for(new k = j; k < GateInfo[i][gSyncedCount] - 1; k++)
                {
                    GateInfo[i][gSyncedGates][k] = GateInfo[i][gSyncedGates][k + 1];
                }
                GateInfo[i][gSyncedCount]--;
                break;
            }
        }
    }  
    if(IsValidDynamicObject(GateInfo[gateID][gObjectID])) DestroyDynamicObject(GateInfo[gateID][gObjectID]); 
    if(IsValidDynamicArea(GateInfo[gateID][gAreaID])) DestroyDynamicArea(GateInfo[gateID][gAreaID]);
    if(GateInfo[gateID][gTimerID] != -1) 
    {
        KillTimer(GateInfo[gateID][gTimerID]);
        GateInfo[gateID][gTimerID] = -1;
    }
    GateInfo[gateID][gExists] = false;
    GateInfo[gateID][gObjectID] = INVALID_STREAMER_ID;
    GateInfo[gateID][gAreaID] = INVALID_STREAMER_ID;
    GateInfo[gateID][gPlayersInArea] = 0;
    GateInfo[gateID][gSyncedCount] = 0;
    OG_FreeGateSlot(gateID);
    return 1;
}

stock OpenAutoGate(gateID, bool:smooth = true) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gExists] || GateInfo[gateID][gIsOpen]) return 0;
    new Float:moveSpeed = GateInfo[gateID][gSpeed]; 
    if(smooth) 
    {
        new Float:currX, Float:currY, Float:currZ;
        GetDynamicObjectPos(GateInfo[gateID][gObjectID], currX, currY, currZ);
        new Float:dist = VectorSize(GateInfo[gateID][gOpenPos][0] - currX, GateInfo[gateID][gOpenPos][1] - currY, GateInfo[gateID][gOpenPos][2] - currZ);
        if(dist < 0.1 && moveSpeed > 0.01)
        {
            moveSpeed = moveSpeed * 0.01;
        }
    }
    MoveDynamicObject(GateInfo[gateID][gObjectID], GateInfo[gateID][gOpenPos][0], GateInfo[gateID][gOpenPos][1], GateInfo[gateID][gOpenPos][2], moveSpeed, GateInfo[gateID][gOpenPos][3], GateInfo[gateID][gOpenPos][4], GateInfo[gateID][gOpenPos][5]);   
    GateInfo[gateID][gIsOpen] = true;  
    if(GateInfo[gateID][gTimerID] != -1) 
    {
        KillTimer(GateInfo[gateID][gTimerID]);
        GateInfo[gateID][gTimerID] = -1;
    }
    OG_OpenSyncedGates(gateID, smooth);    
    OG_CallOnGateOpened(gateID);
    return 1;
}

stock CloseAutoGate(gateID, bool:smooth = true)
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gExists] || !GateInfo[gateID][gIsOpen]) return 0;
    new Float:moveSpeed = GateInfo[gateID][gSpeed];   
    if(smooth) 
    {
        new Float:currX, Float:currY, Float:currZ;
        GetDynamicObjectPos(GateInfo[gateID][gObjectID], currX, currY, currZ);
        new Float:dist = VectorSize(GateInfo[gateID][gClosedPos][0] - currX, GateInfo[gateID][gClosedPos][1] - currY, GateInfo[gateID][gClosedPos][2] - currZ);
        if(dist < 0.1 && moveSpeed > 0.01)
        {
            moveSpeed = moveSpeed * 0.01;
        }
    }
    MoveDynamicObject(GateInfo[gateID][gObjectID], GateInfo[gateID][gClosedPos][0], GateInfo[gateID][gClosedPos][1], GateInfo[gateID][gClosedPos][2], moveSpeed, GateInfo[gateID][gClosedPos][3], GateInfo[gateID][gClosedPos][4], GateInfo[gateID][gClosedPos][5]);
    GateInfo[gateID][gIsOpen] = false;
    if(GateInfo[gateID][gTimerID] != -1) 
    {
        KillTimer(GateInfo[gateID][gTimerID]);
        GateInfo[gateID][gTimerID] = -1;
    }
    OG_CloseSyncedGates(gateID, smooth);    
    OG_CallOnGateClosed(gateID);
    return 1;
}

stock OpenManualGate(gateID, playerid, bool:smooth = true)
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gExists]) return 0;
    if(GateInfo[gateID][gIsOpen]) return 0; 
    if(!IsPlayerInDynamicArea(playerid, GateInfo[gateID][gAreaID])) return 0;
    if(OG_CallRequestAccess(playerid, gateID))
    {
        OpenAutoGate(gateID, smooth);
    }
    return 1;
}

stock IsAutoGateOpen(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gExists]) return 0;
    return GateInfo[gateID][gIsOpen];
}

stock DestroyAllAutoGates() 
{
    for(new i = 0; i < MAX_GATES; i++) 
    {
        if(GateInfo[i][gExists]) DestroyAutoGate(i);
    }    
    OG_InitGateSystem();
    return 1;
}

stock GetSyncedGatesCount(gateID)
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gExists]) return 0;
    return GateInfo[gateID][gSyncedCount];
}

stock RefreshGateSynchronization()
{
    for(new i = 0; i < MAX_GATES; i++)
    {
        if(GateInfo[i][gExists])
        {
            OG_DetectNearbyGates(i);
        }
    }
    return 1;
}

forward OG_CloseTimer(gateID);
public OG_CloseTimer(gateID) 
{
    if(gateID < 0 || gateID >= MAX_GATES || !GateInfo[gateID][gExists]) return 0;    
    GateInfo[gateID][gTimerID] = -1;
    if(IsAnyPlayerInGateArea(gateID)) return 1;
    CloseAutoGate(gateID);
    return 1;
}

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
#if defined OG_OnGameModeInit
    forward OG_OnGameModeInit();
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
#if defined OG_OnGameModeExit
    forward OG_OnGameModeExit();
#endif

public OnPlayerEnterDynamicArea(playerid, areaid) 
{
    new gateID = Streamer_GetIntData(STREAMER_TYPE_AREA, areaid, E_STREAMER_EXTRA_ID);
    if(gateID >= 0 && gateID < MAX_GATES && GateInfo[gateID][gExists] && GateInfo[gateID][gAreaID] == areaid) 
    {
        GateInfo[gateID][gPlayersInArea]++;
        if(GateInfo[gateID][gTimerID] != -1) 
        {
            KillTimer(GateInfo[gateID][gTimerID]);
            GateInfo[gateID][gTimerID] = -1;
        }
        if(GateInfo[gateID][gAutoOpen] && !GateInfo[gateID][gIsOpen]) 
        {
            if(OG_CallRequestAccess(playerid, gateID))
            {
                OpenAutoGate(gateID);
            }
        }
    }     
    #if defined OG_OnPlayerEnterDynamicArea 
        return OG_OnPlayerEnterDynamicArea(playerid, areaid);
    #else 
        return 1;
    #endif
}

#if defined _ALS_OnPlayerEnterDynamicArea
    #undef OnPlayerEnterDynamicArea
#else
    #define _ALS_OnPlayerEnterDynamicArea
#endif

#define OnPlayerEnterDynamicArea OG_OnPlayerEnterDynamicArea
#if defined OG_OnPlayerEnterDynamicArea
    forward OG_OnPlayerEnterDynamicArea(playerid, areaid);
#endif

public OnPlayerLeaveDynamicArea(playerid, areaid) 
{
    new gateID = Streamer_GetIntData(STREAMER_TYPE_AREA, areaid, E_STREAMER_EXTRA_ID);
    if(gateID >= 0 && gateID < MAX_GATES && GateInfo[gateID][gExists] && GateInfo[gateID][gAreaID] == areaid) 
    {
        GateInfo[gateID][gPlayersInArea]--;
        if(GateInfo[gateID][gPlayersInArea] < 0) GateInfo[gateID][gPlayersInArea] = 0;
        if(GateInfo[gateID][gIsOpen]) 
        {
            if(!IsAnyPlayerInGateArea(gateID))
            {
                if(GateInfo[gateID][gTimerID] != -1) KillTimer(GateInfo[gateID][gTimerID]);
                GateInfo[gateID][gTimerID] = SetTimerEx(OG_TIMER_FUNC, GateInfo[gateID][gAutoCloseTime], false, "i", gateID);
            }
        }
    } 
    #if defined OG_OnPlayerLeaveDynamicArea 
        return OG_OnPlayerLeaveDynamicArea(playerid, areaid);
    #else 
        return 1;
    #endif
}

#if defined _ALS_OnPlayerLeaveDynamicArea
    #undef OnPlayerLeaveDynamicArea
#else
    #define _ALS_OnPlayerLeaveDynamicArea
#endif

#define OnPlayerLeaveDynamicArea OG_OnPlayerLeaveDynamicArea
#if defined OG_OnPlayerLeaveDynamicArea
    forward OG_OnPlayerLeaveDynamicArea(playerid, areaid);
#endif