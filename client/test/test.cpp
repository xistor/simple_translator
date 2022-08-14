#include <iostream>
#include "BlockingQueue.h"
#include <thread>
#include <vector>

xistor::BlockingQueue<int> bq;

int main() {


    std::vector<int> vres; // Capture results of consume

    std::thread p = std::thread([](){
        for(int i=0; i<20; i++){
            bq.put(i);
        }
    });

    std::thread c = std::thread([&vres](){
        for(int i=0; i<20; i++){
            //To prove thsi works you can slow down the consumer as follows
            //if((i%5)==0) std::this_thread::sleep_for(std::chrono::milliseconds(1000) );
            //Then return the count in pop so that you see what count is.
            int res; //will just discard
            res = bq.take();
            //vres is not thread safe but we are the only ones using it so OK for demo only
            vres.push_back(res);
        }
    });
    c.join();  // join main tread to this thread so it can finish its work
    p.join();  // join main tread to this thread so it can finish its work
    
    for(int n :vres)
        std::cout << "Consumed " << n << std::endl;
    

    return 0;
}