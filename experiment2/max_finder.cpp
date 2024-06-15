#include<iostream>
using namespace std;

int main(){
    int n;
    int max=-10000;
    int temp;
    cout<<"pls input the length of the sequence:";
    cin>>n;
    for(int i=0; i<n; i++){
        cin>>temp;
        max = temp>max?temp:max;
    }
    cout<<"the biggest num is:"<<max<<endl;
    return 0;
}