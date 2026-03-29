scenarioName = "Flickshot"

-- Flickshot Scenario

local spawnTimer = 0
local spawnInterval = 1.0
local targetLifetime = 1.5
local activeTargets = {}

function onStart()
	setPlayerPos(0, 1, 5.0)
	setIsAuto(false)
	clearTargets()
	activeTargets = {}
	spawnTimer = 0
	setCooldown(0.15)
end

function spawnNewTarget()
	local x = math.random(-8, 8)
	local y = math.random(1, 8)
	local z = -10
	local id = os.clock() .. math.random()
	table.insert(activeTargets, { x = x, y = y, z = z, id = id, time = 0 })
end

function onUpdate(dt)
	spawnTimer = spawnTimer + dt
	if spawnTimer >= spawnInterval then
		spawnNewTarget()
		spawnTimer = 0
	end

	clearTargets()
	local nextTargets = {}
	for i, t in ipairs(activeTargets) do
		t.time = t.time + dt
		if t.time < targetLifetime then
			spawnTarget(t.x, t.y, t.z, 0.5)
			table.insert(nextTargets, t)
		end
	end
	activeTargets = nextTargets
end

function onHit(x, y, z)
	-- On hit, we should ideally remove the specific target,
	-- but for now we just clear all and let update re-spawn
	-- (simpler for this basic engine)
	activeTargets = {}
end
