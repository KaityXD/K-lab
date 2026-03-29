# Kim-Lol Lua API Documentation

This document describes the Lua API available for creating scenarios in Kim-Lol.

## Table of Contents
- [Lifecycle Callbacks](#lifecycle-callbacks)
- [Target Management](#target-management)
- [World & Environment](#world--environment)
- [Player & Camera](#player--camera)
- [Game State & Statistics](#game-state--statistics)
- [Audio](#audio)
- [Visuals & Effects](#visuals--effects)
- [Input](#input)
- [Random Functions](#random-functions)
- [Constants](#constants)
- [Example Scenario](#example-scenario)

---

## Lifecycle Callbacks

These functions are called by the engine at specific times. Implement them in your Lua script to define behavior.

### `onStart()`
Called when the scenario is first loaded or restarted. Use this to initialize the player position, targets, and game settings.

### `onUpdate(dt)`
Called every frame. `dt` is the time elapsed since the last frame in seconds.

### `onHit(x, y, z)`
Called when the player successfully hits a target. `x, y, z` are the coordinates of the hit point.

### `onMiss()`
Called when the player fires a shot but misses all targets.

---

## Target Management

### `spawnTarget(x, y, z, radius, shape, color, velocity, lifetime, health, showHP, name, scale)`
Spawns a new target at the specified coordinates.
- `x, y, z`: Coordinates (float).
- `radius`: (Optional) Size of the target. Default: `0.5`.
- `shape`: (Optional) `TargetShape` constant. Default: `TargetShape.SPHERE`.
- `color`: (Optional) Table `{r, g, b}`. Default: `{1, 0, 0}` (Red).
- `velocity`: (Optional) Table `{vx, vy, vz}`.
- `lifetime`: (Optional) Time in seconds before the target disappears.
- `health`: (Optional) Hit points. Default: `100`.
- `showHP`: (Optional) Boolean to show/hide the health bar.
- `name`: (Optional) String name for the target.
- `scale`: (Optional) Table `{sx, sy, sz}` for non-uniform scaling.

### `clearTargets()`
Removes all active targets from the scene.

### `getTargetCount()`
Returns the number of currently active targets.

### `getTargets()`
Returns a list of tables representing active targets. Each sub-table contains:
`{ x, y, z, radius, health, maxHealth }`.

### `removeTarget(index)`
Removes the target at the specified 1-based index.

### `setTargetSize(radius)`
Sets the radius for all currently active targets.

### `setTargetColor(index, r, g, b)`
Updates the color of the target at the specified index.

### `setTargetHealth(index, hp)`
Sets the health of the target at the specified index.

### `setTargetHealthBar(index, show)`
Shows or hides the health bar for a specific target.

### `setTargetShape(index, shape)`
Changes the shape of a target.

### `setTargetVelocity(index, vx, vy, vz)`
Sets the movement velocity for a target.

### `getTargetVelocity(index)`
Returns three values: `vx, vy, vz`.

### `setTargetGravity(index, g)`
Sets the gravity scale for a specific target.

### `setTargetScale(index, sx, sy, sz)`
Sets the scale for a target.

### `setTargetInvincible(index, bool)`
If true, the target cannot be destroyed.

---

## World & Environment

### `spawnWall(x, y, z, sx, sy, sz, color, rotation)`
Spawns a static wall/block.
- `x, y, z`: Position.
- `sx, sy, sz`: Size (scale).
- `color`: Table `{r, g, b}`.
- `rotation`: Table `{rx, ry, rz}` (Euler angles in degrees) or a single number for X-axis rotation.

### `clearWalls()`
Removes all walls from the scene.

### `setSkyColor(r, g, b)`
Sets the background/sky color.

### `setFloorColor(r, g, b)`
Sets the floor color.

---

## Player & Camera

### `setPlayerPos(x, y, z)`
Sets the player's position in the world.

### `getCameraPos()`
Returns three values: `x, y, z` representing the current camera position.

### `getCameraFront()`
Returns three values: `x, y, z` representing the forward direction vector of the camera.

---

## Game State & Statistics

### `getTime()`
Returns the remaining time for the scenario in seconds.

### `setTimeOut(seconds)`
Sets the total duration of the scenario.

### `getScore()`, `setScore(score)`
Gets or sets the current player score.

### `getHits()`, `setHits(hits)`
Gets or sets the total number of hits.

### `getMisses()`, `setMisses(misses)`
Gets or sets the total number of misses.

### `getTotalShots()`, `setTotalShots(shots)`
Gets or sets the total shots fired.

### `setAutoShoot(bool)`
Enables or disables automatic firing when a target is under the crosshair.

### `setIsAuto(bool)`
Sets whether the scenario is considered "auto" (tracking style).

### `setFireRate(rpm)`
Sets the weapon's fire rate in rounds per minute.

### `setCooldown(seconds)`
Alternative to `setFireRate`. Sets the time interval between shots.

---

## Audio

### `playSound(path, pitchOffset)`
Plays a sound from the `assets/sounds/` directory.
- `path`: Filename (e.g., "hit.wav").
- `pitchOffset`: (Optional) Pitch shift value.

### `playSoundAt(path, x, y, z, pitchOffset)`
Plays a 3D sound at a specific position.

---

## Visuals & Effects

### `setCrosshair(config)`
Configures the crosshair appearance using a table:
```lua
setCrosshair({
    thickness = 2,
    length = 10,
    gap = 4,
    dot = true,
    dotSize = 2,
    circle = false,
    circleRadius = 15,
    outline = true,
    outlineThickness = 1,
    color = {1, 1, 1, 1} -- {r, g, b, a}
})
```

### `emitParticles(x, y, z, color, count)`
Spawns a burst of particles at the specified location.

---

## Input

### `isKeyDown(keyCode)`
Returns true if the specified key is currently pressed.
(Key codes are typically standard GLFW/ASCII values).

### `isMouseDown(button)`
Returns true if the mouse button is pressed.
- `0`: Left Click
- `1`: Right Click
- `2`: Middle Click

---

## Random Functions

All `math.random` calls are intercepted and replaced with a deterministic seeded version to support consistent replays.

### `setRandomSeed(seed)`, `getRandomSeed()`
Gets or sets the seed for the random number generator.

### `randomFloat(min, max)`
Returns a random float between `min` and `max`.

### `randomInt(min, max)`
Returns a random integer between `min` and `max` (inclusive).

### `randomBool(probability)`
Returns true with the given probability (0.0 to 1.0).

### `randomGaussian(mean, stddev)`
Returns a random number following a Gaussian distribution.

### `randomPoisson(lambda)`
Returns a random integer following a Poisson distribution.

### `randomElement(...)`
Returns one of the arguments passed to it at random.

---

## Constants

### `TargetShape`
- `TargetShape.BOX`
- `TargetShape.SPHERE`
- `TargetShape.BEAN`
- `TargetShape.CONE`

---

## Example Scenario

```lua
scenarioName = "Simple Gridshot"

local targetCount = 3
local z_depth = -10

function onStart()
    setPlayerPos(0, 3, 5)
    setTimeOut(60)
    clearTargets()
    
    for i = 1, targetCount do
        spawnNewTarget()
    end
end

function spawnNewTarget()
    local x = randomFloat(-10, 10)
    local y = randomFloat(1, 7)
    spawnTarget(x, y, z_depth, 0.5, TargetShape.SPHERE, {1, 0, 0})
end

function onUpdate(dt)
    if getTargetCount() < targetCount then
        spawnNewTarget()
    end
end

function onHit(x, y, z)
    playSound("hit.wav")
    emitParticles(x, y, z, {1, 0.5, 0}, 20)
end
```
