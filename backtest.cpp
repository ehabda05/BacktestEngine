#include "backtest.hpp"
#include <stdexcept>
#include <cmath>

Backtest::Backtest() : startCapital_(0), endCapital_(0), returns_(0.0), drawDown_(0.0), sharpe_(0.0), turnover_(0.0) {}

void Backtest::readData(const std::string& filename) {
    // Implementation to read data from a file and populate the 'data' vector
}

void Backtest::setCapital(double capital) {
    startCapital_ = capital;
    endCapital_ = capital; // Initialize endCapital to startCapital
}

void Backtest::run(Strategy& strategy) {
    // Implementation to run the backtest using the provided strategy
}

double Backtest::getStartCapital() const {
    return startCapital_;
}

double Backtest::getEndCapital() const {
    return endCapital_;
}

double Backtest::getReturns() const {
    return returns_;
}

double Backtest::getDrawDown() const {
    return drawDown_;
}

double Backtest::getSharpe() const {
    return sharpe_;
}

double Backtest::getTurnover() const {
    return turnover_;
}

void Backtest::print() const {
    // Implementation to print the results of the backtest
}

void Backtest::reset() {
    startCapital_ = 0;
    endCapital_ = 0;
    returns_ = 0.0;
    drawDown_ = 0.0;
    sharpe_ = 0.0;
    turnover_ = 0.0;
    data_.clear();
}

MeanReversionStrategy::MeanReversionStrategy(std::size_t lookback, double entry, double exit) : lookback_(lookback), entry_(entry), exit_(exit) {
    // Mean Reversion Strategy implementation
    if (lookback_ == 0) {
        throw std::invalid_argument("Lookback period must be greater than 0");
    }
    if (entry_ <= 0 || exit_ <= 0) {
        throw std::invalid_argument("Entry and exit thresholds must be greater than 0");
    }
    if (exit_ >= entry_) {
        throw std::invalid_argument("Exit threshold must be less than entry threshold");
    }
}

Signal MeanReversionStrategy::onBar(const std::vector<Bar>& data, std::size_t i) {
    // Implementation of the mean reversion strategy logic
    if (i < lookback_ || i >= data.size()) {
        return Signal::Hold;
    }

    double sum = 0.0;

    for (std::size_t j = i - lookback_; j < i; ++j) {
        sum += data[j].close;
    }

    double mean = sum / lookback_;

    double variance = 0.0;

    for (std::size_t j = i - lookback_; j < i; ++j) {
        double diff = data[j].close - mean;
        variance += diff * diff;
    }

    double stddev = std::sqrt(variance / lookback_);

    if (stddev == 0.0) {
        return Signal::Hold;
    }

    double zScore = (data[i].close - mean) / stddev;

    if (zScore < -entry_) {
        return Signal::Buy;
    }

    if (zScore > entry_) {
        return Signal::Sell;
    }

    return Signal::Hold;
}

MomentumStrategy::MomentumStrategy(std::size_t lookback, double threshold) : lookback_(lookback), threshold_(threshold) {
    // Momentum Strategy implementation
    if (lookback_ == 0) {
        throw std::invalid_argument("Lookback period must be greater than 0");
    }
    if (threshold_ <= 0) {
        throw std::invalid_argument("Threshold must be greater than 0");
    }
}

Signal MomentumStrategy::onBar(const std::vector<Bar>& data, std::size_t i) {
    // Implementation of the momentum strategy logic
     if (i < lookback_ || i >= data.size()) {
        return Signal::Hold;
    }

    double oldPrice = data[i - lookback_].close;
    double newPrice = data[i].close;

    if (oldPrice == 0.0) {
        return Signal::Hold;
    }

    double momentum = (newPrice - oldPrice) / oldPrice;

    if (momentum > threshold_) {
        return Signal::Buy;
    } else if (momentum < -threshold_) {
        return Signal::Sell;
    } else {
        return Signal::Hold;
    }
}

BreakoutStrategy::BreakoutStrategy(std::size_t lookback) : lookback_(lookback) {
    // Breakout Strategy implementation
    if (lookback_ == 0) {
        throw std::invalid_argument("Lookback period must be greater than 0");
    }
}

Signal BreakoutStrategy::onBar(const std::vector<Bar>& data, std::size_t i) {
    // Implementation of the breakout strategy logic
    if (i < lookback_ || i >= data.size()) {
        return Signal::Hold;
    }

    double highestHigh = data[i - lookback_].high;
    double lowestLow = data[i - lookback_].low;

    for (std::size_t j = i - lookback_; j < i; ++j) {
        if (data[j].high > highestHigh) {
            highestHigh = data[j].high;
        }

        if (data[j].low < lowestLow) {
            lowestLow = data[j].low;
        }
    }

    double newPrice = data[i].close;

    if (newPrice > highestHigh) {
        return Signal::Buy;
    } else if (newPrice < lowestLow) {
        return Signal::Sell;
    } else {
        return Signal::Hold;
    }
}
