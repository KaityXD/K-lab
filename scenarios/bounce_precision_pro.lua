scenarioName = "test 2"

-- Bounce Precision Pro
-- Multiple small targets bouncing around at high speeds.
-- Tests both tracking and reactive flicking.

local targetCount = 3
local currentTargets = 0
local radius = 0.2
local targets = {}

function spawnBouncingTarget()
	local x = math.random(-5, 5)
	local y = math.random(3, 7)
	local z = -10
	local vx = (math.random() * 2 - 1) * 15
	local vy = (math.random() * 2 - 1) * 15

	local id = #targets + 1
	targets[id] = {
		pos = { x = x, y = y, z = z },
		vel = { x = vx, y = vy, z = 0 },
	}

	currentTargets = currentTargets + 1
end

function onStart()
	setPlayerPos(0, 1, 5.0)
	setIsAuto(true)
	clearTargets()
	currentTargets = 0
	targets = {}
	setAutoShoot(false)
	setTargetsPersistent(true) -- We handle updating them manually in Lua
	setCooldown(0.15)
end

function onUpdate(dt)
	if currentTargets < targetCount then
		spawnBouncingTarget()
	end

	clearTargets()
	for i, t in pairs(targets) do
		-- Update position
		t.pos.x = t.pos.x + t.vel.x * dt
		t.pos.y = t.pos.y + t.vel.y * dt

		-- Bounce off walls
		if t.pos.x > 15 or t.pos.x < -15 then
			t.vel.x = -t.vel.x
			t.pos.x = math.max(-15, math.min(15, t.pos.x))
		end
		if t.pos.y > 10 or t.pos.y < 1 then
			t.vel.y = -t.vel.y
			t.pos.y = math.max(1, math.min(10, t.pos.y))
		end

		spawnTarget(t.pos.x, t.pos.y, t.pos.z, radius, TargetShape.SPHERE, { 0.5, 0.1, 0.9 })
	end
end

function onHit(x, y, z)
	-- Since we clearTargets every update, the engine might not know WHICH target we hit
	-- if we use setTargetsPersistent(true) and clearTargets in onUpdate.
	-- However, Scenario::checkHits is called after update but before the next update.
	-- In this case, we need to remove the target from our Lua table too.
	-- But which one? The engine doesn't tell us.

	-- Let's rethink: if we use setTargetsPersistent(false), it disappears on hit.
	-- But we are clearing all anyway.

	-- Actually, if we just want them to bounce, we can use the engine's velocity.
	-- Wait, does the engine handle bouncing? No, Scenario::update just moves them.

	-- Let's change the strategy to use engine's lifetime or just re-spawn one on hit.
	currentTargets = currentTargets - 1
	-- Reset all targets for simplicity when one is hit, since we don't know which one.
	-- Or better, just clear the table and let onUpdate refill it.
	targets = {}
end
