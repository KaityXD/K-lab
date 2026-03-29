scenarioName = "Seeded Random Test"

description = "Test scenario to verify seeded randomness for replay system"
tags = {"test", "random", "replay"}

function onStart()
    setPlayerPos(0, 3, 5.0)
    setTimeOut(10)
    setIsAuto(false)
    setAutoShoot(false)
    setCooldown(0.1)
    clearTargets()
    
    -- Use the engine's seeded random instead of math.random
    print("Engine seed: " .. (getRandomSeed and getRandomSeed() or "not available"))
    
    -- Spawn 5 targets at random positions using engine's seeded random
    for i = 1, 5 do
        local x = randomFloat(-8, 8)
        local y = randomFloat(2, 8)
        local z = randomFloat(-15, -5)
        local size = randomFloat(0.3, 0.8)
        
        print(string.format("Spawning target %d at (%.2f, %.2f, %.2f) size %.2f", i, x, y, z, size))
        spawnTarget(x, y, z, size, TargetShape.SPHERE, {randomFloat(), randomFloat(), randomFloat()})
    end
    
    -- Test different random distributions
    print("Uniform random samples:")
    for i = 1, 5 do
        print(string.format("  %.4f", randomFloat()))
    end
    
    print("Gaussian random samples:")
    for i = 1, 5 do
        print(string.format("  %.4f", randomGaussian(0, 1)))
    end
    
    print("Integer random samples:")
    for i = 1, 5 do
        print(string.format("  %d", randomInt(1, 10)))
    end
    
    print("Boolean random samples:")
    for i = 1, 5 do
        print(string.format("  %s", randomBool(0.7) and "true" or "false"))
    end
end

function onUpdate(dt)
    -- Occasionally spawn a new target with random properties
    if randomBool(0.02) then  -- 2% chance per frame
        local x = randomFloat(-10, 10)
        local y = randomFloat(1, 6)
        local z = randomFloat(-12, -8)
        local size = randomFloat(0.2, 0.6)
        spawnTarget(x, y, z, size, TargetShape.SPHERE, {randomFloat(), randomFloat(), randomFloat()})
    end
end

function onHit(x, y, z)
    -- Random hit effects
    if randomBool(0.3) then
        emitParticles(x, y, z, {randomFloat(), randomFloat(), randomFloat()}, randomInt(10, 30))
    end
end