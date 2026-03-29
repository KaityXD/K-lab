scenarioName = "Gridshot Sequential"

-- Gridshot Sequential
-- 3 targets in 3x3 grid: 1 Green, 2 Red.
-- Hit the Green one -> it disappears, one of the remaining Red ones turns Green.
-- When all 3 are gone, spawn 3 new ones.

local targetCount = 3
local grid_size = 3
local spacing = 2.2
local base_z = -10
local base_y = 5

local lastHitX, lastHitY = 0, 0

function onStart()
	setPlayerPos(0, 5, 5.0)
	setTimeOut(60)
	setIsAuto(false)
	setAutoShoot(false)
	setCooldown(0.05)
	setTargetsPersistent(true) -- We handle removal manually via removeTarget
	clearTargets()
	spawnRound()
end

function getGridPos(row, col)
	local x = (col - 2) * spacing
	local y = base_y + (row - 2) * spacing
	return x, y, base_z
end

function spawnRound()
	local spots = {}
	for r = 1, grid_size do
		for c = 1, grid_size do
			local gx, gy, gz = getGridPos(r, c)
			-- Avoid last hit spot if possible
			if math.abs(gx - lastHitX) > 0.1 or math.abs(gy - lastHitY) > 0.1 then
				table.insert(spots, { x = gx, y = gy, z = gz })
			end
		end
	end

	-- Pick 3 unique spots
	local pickedCount = 0
	while pickedCount < 3 and #spots > 0 do
		local idx = math.random(#spots)
		local s = table.remove(spots, idx)

		-- First one is Green, others are Red
		local color = (pickedCount == 0) and { 0, 1, 0 } or { 1, 0, 0 }
		spawnTarget(s.x, s.y, s.z, 0.6, TargetShape.SPHERE, color)
		pickedCount = pickedCount + 1
	end
end

function onUpdate(dt)
	-- If somehow all targets are gone, spawn a new round
	if getTargetCount() == 0 then
		spawnRound()
	end
end

function onHit(x, y, z)
	lastHitX, lastHitY = x, y

	local activeTargets = getTargets()
	local hitGreen = false
	local greenIdx = -1
	local hitIdx = -1

	-- Find which target was hit and if it's green
	for i, t in ipairs(activeTargets) do
		-- Check if this is the target at the hit position
		if math.abs(t.x - x) < 0.1 and math.abs(t.y - y) < 0.1 then
			hitIdx = i
			-- We consider it "green" if its green component is high (crude check)
			-- But it's better to just check if we have any green targets left.
			-- Since we control the colors, we know the first one we spawned was index 1.
			-- However, let's use a more robust way: we'll check the current green index.
			break
		end
	end

	-- We'll track the green index in a local variable for robustness
	-- But since we can't easily store state that persists correctly across refreshes
	-- without careful management, let's just find the one that IS green.
	-- (We can't get color from getTargets() yet, so I'll just rely on a local variable)
end

-- Refined version with local tracking
local currentGreenIdx = 1 -- Index in getTargets() list

function onStart()
	setTimeOut(60)
	setIsAuto(false)
	setAutoShoot(false)
	setCooldown(0.05)
	setTargetsPersistent(true)
	clearTargets()
	currentGreenIdx = 1
	spawnRound()
end

function spawnRound()
	local spots = {}
	for r = 1, grid_size do
		for c = 1, grid_size do
			local gx, gy, gz = getGridPos(r, c)
			if math.abs(gx - lastHitX) > 0.1 or math.abs(gy - lastHitY) > 0.1 then
				table.insert(spots, { x = gx, y = gy, z = gz })
			end
		end
	end

	for i = 1, 3 do
		local idx = math.random(#spots)
		local s = table.remove(spots, idx)
		local color = (i == 1) and { 0, 1, 0 } or { 1, 0, 0 }
		spawnTarget(s.x, s.y, s.z, 0.6, TargetShape.SPHERE, color)
	end
	currentGreenIdx = 1
end

function onHit(x, y, z)
	lastHitX, lastHitY = x, y

	local activeTargets = getTargets()
	local hitIdx = -1
	for i, t in ipairs(activeTargets) do
		if math.abs(t.x - x) < 0.1 and math.abs(t.y - y) < 0.1 then
			hitIdx = i
			break
		end
	end

	if hitIdx == currentGreenIdx then
		-- Hit the correct one!
		removeTarget(hitIdx)

		local remaining = getTargetCount()
		if remaining == 0 then
			spawnRound()
		else
			-- One of the remaining turns green
			currentGreenIdx = math.random(remaining)
			setTargetColor(currentGreenIdx, 0, 1, 0)
		end
	else
		-- Hit a red one! -15 penalty.
		local score = getScore()
		setScore(math.max(0, score - 15 - 10)) -- -10 because engine adds +10 on hit

		-- Fix accuracy: engine counted this as a hit, but it's a "bad" hit.
		setHits(getHits() - 1)
		setMisses(getMisses() + 1)
	end
end

function onMiss()
	local score = getScore()
	setScore(math.max(0, score - 15))
end
