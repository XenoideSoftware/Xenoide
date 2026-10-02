
#pragma once

template <typename T> class Observer {
public:
    virtual ~Observer() = default;

    virtual void update(const T &value) = 0;
};
