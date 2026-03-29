-- Air Angelic Scenario
-- Smooth tracking scenario with a flying target that has complex movement patterns.

local targetPos = { x = 0, y = 5, z = -15 }
local targetVel = { x = 0, y = 0, z = 0 }
local targetAcc = { x = 0, y = 0, z = 0 }
local targetRadius = 0.6
local targetHealth = 10000 -- Very high HP for infinite-feeling tracking
local changeTimer = 0
local dashTimer = 0

-- Boundaries
local minX, maxX = -15, 15
local minY, maxY = 1, 12
local minZ, maxZ = -25, -10

function onStart()
    setPlayerPos(0, 1, 5)
    setIsAuto(true)
    setTargetsPersistent(true)
    clearTargets()
    clearWalls()
    
    -- Spawn the initial target
    spawnTarget(targetPos.x, targetPos.y, targetPos.z, targetRadius, TargetShape.BEAN, {0.8, 0.9, 1.0}, {0,0,0}, 0, targetHealth, true)
    
    -- Spawn boundaries (optional, for visual reference)
    local wallColor = {0.1, 0.1, 0.15}
    -- Floor
    spawnWall(0, 0, -17.5, 40, 0.1, 30, wallColor)
    -- Back Wall
    spawnWall(0, 7.5, -27, 40, 15, 0.1, wallColor)
    -- Side Walls
    spawnWall(-17, 7.5, -17.5, 0.1, 15, 30, wallColor)
    spawnWall(17, 7.5, -17.5, 0.1, 15, 30, wallColor)
    -- Ceiling
    spawnWall(0, 15, -17.5, 40, 0.1, 30, wallColor)

    setFireRate(900) -- High fire rate for tracking
    setAutoShoot(true)
end

function onUpdate(dt)
    changeTimer = changeTimer - dt
    dashTimer = dashTimer - dt

    -- RESET ACCELERATION EVERY FRAME to avoid accumulation bug
    local currentRandomAcc = {x = 0, y = 0, z = 0}

    if changeTimer <= 0 then
        -- Randomly change direction of flow
        targetAcc.x = (math.random() * 2 - 1) * 20
        targetAcc.y = (math.random() * 2 - 1) * 15
        targetAcc.z = (math.random() * 2 - 1) * 10
        changeTimer = 0.8 + math.random() * 1.2
    end

    -- Center Attraction (Tether) - gentle pull back to middle
    local centerX, centerY, centerZ = 0, 6, -15
    local tetherStrength = 5.0
    local finalAcc = {
        x = targetAcc.x + (centerX - targetPos.x) * tetherStrength,
        y = targetAcc.y + (centerY - targetPos.y) * tetherStrength,
        z = targetAcc.z + (centerZ - targetPos.z) * tetherStrength
    }

    if dashTimer <= 0 and math.random() < 0.01 then
        -- Very weak, rare dash
        local dashDirX = (math.random() * 2 - 1)
        local dashDirY = (math.random() * 2 - 1)
        
        -- Dash towards center
        if (targetPos.x < 0 and dashDirX < 0) or (targetPos.x > 0 and dashDirX > 0) then dashDirX = -dashDirX end
        
        targetVel.x = targetVel.x + dashDirX * 10
        targetVel.y = targetVel.y + dashDirY * 8
        dashTimer = 5.0 + math.random() * 3.0
    end

    -- Update velocity
    targetVel.x = targetVel.x + finalAcc.x * dt
    targetVel.y = targetVel.y + finalAcc.y * dt
    targetVel.z = targetVel.z + finalAcc.z * dt

    -- CAP MAX VELOCITY for "Air Angelic" feel
    local maxVel = 12.0
    local speed = math.sqrt(targetVel.x^2 + targetVel.y^2 + targetVel.z^2)
    if speed > maxVel then
        targetVel.x = (targetVel.x / speed) * maxVel
        targetVel.y = (targetVel.y / speed) * maxVel
        targetVel.z = (targetVel.z / speed) * maxVel
    end

    -- Damping/Air resistance
    local damping = 2.0
    targetVel.x = targetVel.x * (1 - damping * dt)
    targetVel.y = targetVel.y * (1 - damping * dt)
    targetVel.z = targetVel.z * (1 - damping * dt)

    -- Update position
    targetPos.x = targetPos.x + targetVel.x * dt
    targetPos.y = targetPos.y + targetVel.y * dt
    targetPos.z = targetPos.z + targetVel.z * dt

    -- Soft Boundaries (No hard bounces unless absolutely necessary)
    if targetPos.x < minX or targetPos.x > maxX then
        targetVel.x = 0 -- Just stop if we hit hard wall
        targetPos.x = math.max(minX, math.min(maxX, targetPos.x))
    end
    if targetPos.y < minY or targetPos.y > maxY then
        targetVel.y = 0
        targetPos.y = math.max(minY, math.min(maxY, targetPos.y))
    end
    if targetPos.z < minZ or targetPos.z > maxZ then
        targetVel.z = 0
        targetPos.z = math.max(minZ, math.min(maxZ, targetPos.z))
    end

    -- Sync position to engine target
    if getTargetCount() == 0 then
        spawnTarget(targetPos.x, targetPos.y, targetPos.z, targetRadius, TargetShape.BEAN, {0.8, 0.9, 1.0}, {0,0,0}, 0, targetHealth, true)
    else
        setTargetPosition(1, targetPos.x, targetPos.y, targetPos.z)
        -- Refresh health occasionally to keep the health bar full
        setTargetHealth(1, targetHealth)
    end
end

function onHit(x, y, z)
    -- This scenario is for pure tracking practice.
end

-- Created for fun! Enjoy the angelic tracking practice.
