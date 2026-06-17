#include "backtest.hpp"
#include <stdexcept>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

Backtest::Backtest() : startCapital_(0), endCapital_(0), returns_(0.0), drawDown_(0.0), sharpe_(0.0), turnover_(0.0) {}

void Backtest::readData(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    data_.clear();

    std::string line;

    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        Bar bar;

        std::getline(ss, bar.date, ',');

        std::getline(ss, token, ',');
        bar.open = std::stod(token);

        std::getline(ss, token, ',');
        bar.high = std::stod(token);

        std::getline(ss, token, ',');
        bar.low = std::stod(token);

        std::getline(ss, token, ',');
        bar.close = std::stod(token);

        std::getline(ss, token, ',');
        bar.volume = std::stod(token);

        data_.push_back(bar);
    }

    if (data_.empty()) {
        throw std::runtime_error("No data loaded from file: " + filename);
    }
}

void Backtest::setCapital(double capital) {
    startCapital_ = capital;
    endCapital_ = capital; // Initialize endCapital to startCapital
}

void Backtest::run(Strategy& strategy) {
    if (data_.empty()) {
        throw std::runtime_error("No data loaded. Call readData() first.");
    }

    if (startCapital_ <= 0.0) {
        throw std::runtime_error("Starting capital must be greater than 0.");
    }

    double cash = startCapital_;
    int position = 0;

    std::vector<double> equityCurve;
    std::vector<double> periodReturns;

    equityCurve.reserve(data_.size());

    double previousEquity = startCapital_;
    double peakEquity = startCapital_;

    drawDown_ = 0.0;
    turnover_ = 0.0;
    sharpe_ = 0.0;

    for (std::size_t i = 0; i < data_.size(); ++i) {
        double price = data_[i].close;

        if (price <= 0.0) {
            continue;
        }

        Signal signal = strategy.onBar(data_, i);

        if (signal == Signal::Buy && cash >= price && position == 0) {
            int sharesToBuy = static_cast<int>(cash / price);

            cash -= sharesToBuy * price;
            position += sharesToBuy;
            turnover_ += sharesToBuy * price;
        } 
        else if (signal == Signal::Sell && position > 0) {
            cash += position * price;
            turnover_ += position * price;
            position = 0;
        }
        else if (signal == Signal::Exit && position > 0) {
            cash += position * price;
            turnover_ += position * price;
            position = 0;
        }

        double equity = cash + position * price;
        equityCurve.push_back(equity);

        if (previousEquity > 0.0) {
            double r = (equity - previousEquity) / previousEquity;
            periodReturns.push_back(r);
        }

        if (equity > peakEquity) {
            peakEquity = equity;
        }

        double currentDrawdown = (peakEquity - equity) / peakEquity;

        if (currentDrawdown > drawDown_) {
            drawDown_ = currentDrawdown;
        }

        previousEquity = equity;
    }

    endCapital_ = equityCurve.empty() ? startCapital_ : equityCurve.back();
    returns_ = (endCapital_ - startCapital_) / startCapital_;
    turnover_ = turnover_ / startCapital_;

    if (periodReturns.size() > 1) {
        double sum = 0.0;

        for (double r : periodReturns) {
            sum += r;
        }

        double meanReturn = sum / periodReturns.size();

        double variance = 0.0;

        for (double r : periodReturns) {
            double diff = r - meanReturn;
            variance += diff * diff;
        }

        variance /= periodReturns.size();

        double stddev = std::sqrt(variance);

        if (stddev != 0.0) {
            sharpe_ = meanReturn / stddev * std::sqrt(252.0);
        }
    }
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
    std::cout << "Backtest Results\n";
    std::cout << "----------------\n";
    std::cout << "Start Capital: $" << startCapital_ << '\n';
    std::cout << "End Capital: $" << endCapital_ << '\n';
    std::cout << "Return: " << returns_ * 100.0 << "%\n";
    std::cout << "Max Drawdown: " << drawDown_ * 100.0 << "%\n";
    std::cout << "Sharpe Ratio: " << sharpe_ << '\n';
    std::cout << "Turnover: " << turnover_ << '\n';
}

void Backtest::reset() {
    startCapital_ = 0.0;
    endCapital_ = 0.0;
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

    if (std::abs(zScore) < exit_) {
        return Signal::Exit;
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
