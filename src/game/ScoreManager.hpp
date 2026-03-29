#pragma once
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <future>

namespace klab {

struct HighScore {
    int score;
    float accuracy;
    std::string date;
    std::vector<int> scoreTimeline;
};

struct RunRecord {
    int score;
    float accuracy;
    float reactionTime;
    float flickSpeed;
    std::string date;
    std::vector<int> scoreTimeline;
};

class ScoreManager {
public:
    static ScoreManager& getInstance();

    void load();
    void save();
    void saveAsync();

    void updateHighScore(const std::string& scenarioName, int score, float accuracy, float reactionTime = 0, float flickSpeed = 0, const std::vector<int>& scoreTimeline = {});
    const std::map<std::string, HighScore>& getHighScores() const { return m_highScores; }
    const std::vector<RunRecord>& getRunHistory(const std::string& scenarioName) const;

private:
    ScoreManager() = default;
    ~ScoreManager();
    mutable std::mutex m_mutex;
    std::future<void> m_saveFuture;

    std::map<std::string, HighScore> m_highScores;
    std::map<std::string, std::vector<RunRecord>> m_runHistory;
    
    void* m_db = nullptr; // sqlite3*
    std::string m_dbPath = "scores.db";
    
    // Legacy paths for migration
    std::string m_filePath = "highscores.cfg";
    std::string m_historyPath = "history.cfg";
};

} // namespace klab
