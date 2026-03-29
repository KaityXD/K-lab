scenarioName = "Gridshot (medium)"
-- 3x3 Gridshot Scenario (30 seconds)
-- 3 targets active. Only the one that is hit respawns.

local targetCount = 3
local grid_size = 3
local spacing = 2.2
local base_z = -10
local base_y = 5

local lastHitX = 0
local lastHitY = 0

function onStart()
	setPlayerPos(0, 3, 5.0)
	setTimeOut(30)
	setIsAuto(false)
	setAutoShoot(false)
	setCooldown(0.04)
	clearTargets()

	-- Initial spawn of 3 targets
	for i = 1, targetCount do
		spawnIntoRandomFreeSpot()
	end
end

function getGridPos(row, col)
	local x = (col - 2) * spacing
	local y = base_y + (row - 2) * spacing
	return x, y, base_z
end

function spawnIntoRandomFreeSpot()
	local activeTargets = getTargets()
	local free_spots = {}

	for r = 1, grid_size do
		for c = 1, grid_size do
			local gx, gy, gz = getGridPos(r, c)
			local isOccupied = false

			-- Check if any active target is at this grid spot
			for _, t in ipairs(activeTargets) do
				if math.abs(t.x - gx) < 0.1 and math.abs(t.y - gy) < 0.1 then
					isOccupied = true
					break
				end
			end

			-- Also exclude the last hit spot
			if math.abs(gx - lastHitX) < 0.1 and math.abs(gy - lastHitY) < 0.1 then
				isOccupied = true
			end

			if not isOccupied then
				table.insert(free_spots, { x = gx, y = gy, z = gz })
			end
		end
	end

	if #free_spots > 0 then
		local spot = free_spots[math.random(#free_spots)]
		spawnTarget(spot.x, spot.y, spot.z, 0.6, TargetShape.SPHERE, { 1, 0.2, 0.2 })
	end
end

function onUpdate(dt)
	while getTargetCount() < targetCount do
		spawnIntoRandomFreeSpot()
	end
end

function onHit(x, y, z)
	lastHitX = x
	lastHitY = y
end
