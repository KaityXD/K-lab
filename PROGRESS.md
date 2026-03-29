# Progress Report - March 28, 2026

## Intructions
- list ev

## Tasks Completed

### 1. SQLite Migration for Score Saving
- **Goal:** Move from `.cfg` files to a SQLite database for high scores and run history.
- **Implementation:**
    - Updated `CMakeLists.txt` to find and link against `sqlite3`.
    - Modified `ScoreManager.hpp` to include a `sqlite3*` database handle.
    - Updated `ScoreManager.cpp`:
        - Implemented automatic migration from `highscores.cfg` and `history.cfg` to `scores.db` on first run.
        - Created `high_scores` and `run_history` tables.
        - High scores now store the scenario name, score, accuracy, date, and a comma-separated timeline.
        - Run history stores individual run details, allowing for a richer historical view.
- **Result:** Data is now robustly stored in a relational database, removing dependency on brittle text files.

### 2. Smooth Progress Bar
- **Goal:** Smooth out the progress bar that shows the last best run's progress.
- **Implementation:**
    - Implemented linear interpolation between the discrete points in the `scoreTimeline`.
    - The `bestRunProgress` is now calculated using `elapsed` time as a continuous value.
- **Result:** The white marker and bar color transitions are now perfectly smooth.

### 3. Tracking Target Invincibility
- **Goal:** Fix the issue where tracking scenarios incorrectly reported "Killed [Target]".
- **Implementation:**
    - Added an `isInvincible` flag to `HealthComponent` and `Target` classes.
    - Updated `HealthComponent::takeDamage` to prevent health reduction when invincible.
    - Exposed `setTargetInvincible(index, bool)` to Lua.
    - Updated tracking scenarios (`tracking.lua`, `smooth_track_bean.lua`, `reactive_track_pro.lua`).
- **Result:** Tracking scenarios now show +10 score indicators instead of "Killed" messages.

---

## Future Feature Design: Advanced Scenario Browser

The current scenario list is a simple text list. The goal is to create a modern, data-rich browser using ImGui.

### 1. Visual Layout
The browser will be divided into three main sections:
- **Left Sidebar (Navigation/Filters):**
    - Search Bar: Real-time filtering by name.
    - Category Tags: [All], [Flicking], [Tracking], [Precision], [Speed].
    - Sort Options: [Name A-Z], [High Score], [Last Played], [Most Played].
    - Favorites Toggle: Show only favorited scenarios.
- **Center Area (Scenario Grid/List):**
    - Grid of "Scenario Cards". Each card displays:
        - Scenario Name.
        - Best Score (pulled from SQLite).
        - A small badge for "New" or "Recently Played".
        - Difficulty indicator (colored dots: Green/Yellow/Red).
- **Right Panel (Scenario Preview & Stats):**
    - Selected Scenario Details:
        - Description (from Lua metadata).
        - High Score with Date.
        - **Mini-Graph:** A small line chart showing the last 10 runs' scores to visualize progress.
        - Average Accuracy & Reaction Time for this specific scenario.
        - "Play" Button (Large, highlighted).

### 2. Functional Details
- **Search & Filter:**
    - Use a fuzzy search algorithm to find scenarios even with typos.
    - Filter by tags: Scenarios will define tags in Lua (e.g., `tags = {"tracking", "fast"}`).
- **Integration with SQLite:**
    - Fetch metadata (like `play_count` and `last_played`) by adding a new `scenario_metadata` table.
    - Use the `run_history` table to calculate "improvement rate" (e.g., "You are 15% better than last week").
- **Customization:**
    - Ability to "Favorite" a scenario (stored in DB).
    - Custom thumbnails: If a `.png` exists with the same name as the `.lua`, display it in the grid.

### 3. Database Schema Updates (Proposed)
To support the browser, we will add:
```sql
CREATE TABLE IF NOT EXISTS scenario_metadata (
    scenario_name TEXT PRIMARY KEY,
    is_favorite INTEGER DEFAULT 0,
    play_count INTEGER DEFAULT 0,
    last_played TEXT,
    difficulty INTEGER DEFAULT 1 -- 1: Easy, 2: Med, 3: Hard
);
```

