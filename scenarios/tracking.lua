-- Tracking Scenario

local targetPos = { x = 0, y = 5, z = -10 }
local targetVel = { x = 5, y = 0, z = 0 }
local radius = 0.5
local score = 0

scenarioName = "Basic Tracking"
description = "A simple scenario to practice smooth tracking of a moving target."
tags = { "tracking", "flicking" }

function onStart()
	score = 0
	setIsAuto(true)
	clearTargets()
	spawnTarget(targetPos.x, targetPos.y, targetPos.z, radius)
	setAutoShoot(true)
	setTargetsPersistent(true)
	setFireRate(700) -- High fire rate for tracking
end

function onUpdate(dt)
	targetPos.x = targetPos.x + targetVel.x * dt
	targetPos.y = targetPos.y + targetVel.y * dt

	if targetPos.x > 10 or targetPos.x < -10 then
		targetVel.x = -targetVel.x
	end

	clearTargets()
	spawnTarget(targetPos.x, targetPos.y, targetPos.z, radius)
	setTargetInvincible(1, true)
end

function onHit(x, y, z)
	score = score + 10
	setScore(score)
end
