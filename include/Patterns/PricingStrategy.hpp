#pragma once

#include <string>
#include <algorithm>

namespace hotel::patterns {

class PricingStrategy {
public:
    virtual ~PricingStrategy() = default;
    [[nodiscard]] virtual double calculateTotal(double baseRate, int nights) const = 0;
    [[nodiscard]] virtual std::string getStrategyName() const = 0;
    [[nodiscard]] virtual double getMultiplier() const = 0;
};

class StandardPricingStrategy : public PricingStrategy {
public:
    [[nodiscard]] double calculateTotal(double baseRate, int nights) const override {
        return baseRate * static_cast<double>(nights);
    }
    [[nodiscard]] std::string getStrategyName() const override {
        return "Standard Rate";
    }
    [[nodiscard]] double getMultiplier() const override {
        return 1.0;
    }
};

class VIPPricingStrategy : public PricingStrategy {
public:
    explicit VIPPricingStrategy(double discountPercentage = 15.0)
        : discountPct_(discountPercentage) {}

    [[nodiscard]] double calculateTotal(double baseRate, int nights) const override {
        double factor = (100.0 - discountPct_) / 100.0;
        return baseRate * static_cast<double>(nights) * factor;
    }
    [[nodiscard]] std::string getStrategyName() const override {
        return "VIP Loyalty Discount (" + std::to_string(static_cast<int>(discountPct_)) + "% off)";
    }
    [[nodiscard]] double getMultiplier() const override {
        return (100.0 - discountPct_) / 100.0;
    }

private:
    double discountPct_ = 15.0;
};

class PeakSeasonPricingStrategy : public PricingStrategy {
public:
    explicit PeakSeasonPricingStrategy(double surchargePercentage = 25.0)
        : surchargePct_(surchargePercentage) {}

    [[nodiscard]] double calculateTotal(double baseRate, int nights) const override {
        double factor = (100.0 + surchargePct_) / 100.0;
        return baseRate * static_cast<double>(nights) * factor;
    }
    [[nodiscard]] std::string getStrategyName() const override {
        return "Peak Season Surcharge (+" + std::to_string(static_cast<int>(surchargePct_)) + "%)";
    }
    [[nodiscard]] double getMultiplier() const override {
        return (100.0 + surchargePct_) / 100.0;
    }

private:
    double surchargePct_ = 25.0;
};

class ExtendedStayPricingStrategy : public PricingStrategy {
public:
    explicit ExtendedStayPricingStrategy(int thresholdNights = 7, double discountPct = 10.0)
        : threshold_(thresholdNights), discountPct_(discountPct) {}

    [[nodiscard]] double calculateTotal(double baseRate, int nights) const override {
        if (nights >= threshold_) {
            double factor = (100.0 - discountPct_) / 100.0;
            return baseRate * static_cast<double>(nights) * factor;
        }
        return baseRate * static_cast<double>(nights);
    }
    [[nodiscard]] std::string getStrategyName() const override {
        return "Extended Stay Discount (7+ nights, 10% off)";
    }
    [[nodiscard]] double getMultiplier() const override {
        return (100.0 - discountPct_) / 100.0;
    }

private:
    int threshold_ = 7;
    double discountPct_ = 10.0;
};

} // namespace hotel::patterns
