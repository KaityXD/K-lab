scenarioName = "HELL 2"

-- Reactive Tracking Pro
-- A very fast target that changes direction instantly and frequently.

local targetPos = { x = 0, y = 5, z = -10 }
local targetVel = { x = 15, y = 0, z = 0 }
local radius = 0.3
local changeTimer = 0
local changeInterval = 0.4 -- Seconds between potential direction changes

function onStart()
	setIsAuto(true)
	setPlayerPos(0, 0, 5)
	clearTargets()
	targetPos = { x = 0, y = 5, z = -10 }
	targetVel = { x = 15, y = 0, z = 0 }
	setAutoShoot(true)
	setTargetsPersistent(true)
	spawnTarget(targetPos.x, targetPos.y, targetPos.z, radius, TargetShape.SPHERE, { 0.1, 0.9, 0.9 })
	setCooldown(0.1)
end

function onUpdate(dt)
	changeTimer = changeTimer + dt

	if changeTimer >= changeInterval then
		-- Randomly change direction
		local angle = math.random() * 2 * math.pi
		local speed = 12
		targetVel.x = math.cos(angle) * speed
		targetVel.y = math.sin(angle) * speed
		changeTimer = 0
		-- Shorten interval slightly for unpredictability
		changeInterval = 0.2 + math.random() * 0.4
	end

	-- Update position
	targetPos.x = targetPos.x + targetVel.x * dt
	targetPos.y = targetPos.y + targetVel.y * dt

	-- Bounds checking
	if targetPos.x > 15 or targetPos.x < -15 then
		targetVel.x = -targetVel.x
		targetPos.x = math.max(-15, math.min(15, targetPos.x))
	end
	if targetPos.y > 10 or targetPos.y < 1 then
		targetVel.y = -targetVel.y
		targetPos.y = math.max(1, math.min(10, targetPos.y))
	end

	clearTargets()
	spawnTarget(targetPos.x, targetPos.y, targetPos.z, radius, TargetShape.SPHERE, { 0.1, 0.9, 0.9 })
	setTargetInvincible(1, true)
end

function onHit(x, y, z)
	-- Score tracked automatically by engine
end
