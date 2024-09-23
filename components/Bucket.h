#ifndef BUCKET_H
#define BUCKET_H

#include <chrono>

class Bucket {
private:
    std::chrono::steady_clock::time_point lastCheck;
    double allowance;
    double rate;
    double time;
    bool infinite;

public:
    Bucket(double rate, double time);
    void update();
    bool canSpend(double count);
    void setInfinite(bool value);
    bool isInfinite() const;
    double getRate() const { return rate; }
    double getTime() const { return time; }
};

#endif // BUCKET_H