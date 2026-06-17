#include "backtest.hpp"

int main() {
    Backtest bt;

    bt.setCapital(10000.0);
    bt.readData("prices.csv");

    MeanReversionStrategy strategy(20, 2.0, 0.5);

    bt.run(strategy);
    bt.print();

    return 0;
}