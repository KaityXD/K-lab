scenarioName = "Aim Adjust Hard"
-- Aim Adjust Pro Scenario
-- Every shot (hit or miss) makes the target jump in a random direction by a fixed distance
-- Slightly larger targets for better tracking practice.

local targetPos = { x = 0, y = 5, z = -10 }
local targetDest = { x = 0, y = 5, z = -10 }
local jumpDistance = 3.5
local area = { xMin = -10, xMax = 10, yMin = 1, yMax = 9 }
local lerpSpeed = 20.0
local radius = 0.25 -- Increased from 0.4

function onStart()
	setPlayerPos(0, 3.2, 5.0)
	setTimeOut(30)
	setIsAuto(false)
	clearTargets()
	setAutoShoot(false)
	setTargetsPersistent(true)
	targetPos = { x = 0, y = 5, z = -10 }
	targetDest = { x = 0, y = 5, z = -10 }
	spawnTarget(targetPos.x, targetPos.y, targetPos.z, radius, TargetShape.SPHERE, { 1, 1, 0 })
	setCooldown(0.15)
end

function onUpdate(dt)
	-- Smoothly move target towards destination
	targetPos.x = targetPos.x + (targetDest.x - targetPos.x) * math.min(1.0, lerpSpeed * dt)
	targetPos.y = targetPos.y + (targetDest.y - targetPos.y) * math.min(1.0, lerpSpeed * dt)

	clearTargets()
	spawnTarget(targetPos.x, targetPos.y, targetPos.z, radius, TargetShape.SPHERE, { 1, 1, 0 })
end

function moveTarget()
	-- Jump a fixed distance in a random direction (angle)
	local angle = math.random() * 2 * math.pi
	local dx = math.cos(angle) * jumpDistance
	local dy = math.sin(angle) * jumpDistance

	local newX = targetPos.x + dx
	local newY = targetPos.y + dy

	-- If out of bounds, jump in the opposite direction instead
	if newX < area.xMin or newX > area.xMax then
		newX = targetPos.x - dx
	end
	if newY < area.yMin or newY > area.yMax then
		newY = targetPos.y - dy
	end

	-- Final clamp just in case both directions were out
	targetDest.x = math.max(area.xMin, math.min(area.xMax, newX))
	targetDest.y = math.max(area.yMin, math.min(area.yMax, newY))
end

function onHit(x, y, z)
	moveTarget()
end

function onMiss()
	moveTarget()
end
