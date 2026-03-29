scenarioName = "Frogtagon WIP"

-- Frogtagon Scenario (Movement-based training)
-- A target that "jumps" in an unpredictable octagon pattern

local targetPos = { x = 0, y = 5, z = -10 }
local targetVel = { x = 0, y = 0, z = 0 }
local radius = 0.5

local jumpTimer = 0
local jumpInterval = 0.7 -- Seconds between each jump
local jumpSpeed = 18

function onStart()
	setPlayerPos(0, 1, 5.0)
	setIsAuto(true)
	setTimeOut(30)
	clearTargets()
	targetPos = { x = 0, y = 5, z = -10 }
	targetVel = { x = 0, y = 0, z = 0 }
	spawnTarget(targetPos.x, targetPos.y, targetPos.z, radius, TargetShape.SPHERE, { 0.2, 0.8, 0.2 })
	setTargetsPersistent(true)
	setAutoShoot(true)
	setCooldown(0.04)
end

function onUpdate(dt)
	jumpTimer = jumpTimer + dt

	if jumpTimer >= jumpInterval then
		-- Random jump direction in an octagon-like pattern
		local angle = math.random(0, 7) * (math.pi / 4)
		targetVel.x = math.cos(angle) * jumpSpeed
		targetVel.y = math.sin(angle) * jumpSpeed
		jumpTimer = 0
	end

	-- Update position
	targetPos.x = targetPos.x + targetVel.x * dt
	targetPos.y = targetPos.y + targetVel.y * dt

	-- Decay velocity (air resistance simulation)
	targetVel.x = targetVel.x * (1 - 5 * dt)
	targetVel.y = targetVel.y * (1 - 5 * dt)

	-- Constraints (Keep it in view)
	if targetPos.x > 12 or targetPos.x < -12 then
		targetVel.x = -targetVel.x
	end
	if targetPos.y > 10 or targetPos.y < 1 then
		targetVel.y = -targetVel.y
	end

	clearTargets()
	spawnTarget(targetPos.x, targetPos.y, targetPos.z, radius, TargetShape.SPHERE, { 0.2, 0.8, 0.2 })
end

function onHit(x, y, z)
	-- Points for tracking the frog
end
