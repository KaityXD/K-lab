scenarioName = "Gridshot 7x7"

-- Grid 7x7 HP Tracking
-- 4 targets on a 7x7 grid with high HP.
-- Designed for precision switching and tracking.

local targetCount = 4
local grid_size = 7
local spacing = 2.2
local base_z = -12
local base_y = 7
local maxHP = 600

function onStart()
	setPlayerPos(0, 5, 5.0)
	setTimeOut(60)
	setIsAuto(true)
	setCooldown(0.04)
	setTargetsPersistent(false) -- They disappear when HP=0
	clearTargets()

	for i = 1, targetCount do
		spawnIntoRandomFreeSpot()
	end
end

function getGridPos(row, col)
	local x = (col - (grid_size + 1) / 2) * spacing
	local y = base_y + (row - (grid_size + 1) / 2) * spacing
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

			if not isOccupied then
				table.insert(free_spots, { x = gx, y = gy, z = gz })
			end
		end
	end

	if #free_spots > 0 then
		local spot = free_spots[math.random(#free_spots)]
		spawnTarget(spot.x, spot.y, spot.z, 0.45, TargetShape.SPHERE, { 1, 0.5, 0.1 }, { 0, 0, 0 }, 0, maxHP, true)
	end
end

function onUpdate(dt)
	while getTargetCount() < targetCount do
		spawnIntoRandomFreeSpot()
	end
end

function onHit(x, y, z)
	-- Scoring is automatic
end
