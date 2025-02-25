#include <iostream>
#include <random>
#include <cstdlib>
#include <ctime>

#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

///* 
template <class TrafficLightPhase>
TrafficLightPhase MessageQueue<TrafficLightPhase>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    
    // perform queue modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    // remove last vector element from queue
    TrafficLightPhase msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <class TrafficLightPhase>
void MessageQueue<TrafficLightPhase>::send(TrafficLightPhase &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add vector to queue
    _queue.push_back(std::move(msg));
    _cond.notify_one(); // notify client
}
//*/

/* Implementation of class "TrafficLight" */

// /* 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    
    while (true)
    {
        TrafficLightPhase message = _qmsg.receive();
        if (message==TrafficLightPhase::green){
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread 
    // when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    
    // Seed the random number generator
    std::srand(std::time(0));
    // Cycle duration bounds
    double min = 4.0, max = 6.0;
    // Generate a random double between 0 and 1
    double random = (double) std::rand() / RAND_MAX; 
    // Scale the random number to the desired range
    double rand_dt = min + random * (max - min);
    // Init. Time
    auto startTime = std::chrono::high_resolution_clock::now();
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dt_loop;
       
    while(true)
    {
        // Update Time reading:
        endTime = std::chrono::high_resolution_clock::now();
        dt_loop = endTime - startTime;

        if (dt_loop.count() >= rand_dt) {
            if (_currentPhase==TrafficLightPhase::green)
            {
                _currentPhase = TrafficLightPhase::red;
            } else
            {
                _currentPhase = TrafficLightPhase::green;
            } 
            // Send update:
            _qmsg.send(std::move(_currentPhase));
            // Update Time metrics:
            random = (double) std::rand() / RAND_MAX; 
            rand_dt = min + random * (max - min);
            startTime = std::chrono::high_resolution_clock::now();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

}

//*/