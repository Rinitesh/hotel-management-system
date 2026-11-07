#pragma once

#include <string>
#include <utility>

namespace hotel::models {

class ServiceItem {
public:
    ServiceItem() = default;
    ServiceItem(std::string id, std::string name, double price, std::string category, std::string timestamp = "")
        : id_(std::move(id)), name_(std::move(name)), price_(price), category_(std::move(category)), timestamp_(std::move(timestamp)) {}

    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    double getPrice() const { return price_; }
    const std::string& getCategory() const { return category_; }
    const std::string& getTimestamp() const { return timestamp_; }

    void setTimestamp(const std::string& ts) { timestamp_ = ts; }

private:
    std::string id_;
    std::string name_;
    double price_ = 0.0;
    std::string category_;
    std::string timestamp_;
};

} // namespace hotel::models
