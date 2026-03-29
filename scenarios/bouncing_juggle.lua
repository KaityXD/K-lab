scenarioName = "Bouncing Juggle"
description = "A simple scenario that my gf mave me an idea shes so cute respect"

-- Scenario based on a movie scene where you keep an object in the air by shooting it.
-- Target has 500 HP (5 shots).
-- On death, a new target respawns.
-- If target hits the ground, -15 penalty and it resets.

local groundLimit = 1.0
local ceilingLimit = 140.0
local spawnPos = { x = 0, y = 8, z = -12 }
local gravity = 18.0
local upwardImpulse = 14.0
local horizontalRandom = 6.0

function spawnNewTarget()
	-- spawnTarget(x, y, z, radius, shape, color, velocity, lifetime, health, showHP)
	spawnTarget(
		spawnPos.x,
		spawnPos.y,
		spawnPos.z,
		0.4,
		TargetShape.SPHERE,
		{ 1.0, 0.6, 0.0 }, -- Orange
		{ 0, 0, 0 },
		0,
		500.0,
		true
	)
	-- The target we just spawned will be the only active one
	setTargetGravity(1.2, gravity)
end

function onStart()
	setTimeOut(30)
	setIsAuto(false) -- Semi-auto as requested
	setFireRate(600)
	clearTargets()
	spawnNewTarget()
end

function onUpdate(dt)
	local targets = getTargets()

	if #targets == 0 then
		spawnNewTarget()
		return
	end

	local t = targets[1]

	-- Ground Penalty
	if t.y < groundLimit then
		-- Penalty
		setScore(getScore() - 15)

		-- Reset target to spawn position
		setTargetPosition(1, spawnPos.x, spawnPos.y, spawnPos.z)
		setTargetVelocity(1, 0, 0, 0)
	end

	-- Ceiling Limit (don't let it fly off screen)
	if t.y > ceilingLimit then
		local vx, vy, vz = getTargetVelocity(1)
		if vx then
			setTargetVelocity(1, vx, -math.abs(vy) * 0.5, vz)
		end
	end

	-- Horizontal Bounds
	if t.x > 15 then
		local vx, vy, vz = getTargetVelocity(1)
		if vx then
			setTargetVelocity(1, -math.abs(vx), vy, vz)
		end
	elseif t.x < -15 then
		local vx, vy, vz = getTargetVelocity(1)
		if vx then
			setTargetVelocity(1, math.abs(vx), vy, vz)
		end
	end
end

function onHit(x, y, z)
	-- Apply upward impulse when hit
	local vx, vy, vz = getTargetVelocity(1)

	-- If vx is nil, the target was likely destroyed by this hit
	if not vx then
		return
	end

	-- Add upward velocity. If falling, we cancel some of the downward momentum and add more.
	local newVY = math.max(vy, 0) + upwardImpulse
	local newVX = vx + (math.random() - 0.5) * horizontalRandom

	setTargetVelocity(1, newVX, newVY, vz)
end

function onMiss()
	-- Standard miss behavior
end
