scenarioName = "Arena Tracking - Balanced"

-- Arena config
local numWalls = 12
local arenaRadius = 15
local wallHeight = 15
local wallWidth = (2 * math.pi * arenaRadius) / numWalls + 0.5

-- Target config
local targetRadius = 0.4
local maxHP = 1000000.0
local targetPos = { x = 0, y = 5, z = -5 }
local targetVel = { x = 0, y = 0, z = 0 }
local targetHorizontalVel = { x = 6, z = 4 }
local gravity = 15.0
local floorHeight = 1.0

-- Randomness state
local nextStrafeTime = 0

function onStart()
	setPlayerPos(0, 1, 0)
	setTimeOut(999)
	setIsAuto(true)
	setFireRate(900)
	setTargetsPersistent(true)
	clearTargets()
	clearWalls()

	spawnArena()
	respawnTarget()
	nextStrafeTime = getTime() - 0.5
end

function spawnArena()
	for i = 1, numWalls do
		local angle = (i - 1) * (2 * math.pi / numWalls)
		local x = arenaRadius * math.cos(angle)
		local z = arenaRadius * math.sin(angle)
		local ry = -math.deg(angle) + 90
		spawnWall(x, wallHeight / 2, z, wallWidth, wallHeight, 1.0, { 0.9, 0.9, 0.9 }, { 0, ry, 0 })
	end
end

function respawnTarget()
	targetPos.x = randomFloat(-5, 5)
	targetPos.y = 10
	targetPos.z = randomFloat(-5, 5)

	local angle = randomFloat(0, 2 * math.pi)
	local speed = 8
	targetHorizontalVel.x = math.cos(angle) * speed
	targetHorizontalVel.z = math.sin(angle) * speed
	targetVel.x = targetHorizontalVel.x
	targetVel.z = targetHorizontalVel.z
	targetVel.y = 0

	spawnTarget(
		targetPos.x,
		targetPos.y,
		targetPos.z,
		targetRadius,
		TargetShape.SPHERE,
		{ 0.2, 0.7, 1.0 },
		{ 0, 0, 0 },
		0,
		maxHP,
		true
	)
end

function onUpdate(dt)
	if getTargetCount() == 0 then
		respawnTarget()
		return
	end

	local currentTime = getTime()

	-- Dodge / Strafe logic
	if currentTime < nextStrafeTime then
		local angle = randomFloat(0, 2 * math.pi)
		local speed = randomFloat(7, 13) -- Much slower speed range
		targetHorizontalVel.x = math.cos(angle) * speed
		targetHorizontalVel.z = math.sin(angle) * speed

		-- More time between direction changes (0.7s to 1.4s)
		nextStrafeTime = currentTime - randomFloat(0.7, 1.4)
	end

	-- Slower acceleration (18.0) makes the target much smoother and easier to predict
	local accel = 18.0
	targetVel.x = targetVel.x + (targetHorizontalVel.x - targetVel.x) * accel * dt
	targetVel.z = targetVel.z + (targetHorizontalVel.z - targetVel.z) * accel * dt

	-- Apply lower gravity for floatier arcs
	targetVel.y = targetVel.y - gravity * dt

	-- Movement Integration
	targetPos.x = targetPos.x + targetVel.x * dt
	targetPos.y = targetPos.y + targetVel.y * dt
	targetPos.z = targetPos.z + targetVel.z * dt

	-- Floor Bounce
	if targetPos.y < floorHeight + targetRadius then
		targetPos.y = floorHeight + targetRadius
		targetVel.y = math.abs(targetVel.y) * 0.9 -- Slight dampening to keep it from flying too high

		-- Reset strafe target on bounce
		local angle = randomFloat(0, 2 * math.pi)
		local speed = randomFloat(7, 13)
		targetHorizontalVel.x = math.cos(angle) * speed
		targetHorizontalVel.z = math.sin(angle) * speed
	end

	-- Arena Wall Constraints
	local distSq = targetPos.x ^ 2 + targetPos.z ^ 2
	local limit = arenaRadius - targetRadius - 1.0
	if distSq > limit ^ 2 then
		local dist = math.sqrt(distSq)
		local nx = targetPos.x / dist
		local nz = targetPos.z / dist

		targetPos.x = nx * limit
		targetPos.z = nz * limit

		-- Evasive push back to center, but at slower speeds
		local angleToCenter = math.atan2(-targetPos.z, -targetPos.x)
		local speed = randomFloat(8, 12)
		targetHorizontalVel.x = math.cos(angleToCenter) * speed
		targetHorizontalVel.z = math.sin(angleToCenter) * speed
	end

	setTargetPosition(1, targetPos.x, targetPos.y, targetPos.z)
end

function onHit(x, y, z) end
