scenarioName = "Gravity Track HP"

-- Gravity Bounce Tracking Scenario (Improved)
-- A single target that requires constant tracking.
-- Now with persistent HP and better movement.

local targetPos = { x = 0, y = 8, z = -12 }
local targetVel = { x = 7, y = 0, z = 0 }
local radius = 0.8 -- Slightly smaller for challenge
local gravity = 18.0
local bounceFactor = 1.05 -- Slight acceleration on bounce to keep energy
local maxHP = 1500.0

local bounds = { xMin = -16, xMax = 16, yMin = 1.2, yMax = 14 }

function onStart()
	setPlayerPos(0, 1, 5.0)
	setTimeOut(30)
	setIsAuto(true)
	setAutoShoot(false)
	setFireRate(900)
	setTargetsPersistent(true)
	clearTargets()

	spawnTarget(
		targetPos.x,
		targetPos.y,
		targetPos.z,
		radius,
		TargetShape.SPHERE,
		{ 0.1, 0.8, 1.0 },
		{ 0, 0, 0 },
		0,
		maxHP,
		false
	)
end

function respawn()
	targetPos.x = (math.random() - 0.5) * 20
	targetPos.y = 8 + math.random() * 4
	targetVel.x = (math.random() > 0.5 and 1 or -1) * (8 + math.random() * 4)
	targetVel.y = 0

	clearTargets()
	spawnTarget(
		targetPos.x,
		targetPos.y,
		targetPos.z,
		radius,
		TargetShape.SPHERE,
		{ 0.1, 0.8, 1.0 },
		{ 0, 0, 0 },
		0,
		maxHP,
		false
	)
end

function onUpdate(dt)
	if getTargetCount() == 0 then
		respawn()
		return
	end

	-- Apply Gravity to Velocity
	targetVel.y = targetVel.y - gravity * dt

	-- Apply Velocity to Position
	targetPos.x = targetPos.x + targetVel.x * dt
	targetPos.y = targetPos.y + targetVel.y * dt

	-- Floor Bounce
	if targetPos.y - radius < bounds.yMin then
		targetPos.y = bounds.yMin + radius
		targetVel.y = math.abs(targetVel.y) * bounceFactor
		-- Change horizontal direction slightly on bounce
		targetVel.x = targetVel.x + (math.random() - 0.5) * 6
	end

	-- Side Wall Bounces
	if targetPos.x + radius > bounds.xMax then
		targetPos.x = bounds.xMax - radius
		targetVel.x = -math.abs(targetVel.x) * 0.95
	elseif targetPos.x - radius < bounds.xMin then
		targetPos.x = bounds.xMin + radius
		targetVel.x = math.abs(targetVel.x) * 0.95
	end

	-- Ceiling Limit
	if targetPos.y + radius > bounds.yMax then
		targetPos.y = bounds.yMax - radius
		targetVel.y = -math.abs(targetVel.y) * 0.5
	end

	-- Smooth velocity clamping
	local maxVelX = 18
	local maxVelY = 22
	targetVel.x = math.max(-maxVelX, math.min(maxVelX, targetVel.x))
	targetVel.y = math.max(-maxVelY, math.min(maxVelY, targetVel.y))

	-- Update the actual target position in the engine
	setTargetPosition(1, targetPos.x, targetPos.y, targetPos.z)
end

function onHit(x, y, z)
	-- Scoring is automatic
end
