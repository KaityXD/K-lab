scenarioName = "Fast Track Bean"

-- Dash Track Bean Scenario
-- Target slides on the floor and performs smooth dashes.
-- Velocity is now vector-interpolated for readable momentum.

local pos = { x = 0, y = 0.5, z = -12 } -- Fixed Y to equal radius (no floor clipping)
local vel = { x = 0, z = 0 }
local targetVel = { x = 0, z = 0 }
local radius = 0.5

local stateTimer = 0
local state = "WAIT"

function onStart()
	setPlayerPos(0, 1, 5.0)
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
	local speed = math.random(10, 16) -- Slightly bumped since we interpolate the whole vector now

	-- Set the target velocity vector
	targetVel.x = math.cos(angle) * speed
	targetVel.z = math.sin(angle) * speed
end

function onUpdate(dt)
	stateTimer = stateTimer - dt

	if stateTimer <= 0 then
		if state == "DASH" then
			state = "WAIT"
			stateTimer = math.random(0.3, 0.7) -- Pause for re-acquisition
			targetVel.x = 0
			targetVel.z = 0
		else
			resetDash()
		end
	end

	-- Smoothly interpolate the actual velocity vector (feels way better to track)
	local accel = 8.0 -- Increase for snappier direction changes, decrease for more slide
	vel.x = vel.x + (targetVel.x - vel.x) * dt * accel
	vel.z = vel.z + (targetVel.z - vel.z) * dt * accel

	-- Apply velocity to position
	pos.x = pos.x + vel.x * dt
	pos.z = pos.z + vel.z * dt

	-- Bounds check with clean velocity reflection
	if pos.x > 12 then
		pos.x = 12
		vel.x = -math.abs(vel.x)
		targetVel.x = -math.abs(targetVel.x)
	elseif pos.x < -12 then
		pos.x = -12
		vel.x = math.abs(vel.x)
		targetVel.x = math.abs(targetVel.x)
	end

	if pos.z > -5 then
		pos.z = -5
		vel.z = -math.abs(vel.z)
		targetVel.z = -math.abs(targetVel.z)
	elseif pos.z < -20 then
		pos.z = -20
		vel.z = math.abs(vel.z)
		targetVel.z = math.abs(targetVel.z)
	end

	clearTargets()
	spawnTarget(pos.x, pos.y, pos.z, radius, TargetShape.BEAN, { 0.2, 0.8, 1.0 })
	setTargetInvincible(1, true)
end

function onHit(x, y, z) end