### 4. UI Implementation Strategy
- Use `ImGui::BeginChild` for the three-column layout.
- Use `ImGui::Selectable` or `ImGui::Button` for the scenario cards to handle selection.
- Implement a "Loading" state if the list of scenarios becomes very large (scanning many Lua files).
- The "Mini-Graph" will use `ImGui::PlotLines` using the `scoreTimeline` from the `high_scores` table.

---

## Future Feature Design: Replay System (.krp)

The replay system will allow users to record their runs and play them back with perfect accuracy. This is crucial for analyzing performance, sharing "god runs," and verifying high scores.

### 1. Core Architecture
The system will be built around a deterministic re-simulation approach. Instead of recording a full video or every entity's position, we record the **initial state** and the **inputs** over time.

- **ReplayManager:** A new core component responsible for:
    - Managing the recording state (Idle, Recording, Playing).
    - Capturing per-frame input data.
    - Interfacing with the file system for `.krp` files.
    - Overriding engine inputs during playback.

### 2. Replay File Format (.krp)
A compact binary format will be used to keep file sizes small (estimated < 500KB for a 60s run).

#### **Header (Fixed Size)**
| Field | Type | Description |
| :--- | :--- | :--- |
| Magic | char[4] | "KLRP" identifier. |
| Version | uint32 | File format version. |
| Scenario | string | The name of the scenario. |
| Seed | uint32 | The RNG seed used for the run. |
| Timestamp | uint64 | Unix epoch of when the run occurred. |
| Sens / FOV | float[2] | Sensitivity (cm/360) and Field of View. |
| Resolution | uint32[2] | Width and Height of the window. |

#### **Data Blocks (Per Frame)**
Recorded at a fixed **120Hz** rate (aligned with the engine's `m_fixedDeltaTime`), ensuring smoothness and consistency even on high-refresh-rate monitors.

| Field | Type | Description |
| :--- | :--- | :--- |
| mouseX | float | Relative mouse movement (yaw offset). |
| mouseY | float | Relative mouse movement (pitch offset). |
| buttons | uint8 | Bitmask for mouse buttons (Left, Right). |

### 3. Implementation Strategy

#### **Determinism & RNG**
To ensure the replay doesn't "desync," we must ensure the game state evolves identically every time:
- **Seed Seeding:** At the start of a recording, a 32-bit seed is generated. This seed is passed to `srand()` (C++) and `math.randomseed()` (Lua).
- **120Hz Fixed Timestep:** The engine's existing `m_fixedDeltaTime` loop (set to 1/120s) in `Engine::update` will be used for both recording and playback. This decouples the simulation from the rendering framerate, allowing 120Hz precision even if the user's FPS is higher or lower.
- **Simulation Time:** All Lua scenarios using `getTime()` will receive the accumulated simulation time rather than the actual wall-clock time.

#### **Recording Workflow**
1. When a run starts, `ReplayManager::startRecording(seed)` is called.
2. Every engine tick, `recordFrame` captures the current mouse deltas and button states from `Input`.
3. When the run ends, the buffer is serialized to `replays/[timestamp]_[scenario].krp`.

#### **Playback Workflow**
1. The user selects a run from the History UI.
2. `ReplayManager::loadReplay(path)` reads the header and data.
3. The engine resets the scenario and seeds the RNG with the recorded seed.
4. `Engine::processInput` is bypassed; instead, the `ReplayManager` provides the mouse offsets and button states for the current frame index.
5. The UI displays playback controls (Play/Pause, 0.5x to 4x speed).

### 4. Database & UI Integration
- **SQLite Update:** The `run_history` table will be updated with a `replay_path` column.
- **Scenario Browser:** A new "Replays" tab or a button next to history entries to trigger playback.
- **HUD:** A "REPLAY" watermark and playback progress bar will be shown during playback.

---

## Next Steps
1. **Research Phase:** Check how many Lua files we currently have and if we need to optimize the scanning process.
2. **Implementation Phase:** Create the `ScenarioBrowser` class in `src/ui/`.
3. **Refactoring:** Move the current scenario selection logic from `Menu.cpp` into the new browser.
4. **Replay System:** Begin implementing `ReplayManager` after the browser is stable.
