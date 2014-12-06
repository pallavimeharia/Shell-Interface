#include <iostream>
#include <unistd.h>
#include <chrono>
using namespace std;
using namespace std::chrono;

int main(){
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();  

    int count=0, max=100;
    while( count < max ){
        t2 = high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        if( duration >= 1000000 ){
            t1 = t2;
            printf("    #timer: %1d seconds passed\n", ++count);
        }
        sleep(0.020);
    }

    return 0;
}
