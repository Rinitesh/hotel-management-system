#pragma once

#include <string>

namespace hotel::models {

class Guest {
public:
    Guest() = default;
    Guest(std::string id, std::string name, std::string email,
          std::string phone, std::string idProof, bool isVIP = false,
          int totalStays = 0, int rewardPoints = 0);

    // Getters
    [[nodiscard]] const std::string& getId() const { return id_; }
    [[nodiscard]] const std::string& getName() const { return name_; }
    [[nodiscard]] const std::string& getEmail() const { return email_; }
    [[nodiscard]] const std::string& getPhone() const { return phone_; }
    [[nodiscard]] const std::string& getIdProof() const { return idProof_; }
    [[nodiscard]] bool isVIP() const { return isVIP_; }
    [[nodiscard]] int getTotalStays() const { return totalStays_; }
    [[nodiscard]] int getRewardPoints() const { return rewardPoints_; }

    // Setters / Updates
    void setVIP(bool vip) { isVIP_ = vip; }
    void addStay(int nights);
    void addRewardPoints(int points) { rewardPoints_ += points; }

private:
    std::string id_;
    std::string name_;
    std::string email_;
    std::string phone_;
    std::string idProof_;
    bool isVIP_ = false;
    int totalStays_ = 0;
    int rewardPoints_ = 0;
};

} // namespace hotel::models
