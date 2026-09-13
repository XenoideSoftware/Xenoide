
#pragma once 

#include <set>

template<typename T>
class Observer;

template<typename T>
class Subject {
public:
	void attach(Observer<T> *obs) {
		observers.insert(obs);
	}

	void detach(Observer<T> *obs) {
		observers.erase(obs);
	}

	void notify(const T &value) {
		for (Observer<T> *obs : observers) {
			obs->update(value);
		}
	}

private:
	std::set<Observer<T>*> observers;
};

