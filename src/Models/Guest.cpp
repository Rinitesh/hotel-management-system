#include "Models/Guest.hpp"
#include <utility>

namespace hotel::models {

Guest::Guest(std::string id, std::string name, std::string email,
             std::string phone, std::string idProof, bool isVIP,
             int totalStays, int rewardPoints)
    : id_(std::move(id)),
      name_(std::move(name)),
      email_(std::move(email)),
      phone_(std::move(phone)),
      idProof_(std::move(idProof)),
      isVIP_(isVIP),
      totalStays_(totalStays),
      rewardPoints_(rewardPoints) {}

void Guest::addStay(int nights) {
    totalStays_ += 1;
    rewardPoints_ += (nights * 100);
    if (totalStays_ >= 3 || rewardPoints_ >= 1000) {
        isVIP_ = true;
    }
}

} // namespace hotel::models
