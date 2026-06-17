#ifndef BACKTEST_HPP
#define BACKTEST_HPP

#include <vector>
#include <string>
#include <cstddef>

struct Bar {
    std::string date;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

enum class Signal {
    Buy,
    Sell,
    Hold,
    Exit
};

class Strategy {
public:
    virtual ~Strategy() = default;

    virtual Signal onBar(const std::vector<Bar>& data, std::size_t i) = 0;
};

class MeanReversionStrategy : public Strategy {
public:
    MeanReversionStrategy(std::size_t lookback, double entry, double exit);

    Signal onBar(const std::vector<Bar>& data, std::size_t i) override;

private:
    std::size_t lookback_;
    double entry_;
    double exit_;
};

class MomentumStrategy : public Strategy {
public:
    MomentumStrategy(std::size_t lookback, double threshold);

    Signal onBar(const std::vector<Bar>& data, std::size_t i) override;

private:
    std::size_t lookback_;
    double threshold_;
};

class BreakoutStrategy : public Strategy {
public:
    BreakoutStrategy(std::size_t lookback);

    Signal onBar(const std::vector<Bar>& data, std::size_t i) override;

private:
    std::size_t lookback_;
};

class Backtest {
public:
    Backtest();

    void readData(const std::string& filename);
    void setCapital(double capital);

    void run(Strategy& strategy);

    double getStartCapital() const;
    double getEndCapital() const;
    double getReturns() const;
    double getDrawDown() const;
    double getSharpe() const;
    double getTurnover() const;

    void print() const;
    void reset();

private:
    std::vector<Bar> data_;

    double startCapital_;
    double endCapital_;
    double returns_;
    double drawDown_;
    double sharpe_;
    double turnover_;
};

#endif // BACKTEST_HPP