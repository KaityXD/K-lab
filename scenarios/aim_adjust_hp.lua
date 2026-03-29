scenarioName = "Aim Adjust (switching)"
-- Aim Adjust HP (Smooth)
-- Target stays still while you track it.
-- When HP reaches 0, it heals to full and SMOOTHLY moves to a new location.

local targetPos = { x = 0, y = 5, z = -10 }
local targetDest = { x = 0, y = 5, z = -10 }
local radius = 0.6
local maxHP = 500.0
local lerpSpeed = 10.0 -- Speed of the smooth movement

local area = { xMin = -12, xMax = 12, yMin = 2, yMax = 10 }

function onStart()
	setPlayerPos(0, 3.2, 5.0)

	setTimeOut(30)
	setIsAuto(true)
	setAutoShoot(false)
	setCooldown(0.05)
	setTargetsPersistent(true)
	clearTargets()

	spawnTarget(
		targetPos.x,
		targetPos.y,
		targetPos.z,
		radius,
		TargetShape.SPHERE,
		{ 1, 1, 0 },
		{ 0, 0, 0 },
		0,
		maxHP,
		true
	)
end

function onUpdate(dt)
	local targets = getTargets()
	if #targets == 0 then
		spawnTarget(
			targetPos.x,
			targetPos.y,
			targetPos.z,
			radius,
			TargetShape.SPHERE,
			{ 1, 1, 0 },
			{ 0, 0, 0 },
			0,
			maxHP,
			true
		)
		return
	end

	local t = targets[1]

	-- When HP is 0, trigger a new destination and heal
	if t.health <= 0 then
		targetDest.x = area.xMin + math.random() * (area.xMax - area.xMin)
		targetDest.y = area.yMin + math.random() * (area.yMax - area.yMin)
		setTargetHealth(1, maxHP)
	end

	-- Smoothly move current position towards destination
	-- If it's already at destination, this does nothing (it stays still)
	targetPos.x = targetPos.x + (targetDest.x - targetPos.x) * math.min(1.0, lerpSpeed * dt)
	targetPos.y = targetPos.y + (targetDest.y - targetPos.y) * math.min(1.0, lerpSpeed * dt)

	setTargetPosition(1, targetPos.x, targetPos.y, targetPos.z)
end

function onHit(x, y, z)
	print("lol")
end
