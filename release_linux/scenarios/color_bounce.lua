-- Color Bounce Scenario
-- Demonstrates new API features: Color, Velocity, and Lifetime

local targetCount = 5
local area = {min = -10, max = 10, yMin = 1, yMax = 8}

function spawnRandomTarget()
    local x = math.random(area.min, area.max)
    local y = math.random(area.yMin, area.yMax)
    local z = -10
    
    local r = math.random()
    local g = math.random()
    local b = math.random()
    
    local vx = (math.random() - 0.5) * 10
    local vy = (math.random() - 0.5) * 10
    
    -- spawnTarget(x, y, z, radius, shape, colorTable, velocityTable, lifetime)
    spawnTarget(x, y, z, 0.4, TargetShape.SPHERE, {r, g, b}, {vx, vy, 0}, 3.0)
end

function onStart()
    setIsAuto(true)
    clearTargets()
    for i=1, targetCount do
        spawnRandomTarget()
    end
    setCooldown(0.15)
end

function onUpdate(dt)
    -- The engine handles movement and lifetime automatically now!
    -- We just need to refill the targets if they expire or get hit.
    -- (Actually, Scenario::update handles removing expired targets)
end

-- We can use this to refill
function onHit(x, y, z)
    spawnRandomTarget()
end

function onMiss()
    -- Optional penalty or sound
end
