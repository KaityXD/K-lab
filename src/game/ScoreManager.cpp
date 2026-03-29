#include "ScoreManager.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <sqlite3.h>
#include <iostream>

namespace klab {

ScoreManager& ScoreManager::getInstance() {
    static ScoreManager instance;
    return instance;
}

ScoreManager::~ScoreManager() {
    if (m_db) {
        sqlite3_close((sqlite3*)m_db);
    }
}

static std::string vectorToString(const std::vector<int>& v) {
    std::stringstream ss;
    for (size_t i = 0; i < v.size(); ++i) {
        ss << v[i] << (i == v.size() - 1 ? "" : ",");
    }
    return ss.str();
}

static std::vector<int> stringToVector(const std::string& s) {
    std::vector<int> v;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            try {
                v.push_back(std::stoi(item));
            } catch (...) {}
        }
    }
    return v;
}

void ScoreManager::load() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (sqlite3_open(m_dbPath.c_str(), (sqlite3**)&m_db) != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg((sqlite3*)m_db) << std::endl;
        return;
    }

    const char* createTables = 
        "CREATE TABLE IF NOT EXISTS high_scores ("
        "scenario_name TEXT PRIMARY KEY,"
        "score INTEGER,"
        "accuracy REAL,"
        "date TEXT,"
        "timeline TEXT);"
        "CREATE TABLE IF NOT EXISTS run_history ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "scenario_name TEXT,"
        "score INTEGER,"
        "accuracy REAL,"
        "reaction_time REAL,"
        "flick_speed REAL,"
        "date TEXT,"
        "timeline TEXT);"
        "CREATE TABLE IF NOT EXISTS scenario_metadata ("
        "scenario_name TEXT PRIMARY KEY,"
        "is_favorite INTEGER DEFAULT 0,"
        "play_count INTEGER DEFAULT 0,"
        "last_played TEXT,"
        "difficulty INTEGER DEFAULT 1);";

    char* errMsg = nullptr;
    if (sqlite3_exec((sqlite3*)m_db, createTables, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to create tables: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    // Check if migration is needed
    bool needsMigration = false;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2((sqlite3*)m_db, "SELECT COUNT(*) FROM high_scores", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_int(stmt, 0) == 0) {
                needsMigration = true;
            }
        }
        sqlite3_finalize(stmt);
    }

    if (needsMigration) {
        std::cout << "Migrating legacy score data to SQLite..." << std::endl;
        sqlite3_exec((sqlite3*)m_db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

        // Migrate High Scores
        {
            std::ifstream f(m_filePath);
            if (f.is_open()) {
                std::string line;
                while (std::getline(f, line)) {
                    std::stringstream ss(line);
                    std::string name;
                    int score;
                    float accuracy;
                    std::string date;
                    if (ss >> name >> score >> accuracy >> date) {
                        std::replace(name.begin(), name.end(), '_', ' ');
                        std::vector<int> timeline;
                        int tScore;
                        while (ss >> tScore) timeline.push_back(tScore);
                        
                        const char* sql = "INSERT OR REPLACE INTO high_scores (scenario_name, score, accuracy, date, timeline) VALUES (?, ?, ?, ?, ?)";
                        sqlite3_stmt* mStmt;
                        if (sqlite3_prepare_v2((sqlite3*)m_db, sql, -1, &mStmt, nullptr) == SQLITE_OK) {
                            sqlite3_bind_text(mStmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
                            sqlite3_bind_int(mStmt, 2, score);
                            sqlite3_bind_double(mStmt, 3, accuracy);
                            sqlite3_bind_text(mStmt, 4, date.c_str(), -1, SQLITE_TRANSIENT);
                            std::string timelineStr = vectorToString(timeline);
                            sqlite3_bind_text(mStmt, 5, timelineStr.c_str(), -1, SQLITE_TRANSIENT);
                            sqlite3_step(mStmt);
                            sqlite3_finalize(mStmt);
                        }
                    }
                }
            }
        }

        // Migrate History
        {
            std::ifstream f(m_historyPath);
            if (f.is_open()) {
                std::string line;
                while (std::getline(f, line)) {
                    std::stringstream ss(line);
                    std::string name;
                    int score;
                    float accuracy;
                    float reactionTime;
                    float flickSpeed;
                    std::string date;
                    if (ss >> name >> score >> accuracy >> reactionTime >> flickSpeed >> date) {
                        if (score == 0 && accuracy == 0 && reactionTime == 0 && flickSpeed == 0) continue;
                        std::replace(name.begin(), name.end(), '_', ' ');
                        std::vector<int> timeline;
                        int tScore;
                        while (ss >> tScore) timeline.push_back(tScore);

                        const char* sql = "INSERT INTO run_history (scenario_name, score, accuracy, reaction_time, flick_speed, date, timeline) VALUES (?, ?, ?, ?, ?, ?, ?)";
                        sqlite3_stmt* mStmt;
                        if (sqlite3_prepare_v2((sqlite3*)m_db, sql, -1, &mStmt, nullptr) == SQLITE_OK) {
                            sqlite3_bind_text(mStmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
                            sqlite3_bind_int(mStmt, 2, score);
                            sqlite3_bind_double(mStmt, 3, accuracy);
                            sqlite3_bind_double(mStmt, 4, reactionTime);
                            sqlite3_bind_double(mStmt, 5, flickSpeed);
                            sqlite3_bind_text(mStmt, 6, date.c_str(), -1, SQLITE_TRANSIENT);
                            std::string timelineStr = vectorToString(timeline);
                            sqlite3_bind_text(mStmt, 7, timelineStr.c_str(), -1, SQLITE_TRANSIENT);
                            sqlite3_step(mStmt);
                            sqlite3_finalize(mStmt);
                        }
                    }
                }
            }
        }
        sqlite3_exec((sqlite3*)m_db, "COMMIT;", nullptr, nullptr, nullptr);
    }

    // Load from SQLite into memory
    m_highScores.clear();
    if (sqlite3_prepare_v2((sqlite3*)m_db, "SELECT scenario_name, score, accuracy, date, timeline FROM high_scores", -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = (const char*)sqlite3_column_text(stmt, 0);
            int score = sqlite3_column_int(stmt, 1);
            float accuracy = (float)sqlite3_column_double(stmt, 2);
            std::string date = (const char*)sqlite3_column_text(stmt, 3);
            const char* timelineTxt = (const char*)sqlite3_column_text(stmt, 4);
            std::string timelineStr = timelineTxt ? timelineTxt : "";
            m_highScores[name] = {score, accuracy, date, stringToVector(timelineStr)};
        }
        sqlite3_finalize(stmt);
    }

    m_runHistory.clear();
    // Load last 50 for each scenario. For simplicity, load all and filter in memory if needed,
    // or just load the most recent 1000 runs total.
    if (sqlite3_prepare_v2((sqlite3*)m_db, "SELECT scenario_name, score, accuracy, reaction_time, flick_speed, date, timeline FROM run_history ORDER BY id ASC", -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = (const char*)sqlite3_column_text(stmt, 0);
            int score = sqlite3_column_int(stmt, 1);
            float accuracy = (float)sqlite3_column_double(stmt, 2);
            float reactionTime = (float)sqlite3_column_double(stmt, 3);
            float flickSpeed = (float)sqlite3_column_double(stmt, 4);
            std::string date = (const char*)sqlite3_column_text(stmt, 5);
            const char* timelineTxt = (const char*)sqlite3_column_text(stmt, 6);
            std::string timelineStr = timelineTxt ? timelineTxt : "";
            
            auto& history = m_runHistory[name];
            history.push_back({score, accuracy, reactionTime, flickSpeed, date, stringToVector(timelineStr)});
            if (history.size() > 50) history.erase(history.begin());
        }
        sqlite3_finalize(stmt);
    }
}

void ScoreManager::save() {
    // With SQLite, we save immediately in updateHighScore.
}

void ScoreManager::saveAsync() {
    // SQLite operations are fast enough for single records, 
    // but we can keep this if we want to move updateHighScore to a thread.
    // For now, let's keep it simple.
}

void ScoreManager::updateHighScore(const std::string& scenarioName, int score, float accuracy, float reactionTime, float flickSpeed, const std::vector<int>& scoreTimeline) {
    if (score == 0 && accuracy == 0 && reactionTime == 0 && flickSpeed == 0) return;

    std::time_t now = std::time(nullptr);
    char dateStr[11];
    std::strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", std::localtime(&now));
    std::string date(dateStr);
    
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) return;

    auto& history = m_runHistory[scenarioName];
    if (!history.empty()) {
        const auto& last = history.back();
        if (last.score == score && last.accuracy == accuracy && 
            last.reactionTime == reactionTime && last.flickSpeed == flickSpeed && 
            last.date == date) {
            return;
        }
    }

    history.push_back({score, accuracy, reactionTime, flickSpeed, date, scoreTimeline});
    if (history.size() > 50) history.erase(history.begin());

    auto it = m_highScores.find(scenarioName);
    bool isNewHigh = (it == m_highScores.end() || score > it->second.score);
    if (isNewHigh) {
        m_highScores[scenarioName] = {score, accuracy, date, scoreTimeline};
    }

    // SQLite Save
    std::string timelineStr = vectorToString(scoreTimeline);
    
    sqlite3_stmt* stmt;
    const char* insertHistory = "INSERT INTO run_history (scenario_name, score, accuracy, reaction_time, flick_speed, date, timeline) VALUES (?, ?, ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2((sqlite3*)m_db, insertHistory, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, scenarioName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, score);
        sqlite3_bind_double(stmt, 3, accuracy);
        sqlite3_bind_double(stmt, 4, reactionTime);
        sqlite3_bind_double(stmt, 5, flickSpeed);
        sqlite3_bind_text(stmt, 6, date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, timelineStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    if (isNewHigh) {
        const char* upsertHighScore = "INSERT OR REPLACE INTO high_scores (scenario_name, score, accuracy, date, timeline) VALUES (?, ?, ?, ?, ?)";
        if (sqlite3_prepare_v2((sqlite3*)m_db, upsertHighScore, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, scenarioName.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, score);
            sqlite3_bind_double(stmt, 3, accuracy);
            sqlite3_bind_text(stmt, 4, date.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, timelineStr.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }
}

const std::vector<RunRecord>& ScoreManager::getRunHistory(const std::string& scenarioName) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    static const std::vector<RunRecord> empty;
    auto it = m_runHistory.find(scenarioName);
    if (it == m_runHistory.end()) return empty;
    return it->second;
}

} // namespace klab
