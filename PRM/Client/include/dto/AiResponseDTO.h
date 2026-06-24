#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

struct AiSkillMatchRequest {
    std::string requirement;

    nlohmann::json toJson() const {
        return {{"requirement", requirement}};
    }
};

struct AiRiskSummaryRequest {
    int projectId;

    nlohmann::json toJson() const {
        return {{"project_id", projectId}};
    }
};

struct AiTeamBuilderRequest {
    std::string requirement;

    nlohmann::json toJson() const {
        return {{"requirement", requirement}};
    }
};

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

struct AiCandidateDTO {
    int employee_id = 0;
    std::string name;
    std::string reason;
};

struct AiTeamMemberDTO {
    int employee_id = 0;
    std::string name;
    std::string role;
    std::string reason;
};

struct AiRiskSummaryDTO {
    std::string summary;
};

struct AiSkillMatchResponse {
    std::vector<AiCandidateDTO> candidates;
    std::optional<std::string> fallback_message;

    static AiSkillMatchResponse fromJson(const nlohmann::json& j) {
        AiSkillMatchResponse res;
        
        // If the server explicitly flagged an error or didn't return success
        if (!j.contains("success") || !j["success"].get<bool>()) {
            res.fallback_message = j.value("message", "AI service error.");
            return res;
        }

        auto data = j.value("data", nlohmann::json::object());

        if (data.is_string()) {
            res.fallback_message = data.get<std::string>();
        } else if (data.contains("raw")) {
            res.fallback_message = data["raw"].get<std::string>();
        } else if (data.is_array()) {
            for (const auto& item : data) {
                AiCandidateDTO c;
                c.employee_id = item.value("employee_id", 0);
                c.name = item.value("name", "Unknown");
                c.reason = item.value("reason", "");
                res.candidates.push_back(c);
            }
        }
        return res;
    }
};

struct AiTeamBuilderResponse {
    std::vector<AiTeamMemberDTO> team;
    std::optional<std::string> fallback_message;

    static AiTeamBuilderResponse fromJson(const nlohmann::json& j) {
        AiTeamBuilderResponse res;

        if (!j.contains("success") || !j["success"].get<bool>()) {
            res.fallback_message = j.value("message", "AI service error.");
            return res;
        }

        auto data = j.value("data", nlohmann::json::object());

        if (data.is_string()) {
            res.fallback_message = data.get<std::string>();
        } else if (data.contains("raw")) {
            res.fallback_message = data["raw"].get<std::string>();
        } else if (data.is_array()) {
            for (const auto& item : data) {
                AiTeamMemberDTO t;
                t.employee_id = item.value("employee_id", 0);
                t.name = item.value("name", "N/A");
                t.role = item.value("role", "Unknown Role");
                t.reason = item.value("reason", "");
                res.team.push_back(t);
            }
        }
        return res;
    }
};

struct AiRiskSummaryResponse {
    AiRiskSummaryDTO data;
    std::optional<std::string> fallback_message;

    static AiRiskSummaryResponse fromJson(const nlohmann::json& j) {
        AiRiskSummaryResponse res;

        if (!j.contains("success") || !j["success"].get<bool>()) {
            res.fallback_message = j.value("message", "AI service error.");
            return res;
        }

        auto data = j.value("data", nlohmann::json::object());

        if (data.is_string()) {
            res.fallback_message = data.get<std::string>();
        } else if (data.contains("raw")) {
            res.fallback_message = data["raw"].get<std::string>();
        } else if (data.is_object() && data.contains("summary")) {
            res.data.summary = data["summary"].get<std::string>();
        } else {
            res.fallback_message = "Unexpected response schema from AI service.";
        }
        return res;
    }
};
