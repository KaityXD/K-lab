scenarioName = "Smooth Track Bean"

-- Smooth Track Bean Scenario
-- Target slides on the floor, gains speed over 5 seconds, then resets.

local timer = 0
local pos = { x = 0, y = 0.05, z = -10 }
local dir = { x = 1, z = 0 }
local baseSpeed = 4.0
local accel = 1.5
local radius = 0.5

function onStart()
	setIsAuto(true) -- High fire rate for tracking
	setAutoShoot(true)
	setTargetsPersistent(true) -- Don't destroy on hit
	setFireRate(900)
	resetTarget()
end

function resetTarget()
	timer = 0
	pos.x = math.random(-8, 8)
	pos.z = math.random(-15, -10)

	-- Random direction
	local angle = math.random() * 2 * math.pi
	dir.x = math.cos(angle)
	dir.z = math.sin(angle)
end

function onUpdate(dt)
	timer = timer + dt
	if timer >= 5.0 then
		resetTarget()
	end

	local currentSpeed = baseSpeed + accel * timer
	pos.x = pos.x + dir.x * currentSpeed * dt
	pos.z = pos.z + dir.z * currentSpeed * dt

	-- Bounds check (arena limits)
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
	-- spawnTarget(x, y, z, r, shape, color)
	spawnTarget(pos.x, pos.y, pos.z, radius, TargetShape.BEAN, { 1, 0.5, 0.1 })
	setTargetInvincible(1, true)
end

function onHit(x, y, z)
	-- Score is handled by the engine
end
