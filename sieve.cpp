#include <iostream>
using namespace std;

int main(){
    cout << "\n\n        Sieve of Erasthonese for first 100 integers.";
    cout << "\n        Calculating : ";
    int n, t;

    bool sieve[100];
    for( int k = 0; k<100; k++){
        sieve[k] = true;
    }
    
    sieve[0] = false;   //1 is not prime
    n=1;
    while(n<100){     
        while ( !sieve[n] ){
            n++;
        }
        t = n + 1; //convert index to integer
        if (t <= 100)
            cout << t << " ";
        while( t <= 100 ){
            sieve[t-1] = false;    
            t += n+1;    
        }
        n++;
    }

    cout << "\n\n";

    return 0;
}
