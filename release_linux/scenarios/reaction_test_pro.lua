-- Reaction Test Pro
-- A single target spawns in a wide area and disappears very quickly.
-- Tests reaction time and raw flicking speed.

local radius = 0.3
local lifetime = 0.6 -- Very short!
local spawnTimer = 0
local spawnInterval = 1.0

function spawnNewTarget()
    local x = math.random(-15, 15)
    local y = math.random(1, 10)
    local z = -10
    
    spawnTarget(x, y, z, radius, TargetShape.SPHERE, {1, 0.5, 0}, {0, 0, 0}, lifetime)
    spawnTimer = 0
end

function onStart()
    setIsAuto(false)
    clearTargets()
    spawnTimer = 0
    setAutoShoot(false)
    setTargetsPersistent(false)
    spawnNewTarget()
    setCooldown(0.15)
end

function onUpdate(dt)
    spawnTimer = spawnTimer + dt
    -- If no target is present, or if enough time has passed
    -- (The engine clears them automatically due to lifetime)
    -- Actually, we can check how many targets there are? No, we don't have that in Lua yet.
    -- So we just use a timer.
    if spawnTimer >= spawnInterval then
        spawnNewTarget()
    end
end

function onHit(x, y, z)
    -- Successfully hit! Maybe shorten spawn interval to make it harder?
    spawnInterval = math.max(0.5, spawnInterval - 0.05)
    -- Speed up next spawn
    spawnTimer = spawnInterval
end

function onMiss()
    -- Missed! Reset difficulty?
    spawnInterval = 1.0
end
