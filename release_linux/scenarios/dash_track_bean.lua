-- Dash Track Bean Scenario
-- Target slides on the floor and performs smooth dashes.
-- Adjusted for better balance (less "super speed").

local pos = { x = 0, y = 0.05, z = -12 }
local dir = { x = 1, z = 0 }
local currentSpeed = 0
local targetSpeed = 5.0
local radius = 0.5

local stateTimer = 0
local state = "DASH" -- "DASH" or "WAIT"

function onStart()
    setIsAuto(true)
    setAutoShoot(true)
    setTargetsPersistent(true)
    setFireRate(900)
    resetDash()
end

function resetDash()
    stateTimer = math.random(0.6, 1.2) -- Time to dash
    state = "DASH"
    
    -- Pick a new random direction
    local angle = math.random() * 2 * math.pi
    dir.x = math.cos(angle)
    dir.z = math.sin(angle)
    
    -- Reduced speeds for better balance
    targetSpeed = math.random(8, 14) 
end

function onUpdate(dt)
    stateTimer = stateTimer - dt
    
    if stateTimer <= 0 then
        if state == "DASH" then
            state = "WAIT"
            stateTimer = math.random(0.3, 0.7) -- Longer pause for re-acquisition
            targetSpeed = 0
        else
            resetDash()
        end
    end

    -- Smoothly interpolate speed
    currentSpeed = currentSpeed + (targetSpeed - currentSpeed) * dt * 6.0

    pos.x = pos.x + dir.x * currentSpeed * dt
    pos.z = pos.z + dir.z * currentSpeed * dt

    -- Bounds check
    if math.abs(pos.x) > 12 then
        dir.x = -dir.x
        pos.x = (pos.x > 0 and 12 or -12)
    end
    if pos.z > -5 then
        dir.z = -dir.z
        pos.z = -5
    elseif pos.z < -20 then
        dir.z = -dir.z
        pos.z = -20
    end

    clearTargets()
    spawnTarget(pos.x, pos.y, pos.z, radius, TargetShape.BEAN, {0.2, 0.8, 1.0})
end

function onHit(x, y, z)
end
