-- Precision Grid Pro
-- A 5x5 grid of very small targets.
-- Tests extreme precision and speed.

local targetCount = 5
local currentTargets = 0
local radius = 0.15

function spawnGridTarget()
    -- Grid from -12 to 12 in X, 2 to 10 in Y
    local x = (math.random(0, 4) - 2) * 6
    local y = 2 + math.random(0, 4) * 2
    local z = -10
    
    -- Add a tiny bit of jitter so it's not a perfect static grid
    x = x + (math.random() * 0.4 - 0.2)
    y = y + (math.random() * 0.4 - 0.2)
    
    spawnTarget(x, y, z, radius, TargetShape.SPHERE, {0.1, 0.9, 0.1})
    currentTargets = currentTargets + 1
end

function onStart()
    setIsAuto(false)
    clearTargets()
    currentTargets = 0
    setAutoShoot(false)
    setTargetsPersistent(false)
    setCooldown(0.15)
end

function onUpdate(dt)
    if currentTargets < targetCount then
        spawnGridTarget()
    end
end

function onHit(x, y, z)
    currentTargets = currentTargets - 1
end
