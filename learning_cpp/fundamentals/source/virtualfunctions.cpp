#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

class Person {
    public:
        virtual void getdata() = 0;
        virtual void putdata() = 0;
    
    protected:
        string name_;
        int age_;
    
    private:
};

class Professor : public Person {        
    public:
        static int NumberOfCurID;
        
        Professor() {
            NumberOfCurID++;
            cur_id_ = NumberOfCurID;
        }
    
        void getdata() {
            cin >> name_ >> age_ >> publications_;
        }
        
        void putdata() {
            cout << name_ << " " << age_ << " " << publications_ << " " << cur_id_ << endl;
        }
    
    protected:
    
    private:
        int publications_;
        int cur_id_;
};

class Student : public Person {
    public:
        static int NumberOfIDs;
        
        Student() {
            NumberOfIDs++;
            id_ = NumberOfIDs;
        }
    
        void getdata() {
            sumOfMarks = 0;
            cin >> name_ >> age_;
            for (int i = 0; i < NumberOfMarks; i++) {
                cin >> marks_[i];
                sumOfMarks += marks_[i];
            }
        }
        
        void putdata() {
            cout << name_ << " " << age_ << " " << sumOfMarks << " " << id_ << endl;
        }
    
    protected:
        static const int NumberOfMarks = 6;
    
    private:
        int marks_[NumberOfMarks];
        int id_;    
        int sumOfMarks;    
};

int Professor::NumberOfCurID = 0;
int Student::NumberOfIDs = 0;

int main(){

    int n, val;
    cin>>n; //The number of objects that is going to be created.
    Person *per[n];

    for(int i = 0;i < n;i++){

        cin>>val;
        if(val == 1){
            // If val is 1 current object is of type Professor
            per[i] = new Professor;

        }
        else per[i] = new Student; // Else the current object is of type Student

        per[i]->getdata(); // Get the data from the user.

    }

    for(int i=0;i<n;i++)
        per[i]->putdata(); // Print the required output for each object.

    return 0;

}
