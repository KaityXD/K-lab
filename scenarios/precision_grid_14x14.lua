scenarioName = "Gridshot Mirco"

-- 14x14 Precision Grid Scenario
-- 5 small targets active in a dense 14x14 grid.
-- Spacing is very close for micro-adjustment practice.

local targetCount = 5
local grid_size = 14
local spacing = 0.8 -- Very close spacing
local base_z = -10
local base_y = 5
local targetRadius = 0.25 -- Small targets

local lastHitX, lastHitY = 0, 0

function onStart()
	setPlayerPos(0, 1, 5.0)
	setTimeOut(60)
	setIsAuto(false)
	setAutoShoot(false)
	setCooldown(0.02)
	clearTargets()

	for i = 1, targetCount do
		spawnIntoRandomFreeSpot()
	end
end

function getGridPos(row, col)
	local x = (col - (grid_size / 2 + 0.5)) * spacing
	local y = base_y + (row - (grid_size / 2 + 0.5)) * spacing
	return x, y, base_z
end

function spawnIntoRandomFreeSpot()
	local activeTargets = getTargets()
	local free_spots = {}

	for r = 1, grid_size do
		for c = 1, grid_size do
			local gx, gy, gz = getGridPos(r, c)
			local isOccupied = false

			for _, t in ipairs(activeTargets) do
				if math.abs(t.x - gx) < 0.1 and math.abs(t.y - gy) < 0.1 then
					isOccupied = true
					break
				end
			end

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
		spawnTarget(spot.x, spot.y, spot.z, targetRadius, TargetShape.SPHERE, { 1, 0.5, 0.2 })
	end
end

function onUpdate(dt)
	while getTargetCount() < targetCount do
		spawnIntoRandomFreeSpot()
	end
end

function onHit(x, y, z)
	lastHitX, lastHitY = x, y
end
