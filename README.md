# K-Lab - High Performance FPS Aim Trainer

K-Lab is a lightweight, scriptable 3D aim trainer built for maximum FPS and minimal input latency. It features a robust C++/OpenGL core and a flexible LuaJIT-powered scenario system, allowing users to create, share, and play custom training modes.

![GitHub License](https://img.shields.io/github/license/user/kim-lol)
![GitHub Stars](https://img.shields.io/github/stars/user/kim-lol)

## ✨ Features

- **🚀 Performance-First:** Built with a custom C++ engine for raw performance and low input lag.
- **📜 Lua Scripting API:** Create complex training scenarios using a simple but powerful Lua API.
- **🛠️ Fully Customizable:** Adjustable FOV, sensitivity, crosshairs, and environment colors.
- **📊 Detailed Statistics:** Track your hits, misses, accuracy, and reaction time.
- **🌍 Cross-Platform:** Supports Linux, macOS, and Windows.
- **🎨 Visuals:** Modern UI powered by Dear ImGui with support for custom shaders and particle effects.

## 🚀 Quick Start

### 📦 Prerequisites
- **CMake** (v3.10+)
- **LuaJIT**
- **GLFW 3**
- **GLEW**
- **GLM**
- **OpenAL / Miniaudio** dependencies

### 🛠️ Building

#### Linux / macOS
```bash
./build.sh
```

#### Windows (MinGW)
```bash
build_windows.bat
```

### 🎮 Running
```bash
./build/klab
```

## 📜 Lua Scenario API

Creating custom scenarios is easy. Simply place a `.lua` file in the `scenarios/` directory.

```lua
scenarioName = "Simple Gridshot"

function onStart()
    setPlayerPos(0, 3, 5)
    setTimeOut(60)
    clearTargets()
    spawnTarget(0, 5, -10, 0.5)
end

function onHit(x, y, z)
    playSound("hit.wav")
    spawnTarget(randomFloat(-10, 10), randomFloat(1, 8), -10, 0.5)
end
```

See [LUA_API.md](LUA_API.md) for the full function reference and advanced features like particle systems, moving targets, and procedural arenas.

## ⌨️ Controls
- **ESC:** Toggle Menu / Pause
- **Left Mouse:** Shoot
- **Mouse:** Aim
- **R:** Restart Scenario
- **F11:** Toggle Fullscreen

## 🤝 Contributing
Contributions are welcome! Feel free to open an issue or submit a pull request for:
- New features or bug fixes.
- Performance optimizations.
- New training scenarios.

## 📄 License
This project is licensed under a custom Non-Commercial License - see the [LICENSE](LICENSE) file for details. Selling this software is strictly prohibited.
