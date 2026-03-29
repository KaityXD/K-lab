-- Wide Flick Pro
-- Targets spawn far apart, requiring large, fast mouse movements.
-- Targets are small to ensure precision at speed.

local targetCount = 2
local currentTargets = 0
local radius = 0.25

function spawnWideTarget()
    -- Choose a side (left or right, top or bottom)
    local x, y
    if math.random() > 0.5 then
        x = math.random(8, 15) -- Right
    else
        x = math.random(-15, -8) -- Left
    end
    
    if math.random() > 0.5 then
        y = math.random(6, 10) -- Top
    else
        y = math.random(1, 4) -- Bottom
    end
    
    local z = -10
    spawnTarget(x, y, z, radius, TargetShape.SPHERE, {0.9, 0.9, 0.1})
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
        spawnWideTarget()
    end
end

function onHit(x, y, z)
    currentTargets = currentTargets - 1
end
