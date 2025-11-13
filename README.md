# OPENGATE

**OpenGate Include** provides an **Automatic Gate System**  
When a player approaches the gate, it opens automatically  
without the need to type commands or press buttons.

## Demonstration

🎥 | [Watch on YouTube](https://youtu.be/NexNcZS4BEU?si=jopC68YlkFwErGUg) // Trash Quality 😡

## Syntax

```pawn
CreateAutoGate(
    modelid,                         // Object model
    Float:closeX, Float:closeY, Float:closeZ, Float:closeRX, Float:closeRY, Float:closeRZ,  // Closed gate position
    Float:openX, Float:openY, Float:openZ, Float:openRX, Float:openRY, Float:openRZ,        // Open gate position
    Float:range = 10.0,              // Distance to automatically open the gate
    speed = 2,                        // Gate opening speed
    autoclose_time = 5000,            // Time in ms to automatically close the gate
    bool:auto_open = true             // true: opens automatically for anyone
                                      // false: opens only if the player meets a specific condition or performs a certain action
);
```

# Usage Examples

Open Gate for Anyone
```pawn
public OnGameModeInit()
{
    CreateAutoGate(
        980, 
        1803.3665, -1722.0623, 13.5428, 0.0, 0.0, 0.0,   // Closed position
        1803.3665, -1722.0623, 17.3954, 0.0, 0.0, 0.0,  // Open position
        5.0, 2, 5000, true                               // Range, speed, auto-close time, auto-open
    );
    return 1;
}
```

 Open Gate Only for Players Meeting a Condition
```pawn
new myGate;

public OnGameModeInit()
{
    myGate = CreateAutoGate(
        980, 
        1803.3665, -1722.0623, 13.5428, 0.0, 0.0, 0.0,   // Closed position
        1803.3665, -1722.0623, 17.3954, 0.0, 0.0, 0.0,  // Open position
        5.0, 2, 5000, false                              // Range, speed, auto-close time, auto-open disabled
    );
    return 1;
}

public OnPlayerUpdate(playerid)
{
    // Example condition: PlayerInfo[playerid][pEmprego] == 1
    if(PlayerInfo[playerid][pEmprego] == 1)
    {
        OpenManualGate(myGate, playerid);
    }
    return 1;
}
```
--------------------------------------------------------------------------------------------------------------

Features:
Automatic gate opening/closing
Supports custom open/close positions
Configurable opening range, speed, and auto-close time
Can restrict opening to specific players or condition
Fully compatible with SAMP scripting

--------------------------------------------------------------------------------------------------------------

Creator: Crazy_ArKzX

Contributors: Tommy, ddx60hz, HaShira_Caos, PortalSamp (utils)

--------------------------------------------------------------------------------------------------------------


