#include "Bucket.h"

Bucket::Bucket(double rate, double time)
    : lastCheck(std::chrono::steady_clock::now()),
      allowance(rate),
      rate(rate),
      time(time),
      infinite(false) {}

void Bucket::update() {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = now - lastCheck;
    
    allowance += elapsed.count() * (rate / time);
    lastCheck = now;

    if (allowance > rate) {
        allowance = rate;
    }
}

bool Bucket::canSpend(double count) {
    if (infinite) return true;

    update();

    if (allowance < count) return false;

    allowance -= count;

    return true;
}

void Bucket::setInfinite(bool value) {
    infinite = value;
}