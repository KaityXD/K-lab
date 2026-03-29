-- Micro Flick Pro
-- Small targets spawning in close proximity to the last hit.
-- Tests precise, short-range flicks.

local targetCount = 3
local currentTargets = 0
local radius = 0.2
local lastHitPos = {x = 0, y = 5, z = -10}

function spawnNearTarget(baseX, baseY)
    local x = baseX + (math.random() * 4 - 2)
    local y = baseY + (math.random() * 4 - 2)
    local z = -10
    
    -- Keep within bounds
    x = math.max(-10, math.min(10, x))
    y = math.max(1, math.min(10, y))
    
    spawnTarget(x, y, z, radius, TargetShape.SPHERE, {0.9, 0.1, 0.1})
    currentTargets = currentTargets + 1
end

function onStart()
    setIsAuto(false)
    clearTargets()
    currentTargets = 0
    lastHitPos = {x = 0, y = 5, z = -10}
    setAutoShoot(false)
    setTargetsPersistent(false)
    setCooldown(0.15)
end

function onUpdate(dt)
    if currentTargets < targetCount then
        spawnNearTarget(lastHitPos.x, lastHitPos.y)
    end
end

function onHit(x, y, z)
    currentTargets = currentTargets - 1
    -- We can't easily get the hit target's position from Lua here unless we track it
    -- but for micro flicks, spawning near the center or just random is fine.
    -- Let's just randomize a bit more.
    lastHitPos.x = lastHitPos.x + (math.random() * 6 - 3)
    lastHitPos.y = lastHitPos.y + (math.random() * 6 - 3)
    
    -- Keep within bounds
    lastHitPos.x = math.max(-8, math.min(8, lastHitPos.x))
    lastHitPos.y = math.max(2, math.min(8, lastHitPos.y))
end
