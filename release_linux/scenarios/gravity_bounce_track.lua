-- Gravity Bounce Tracking Scenario
-- A single large target that falls and bounces. 
-- Tracking focused: Target is persistent and requires constant firing.

local targetPos = { x = 0, y = 8, z = -12 }
local targetVel = { x = 6, y = 0, z = 0 }
local radius = 1.2
local gravity = 15.0
local bounceFactor = 0.9

local bounds = { xMin = -15, xMax = 15, yMin = 1.0, yMax = 15 }

function onStart()
    setTimeOut(60)
    setIsAuto(true) -- High KPS tracking mode
    setAutoShoot(false) -- Manual clicking or holding
    setFireRate(900)
    setTargetsPersistent(true)
    clearTargets()
    
    spawnTarget(targetPos.x, targetPos.y, targetPos.z, radius, TargetShape.SPHERE, {0.2, 0.8, 1.0})
end

function onUpdate(dt)
    -- Apply Gravity to Velocity
    targetVel.y = targetVel.y - gravity * dt
    
    -- Apply Velocity to Position
    targetPos.x = targetPos.x + targetVel.x * dt
    targetPos.y = targetPos.y + targetVel.y * dt
    
    -- Floor Bounce
    if targetPos.y - radius < bounds.yMin then
        targetPos.y = bounds.yMin + radius
        targetVel.y = math.abs(targetVel.y) * bounceFactor
        -- Add some randomness to X on bounce
        targetVel.x = targetVel.x + (math.random() - 0.5) * 4
    end
    
    -- Side Wall Bounces
    if targetPos.x + radius > bounds.xMax then
        targetPos.x = bounds.xMax - radius
        targetVel.x = -math.abs(targetVel.x) * bounceFactor
    elseif targetPos.x - radius < bounds.xMin then
        targetPos.x = bounds.xMin + radius
        targetVel.x = math.abs(targetVel.x) * bounceFactor
    end
    
    -- Ceiling "Bounce" or Limit
    if targetPos.y + radius > bounds.yMax then
        targetPos.y = bounds.yMax - radius
        targetVel.y = -math.abs(targetVel.y)
    end

    -- Clamp velocity to prevent it from going crazy
    local maxVel = 20
    if targetVel.x > maxVel then targetVel.x = maxVel end
    if targetVel.x < -maxVel then targetVel.x = -maxVel end

    -- Update the actual target in the engine
    clearTargets()
    spawnTarget(targetPos.x, targetPos.y, targetPos.z, radius, TargetShape.SPHERE, {0.2, 0.8, 1.0}, {0, 0, 0})
end

function onHit(x, y, z)
    -- Standard tracking feedback
end
