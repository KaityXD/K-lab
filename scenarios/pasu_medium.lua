scenarioName = "Pasu Medium"
-- Pasu Medium (Improved)
-- 5 medium-sized targets moving with random direction changes.
-- Elevated player position and larger environment.

local targetCount = 5
local radius = 0.3
local speed = 7
local playerY = 6.6 -- 1.6 + 5.0

-- INCREASED BOUNDARIES
local boundsX = 12 -- Wider Copyrightzontal range
local boundsYMin = playerY - 6 -- Much higher vertical range
local boundsYMax = playerY + 13
local zPos = -15
local color = { 0.8, 0.8, 0.2 } -- Yellowish

-- Table to store targets' next direction change time
local targetData = {}

function spawnPasuTarget()
	-- Spawn within the new larger bounds
	local x = (math.random() * 2 - 1) * (boundsX - 2)
	local y = playerY + (math.random() * 2 - 1) * 10

	local angle = math.random() * 2 * math.pi
	local vx = math.cos(angle) * speed
	local vy = math.sin(angle) * speed

	spawnTarget(x, y, zPos, radius, TargetShape.SPHERE, color, { vx, vy, 0 })
end

function onStart()
	setIsAuto(false)
	clearTargets()
	clearWalls()

	-- Move player up
	setPlayerPos(0, 10, 5.0)

	-- Spawn a large wall centered at player's height (increased to 70x50)
	spawnWall(0, playerY, -20, 30, 30, 1, { 0.5, 0.5, 0.5 })

	targetData = {}
	for i = 1, targetCount do
		spawnPasuTarget()
		targetData[i] = { nextChange = getTime() + math.random() * 2 }
	end
end

function onUpdate(dt)
	local targets = getTargets()
	local currentTime = getTime()

	for i, t in ipairs(targets) do
		-- Initialize data if it's a new target (spawned after hit)
		if not targetData[i] then
			targetData[i] = { nextChange = currentTime + math.random() * 2 }
		end

		-- Randomly change direction
		if currentTime >= targetData[i].nextChange then
			local angle = math.random() * 2 * math.pi
			local vx = math.cos(angle) * speed
			local vy = math.sin(angle) * speed
			setTargetVelocity(i, vx, vy, 0)
			targetData[i].nextChange = currentTime + 0.8 + math.random() * 1.5
		end

		-- Boundary checks with math.abs for reliable bouncing
		local vx, vy, vz = getTargetVelocity(i)
		local bounced = false
		local nVX, nVY = vx, vy

		if t.x > boundsX then
			nVX = -math.abs(vx)
			bounced = true
		elseif t.x < -boundsX then
			nVX = math.abs(vx)
			bounced = true
		end

		if t.y > boundsYMax then
			nVY = -math.abs(vy)
			bounced = true
		elseif t.y < boundsYMin then
			nVY = math.abs(vy)
			bounced = true
		end

		if bounced then
			setTargetVelocity(i, nVX, nVY, 0)
			-- Position correction to prevent sticking
			setTargetPosition(
				i,
				math.max(-boundsX, math.min(boundsX, t.x)),
				math.max(boundsYMin, math.min(boundsYMax, t.y)),
				zPos
			)
		end
	end
end

function onHit(x, y, z)
	-- Simply spawn a replacement.
	spawnPasuTarget()
end
