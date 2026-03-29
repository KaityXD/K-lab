scenarioName = "Gravity Track Easy"

-- Gravity Bounce Score Bonus
-- Tracking target with HP.
-- When destroyed, gain +50 score and respawn.

local targetPos = { x = 0, y = 8, z = -12 }
local targetVel = { x = 7, y = 0, z = 0 }
local radius = 0.8
local gravity = 18.0
local bounceFactor = 1.02
local maxHP = 1200.0

local bounds = { xMin = -16, xMax = 16, yMin = 1.2, yMax = 14 }
local lastTargetCount = 0

function onStart()
	setPlayerPos(0, 1, 5.0)
	setTimeOut(60)
	setIsAuto(true)
	setAutoShoot(false)
	setFireRate(900)
	setTargetsPersistent(false) -- Auto-destroy on HP=0
	clearTargets()

	respawn()
	lastTargetCount = 1
end

function respawn()
	targetPos.x = (math.random() - 0.5) * 20
	targetPos.y = 8 + math.random() * 4
	targetVel.x = (math.random() > 0.5 and 1 or -1) * (8 + math.random() * 4)
	targetVel.y = 0

	spawnTarget(
		targetPos.x,
		targetPos.y,
		targetPos.z,
		radius,
		TargetShape.SPHERE,
		{ 0.2, 0.9, 0.5 },
		{ 0, 0, 0 },
		0,
		maxHP,
		true
	)
end

function onUpdate(dt)
	local currentCount = getTargetCount()

	if currentCount < lastTargetCount then
		-- Target was destroyed
		setScore(getScore() + 50)
	end
	lastTargetCount = currentCount

	if currentCount == 0 then
		respawn()
		lastTargetCount = 1
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

	local maxVel = 22
	targetVel.x = math.max(-maxVel, math.min(maxVel, targetVel.x))
	targetVel.y = math.max(-maxVel, math.min(maxVel, targetVel.y))

	setTargetPosition(1, targetPos.x, targetPos.y, targetPos.z)
end

function onHit(x, y, z)
	-- Scoring handled by engine + onUpdate bonus
end
