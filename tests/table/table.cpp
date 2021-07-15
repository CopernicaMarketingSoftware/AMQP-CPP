#include <iostream>
#include <amqpcpp.h>

using namespace std;
int main()
{
    AMQP::Array x;
    
    x[0] = "abc";
    x[1] = "xyz";
    
    cout << x << endl;
    cout << x[0] << endl;
    cout << x[1] << endl;
    

    return 0;
}

