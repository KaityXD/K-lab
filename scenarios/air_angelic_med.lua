scenarioName = "Air Angelic Medium"

-- Air Angelic v2 (Slidey Edit)
-- Smooth tracking scenario focused on organic, non-linear movement.
-- Slidey feel: velocity accumulates over time with low damping,
-- so the target glides and carries momentum through direction changes.

local targetPos = { x = 0, y = 6, z = -15 }
local targetVel = { x = 0, y = 0, z = 0 }
local targetRadius = 0.4
local targetHealth = 10000

-- Sine wave layers for base movement (organic flow)
local sineTime = 0
local sineX = { amp = 5.0, freq = 0.7, phase = 0.0 }
local sineY = { amp = 2.5, freq = 1.1, phase = 1.5 } -- offset phase so X and Y desync

-- Random redirect system
local redirectTimer = 0
local redirectInterval = 2.2 -- longer interval = smoother glide between cuts
local redirectForceX = 0
local redirectForceY = 0

-- Speed pulse system (target speeds up/slows down rhythmically)
local speedPulseTime = 0
local baseSpeed = 8.0
local pulseAmount = 3.0

-- Damping: lower = more slidey (momentum carries longer)
local damping = 0.2

-- Boundaries (slightly wider so it has room to glide)
local minX, maxX = -13, 13
local minY, maxY = 1.5, 11.5
local minZ, maxZ = -17, -13

function onStart()
	setPlayerPos(0, 5, 5)
	setIsAuto(true)
	setTargetsPersistent(true)
	clearTargets()
	clearWalls()

	spawnTarget(
		targetPos.x,
		targetPos.y,
		targetPos.z,
		targetRadius,
		TargetShape.SPHERE,
		{ 0.1, 0.1, 0.1 },
		{ 0, 0, 0 },
		0,
		targetHealth,
		true
	)

	-- Walls
	local wallColor = { 0.7, 0.7, 0.7 }
	spawnWall(0, 0, -15, 40, 0.1, 30, wallColor) -- Floor
	spawnWall(0, 7.5, -27, 40, 15, 0.1, wallColor) -- Back wall
	spawnWall(-17, 7.5, -15, 0.1, 15, 30, wallColor) -- Left wall
	spawnWall(17, 7.5, -15, 0.1, 15, 30, wallColor) -- Right wall
	spawnWall(0, 15, -15, 40, 0.1, 30, wallColor) -- Ceiling

	setFireRate(900)
	setAutoShoot(true)

	-- Randomize starting sine phases so each session feels different
	sineX.phase = math.random() * math.pi * 2
	sineY.phase = math.random() * math.pi * 2
end

function onUpdate(dt)
	sineTime = sineTime + dt
	speedPulseTime = speedPulseTime + dt
	redirectTimer = redirectTimer - dt

	-- === SPEED PULSE ===
	local speedMult = baseSpeed + math.sin(speedPulseTime * 1.1) * pulseAmount

	-- === SINE WAVE FORCES ===
	-- These are FORCES added to velocity each frame, not direct assignments.
	-- This is what makes movement feel slidey — the target has to "wind up"
	-- and "coast" rather than instantly matching the sine curve.
	local forceX = math.cos(sineTime * sineX.freq + sineX.phase) * sineX.amp * speedMult * 0.08
	local forceY = math.cos(sineTime * sineY.freq + sineY.phase) * sineY.amp * speedMult * 0.08

	-- === RANDOM REDIRECTS ===
	-- Fires an impulse (one-time force kick) every ~2.2s.
	-- Longer interval than before so glide between kicks is more noticeable.
	if redirectTimer <= 0 then
		local angle = math.random() * math.pi * 2
		local strength = 3.0 + math.random() * 3.0
		redirectForceX = math.cos(angle) * strength
		redirectForceY = math.sin(angle) * strength
		redirectTimer = redirectInterval + (math.random() * 0.8 - 0.4)
	end

	-- === WEAK CENTER TETHER ===
	-- Much weaker than before so it doesn't fight momentum aggressively.
	local tetherX = (0 - targetPos.x) * 0.6
	local tetherY = (6 - targetPos.y) * 0.6

	-- === ACCUMULATE FORCES INTO VELOCITY ===
	-- Key change: += instead of = so previous velocity is preserved (momentum).
	targetVel.x = targetVel.x + (forceX + redirectForceX + tetherX) * dt
	targetVel.y = targetVel.y + (forceY + redirectForceY + tetherY) * dt
	targetVel.z = targetVel.z + (math.sin(sineTime * 0.25) * 0.3 - targetVel.z * 0.5) * dt

	-- Bleed off the redirect impulse so it doesn't accumulate forever
	redirectForceX = redirectForceX * (1 - 3.0 * dt)
	redirectForceY = redirectForceY * (1 - 3.0 * dt)

	-- === DAMPING (air resistance) ===
	-- Low value = momentum carries longer = slidey feel.
	-- Raise toward 1.0 if you want snappier.
	targetVel.x = targetVel.x * (1 - damping * dt)
	targetVel.y = targetVel.y * (1 - damping * dt)
	targetVel.z = targetVel.z * (1 - damping * dt)

	-- === CAP SPEED ===
	local maxVel = 13.0
	local speed = math.sqrt(targetVel.x ^ 2 + targetVel.y ^ 2 + targetVel.z ^ 2)
	if speed > maxVel then
		local s = maxVel / speed
		targetVel.x = targetVel.x * s
		targetVel.y = targetVel.y * s
		targetVel.z = targetVel.z * s
	end

	-- === UPDATE POSITION ===
	targetPos.x = targetPos.x + targetVel.x * dt
	targetPos.y = targetPos.y + targetVel.y * dt
	targetPos.z = targetPos.z + targetVel.z * dt

	-- === SOFT BOUNDARY CLAMP ===
	if targetPos.x < minX or targetPos.x > maxX then
		targetVel.x = 0
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

	-- === SYNC TO ENGINE ===
	if getTargetCount() == 0 then
		spawnTarget(
			targetPos.x,
			targetPos.y,
			targetPos.z,
			targetRadius,
			TargetShape.SPHERE,
			{ 0.1, 0.1, 0.1 },
			{ 0, 0, 0 },
			0,
			targetHealth,
			true
		)
	else
		setTargetPosition(1, targetPos.x, targetPos.y, targetPos.z)
		setTargetHealth(1, targetHealth)
	end
end

function onHit(x, y, z)
	-- Pure tracking practice — no hit reaction needed.
end
