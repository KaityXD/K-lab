scenarioName = "Peeking Bean"

-- Peeking Bean Scenario
-- Target spawns behind a wall and peeks out from left or right side.
-- Random peek distance and side.
-- After reaching peek distance, it stalls. After 1s stall, -10 score every 0.5s.
-- Spawns a new one immediately after hit.

local wall_x = 0
local wall_y = 2
local wall_z = 0
local wall_width = 4
local wall_height = 4

local target_y = 1
local target_z = -0.5
local radius = 0.2
local speed = 12.0

local state = "WAITING" -- WAITING, PEEKING, STALLED, RETURNING
local side = 1 -- 1 for right, -1 for left
local peek_dist = 0
local current_x = 0
local wait_timer = 0
local stall_timer = 0
local penalty_timer = 0

function onStart()
	setPlayerPos(0, 1, 5.0)
	setIsAuto(false)
	clearWalls()
	clearTargets()
	setTargetsPersistent(true)
	setCooldown(0.15)

	-- Spawn the wall in front of the player
	spawnWall(wall_x, wall_y, wall_z, wall_width, wall_height, 0.5, { 0.6, 0.6, 0.6 })

	resetTarget()
end

function resetTarget()
	side = math.random() > 0.5 and 1 or -1
	peek_dist = math.random() * 2.5 + 0.5
	current_x = side * (wall_width / 2 - 0.2)
	state = "WAITING"
	-- Short randomized delay before the next peek (feels more natural than instant)
	wait_timer = math.random() * 0.4 + 0.2
	stall_timer = 0
	penalty_timer = 0
	clearTargets()
end

function onUpdate(dt)
	-- If target was hit or removed and we aren't already waiting to spawn, reset
	if getTargetCount() == 0 and state ~= "WAITING" then
		resetTarget()
		return
	end

	if state == "WAITING" then
		wait_timer = wait_timer - dt
		if wait_timer <= 0 then
			state = "PEEKING"
			-- Spawn the bean target with slim scale {0.6, 1.0, 0.6}
			spawnTarget(current_x, target_y, target_z, radius, TargetShape.BEAN, { 0.3, 0.9, 0.3 }, nil, nil, nil, nil, "Slim Bean", {0.6, 1.0, 0.6})
		end
		return
	end

	local target_edge_x = side * (wall_width / 2 + peek_dist)

	if state == "PEEKING" then
		local move = side * speed * dt
		current_x = current_x + move

		local reached = false
		if side == 1 and current_x >= target_edge_x then
			reached = true
		end
		if side == -1 and current_x <= target_edge_x then
			reached = true
		end

		if reached then
			current_x = target_edge_x
			state = "STALLED"
		end
		setTargetPosition(1, current_x, target_y, target_z)
	elseif state == "STALLED" then
		stall_timer = stall_timer + dt

		-- Penalty logic: after 1s, -10 score every 0.5s
		if stall_timer > 1.0 then
			penalty_timer = penalty_timer + dt
			if penalty_timer >= 0.5 then
				setScore(getScore() - 10)
				penalty_timer = penalty_timer - 0.5
				setTargetColor(1, 1.0, 0.2, 0.2) -- Target turns red
			end
		end

		-- Return after total stall time if not hit
		if stall_timer > 2.5 then
			state = "RETURNING"
		end
	elseif state == "RETURNING" then
		local move = -side * speed * dt
		current_x = current_x + move

		local hidden_x = side * (wall_width / 2 - 0.5)
		local returned = false
		if side == 1 and current_x <= hidden_x then
			returned = true
		end
		if side == -1 and current_x >= hidden_x then
			returned = true
		end

		if returned then
			removeTarget(1)
			resetTarget()
		else
			setTargetPosition(1, current_x, target_y, target_z)
		end
	end
end

function onHit(x, y, z)
	-- Target is hit: immediately start the reset for the next spawn
	resetTarget()
end
