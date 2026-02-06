# OPENGATE

**OpenGate Include** provides an **Automatic Gate System**  
When a player approaches the gate, it opens automatically  
without the need to type commands or press buttons.

my community: https://discord.gg/fqtABsMX6S

**NOTE**: This Include not more a Updates

## Demonstration
🎥 | [Watch on YouTube](https://youtu.be/NexNcZS4BEU?si=jopC68YlkFwErGUg)

# CONTRIBUITORS
> [@TANF12](https://github.com/TanF12)

## Dependencies
- **Streamer Plugin** (required)
- **foreach** or **YSI** (optional, for performance optimization)

## Syntax
```pawn
CreateAutoGate(
    modelid,                    // Object Model 
    Float:closeX, Float:closeY, Float:closeZ,           // Gate Position
    Float:closeRX, Float:closeRY, Float:closeRZ,        // Gate Rotation
    move[],                     // Direction: "left", "right", "up", "down", "forward", "backward"
    Float:movement,             // Distance the Gate Will Move (in meters)
    Float:range = 10.0,         // Detection Range to Trigger Gate Opening
    Float:speed = 2.0,          // Movement Speed
    autoclose_time = 5000,      // Auto-Close Delay (milliseconds)
    bool:auto_open = true       // Auto-open for anyone (true) or require condition (false)
)
```

## Movement Options
* `up` - Move vertically upward
* `down` - Move vertically downward
* `left` - Move to the left relative to gate rotation
* `right` - Move to the right relative to gate rotation
* `forward` - Move forward relative to gate rotation
* `backward` - Move backward relative to gate rotation

# New Functions (1.7.8)
> RefreshGateSynchronization();
> GetSyncedGatesCount(gateid);

## Usage Examples

### Open Gate for Anyone
```pawn
public OnGameModeInit()
{
    CreateAutoGate(980, 1803.3665, -1722.0623, 13.5428, 0.0, 0.0, 0.0, "up", 3.8526, 10.0, 2.0, 5000, true);
    return 1;
}
```

### Open Gate Only for Players Meeting a Condition
```pawn
new myGate;

public OnGameModeInit()
{
    myGate = CreateAutoGate(980, 1803.3665, -1722.0623, 13.5428, 0.0, 0.0, 0.0, "up", 3.8526, 10.0, 2.0, 5000, false);
    return 1;
}

public OnGateRequestAccess(playerid, gateid)
{
    // Example: Only Admin
    if(gateid == myGate)
    {
        if(IsPlayerAdmin(playerid))
            return 1; // Allow access
        return 0; // Deny access
    }
    return 1;
}
```

## Important Functions

### Gate Management
```pawn
DestroyAutoGate(gateID)
// Destroys a gate and frees its resources

OpenAutoGate(gateID)
// Manually opens a gate

CloseAutoGate(gateID)
// Manually closes a gate

OpenManualGate(gateID, playerid)
// Opens a gate for a specific player (checks OnGateRequestAccess)

IsAutoGateOpen(gateID)
// Returns 1 if gate is open, 0 if closed

DestroyAllAutoGates()
// Destroys all gates and reinitializes the system
```

## Callbacks

### OnGateRequestAccess(playerid, gateid)
Called when a player attempts to trigger a gate. Return 1 to allow access, 0 to deny.

```pawn
public OnGateRequestAccess(playerid, gateid)
{
    // Custom access control logic
    if(IsPlayerAdmin(playerid)) return 1;
    return 0;
}
```

### OnGateOpened(gateid)
Called when a gate finishes opening.

```pawn
public OnGateOpened(gateid)
{
    // Custom logic when gate opens
    return 1;
}
```

### OnGateClosed(gateid)
Called when a gate finishes closing.

```pawn
public OnGateClosed(gateid)
{
    // Custom logic when gate closes
    return 1;
}
```


