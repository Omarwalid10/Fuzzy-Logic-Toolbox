
#include <iostream>
#include <map>
#include <vector>
#include <queue>

#define endl "\n"
#define ll long long
#define OMAR ios_base::sync_with_stdio(false); cin.tie(0); cout.tie(0);
using namespace std;

//
//struct Rule{
//    string var1;
//    string set1;
//    string oper;
//    string var2;
//    string set2;
//    string outVar;
//    string outSet;
//};

map<string, string> inOut;
map<string, pair<double, double>> intervals;
//this is the variable-->fuzzy set name----->vector of every values like points of lines
map<string, map<string, vector<pair<double, double>>>> fuzzySets;

vector<vector<string>>rules;
//for each input variable
map<string,int>crispValues;
//this map will be made in the fuzzification step
//each variable-->fuzzy set name----->the result get from the value in crispValues
map<string, map<string,double>> fuzzySetsResults;
//in this variable out name --->{fuzzy set Name(to make centeroied),inferenceRules}
map<string,multimap<string,double>>inferenceRules;
//in this variable out name --->{fuzzy set Name,centeroied value}
map<string,multimap<string,double>>centerioed;

//vector<vector<string>>rules;
void fuzzification(){
    fuzzySetsResults.clear();
    double slop,b,x,y;
    for(auto crisp:crispValues){
        x=crisp.second;
        //for each crisp value get its fuzzy sets
        for(auto fuzzySet: fuzzySets[crisp.first]){
            int i=0;
            string setName=fuzzySet.first;
            fuzzySetsResults[crisp.first][setName]=0;
            pair<double,double>point1,point2;
            for(auto point:fuzzySet.second){
                point2=point;
                if(i){
                    if(x>=point1.first&&x<=point2.first){
                        slop=(double)(point1.second-point2.second)/(double)(point1.first-point2.first);
                        b=point1.second-(slop*point1.first);
                        y=slop*x+b;
                        // variable-->fuzzy set name----->the result y
                        fuzzySetsResults[crisp.first][setName]=y;
                    }
                }
                point1=point;
                i++;
            }

        }
    }
    cout<<"Fuzzification => done "<<endl;
}
void inference(){
    //for each rule
    for(auto rule:rules){
        int size=rule.size();
        //1--->{num,0.3} 2-->{and,-1};
        map<int,pair<string,double>>operatorsValues;
        int count=0,i=0,ors=0,ands=0;
        //count%3 --->0 variable
        string prev;
        bool notOp=false;
        string word;
        //-2 to avoid the output
        for (int j = 0; j <size-2; ++j){
            word=rule[j];
            if(word=="not"){
                notOp=true;
                continue;
            }
            if(count%3==1){
                //the set value
                operatorsValues[i]={"num",fuzzySetsResults[prev][word]};
                if(notOp){//for the first set only
                    notOp=false;
                    operatorsValues[i]={"num",1-fuzzySetsResults[prev][word]};
                }

                i++;
            }
            else if(count%3==2){
                //operator
                if(word=="or"||word=="or_not")ors++;
                else ands++;
                operatorsValues[i]={word,-1};
                i++;
            }
            prev=word;
            count++;
            //1-->set 2-->operator
        }
        double val1,val2,res=0;
        //we use it to store every calculated (and) values and the every values of (or) to make maximization
        queue<double>values;
        //start with (and) first calculate every ands values and store it in values queue
        for (int j = 0; j < operatorsValues.size(); ++j) {
            if(operatorsValues[j].first=="and"||operatorsValues[j].first=="and_not"){
                val1=operatorsValues[j-1].second;
                if(operatorsValues[j].first=="and_not"){
                  val2=1-operatorsValues[j+1].second ;
               }
                else{
                  val2=operatorsValues[j+1].second ;
               }
               values.push(min(val1,val2));
                //to if the next is (and) it get the previous val of previous and
               operatorsValues[j+1].second=min(val1,val2);
//               //to not include it again in ors
               operatorsValues[j-1].first="NotNum";
               operatorsValues[j+1].first="NotNum";
            }
        }
        //for ors
        for (int j = 0; j < operatorsValues.size(); ++j) {
            if(operatorsValues[j].first=="or" ||operatorsValues[j].first=="or_not"){
                if(operatorsValues[j-1].first=="num")values.push(operatorsValues[j-1].second);
                if(operatorsValues[j+1].first=="num") {
                    if (operatorsValues[j].first == "or_not") {
                       values.push(1 - operatorsValues[j + 1].second);
                    } else {
                        values.push(operatorsValues[j + 1].second);
                    }
                }
            }
        }
        while(!values.empty()){
            res=max(values.front(),res);
            values.pop();
        }
        //to get the inference result for the output
        // var--->set------>inference res
        inferenceRules[rule[size-2]].insert({rule[size-1],res});


    }
    cout<<"Inference => done "<<endl;
}
void centeroied(){
    for (auto it: fuzzySets) {
        for (auto itt: it.second) {

            double sum=0,i=0;
            for (auto ittt: itt.second) {
                sum+= ittt.first;
                i++;
            }
            centerioed[it.first].insert({itt.first,sum/i});
        }
    }
}
void defuzzification(){
    //to calculate centeroied
 centeroied();
    cout<<"Defuzzification => done"<<endl;
    // weighted average method
    //the first loop get each out variable its rules
    for(auto variableWithSetAndResult:inferenceRules){
        double sumNominator=0,sumDenominator=0,res=0;
        //each set ---------> res infe
        for(auto setResult:variableWithSetAndResult.second)
        {
            //because it is a multimab I get the multi of variableWithSetAndResult.first(out var) and make operations
            for(auto center:centerioed[variableWithSetAndResult.first]){
                if(setResult.first==center.first)sumNominator+=setResult.second*center.second;
            }
            sumDenominator+=setResult.second;
        }
        string predictedSet;
        double diff=100000000.0;
        //to avoid zero in denominator
        if(sumDenominator!=0)res=sumNominator/sumDenominator;
        //to get the closest set of out var
        for(auto center:centerioed[variableWithSetAndResult.first]){
            if(abs(center.second-res)<diff){
                diff=abs(center.second-res);
                predictedSet=center.first;
            }
        }
        cout<<"predict "<<variableWithSetAndResult.first<<" is "<<predictedSet<<" "<<res<<endl;
    }

}

void simulation(){
    cout<<endl<<"Running the simulation...."<<endl;
    fuzzification();
    inference();
    defuzzification();
}


int main() {

    while(true) {
        inOut.clear();intervals.clear();fuzzySets.clear();rules.clear();crispValues.clear();
        cout << "Fuzzy Logic Toolbox"
                "\n==================="
                "\n1- Create a new fuzzy system"
                "\n2- Quit" << endl << endl;
        int q, option;
        cin >> q;
        bool o1 = false, o2 = false, o3 = false;


        string ru;
        if (q == 1) {
            while (true) {
                cout << "Main Menu:"
                        "\n==================="
                        "\n1- Add variables."
                        "\n2- Add fuzzy sets to an existing variable."
                        "\n3- Add rules."
                        "\n4- Run the simulation on crisp values."
                        "\n5- Show the Maps."
                        "\n6- to close this system." << endl << endl;
                cin >> option;
                string variable, io, setName;
                double lower, upper, val;
                if (option == 1) {
                    cout << "Enter the variables name, type (IN/OUT) and range lower upper without () or ,:\n"
                            "(Press x to finish)" << endl << endl;
                    while (true) {
                        cin >> variable;
                        if (variable == "x")break;
                        cin >> io >> lower >> upper;
                        if (intervals.count(variable)) {
                            cout << "!! this variable is exist !!" << endl << endl;
                            continue;
                        }
                        if(io=="IN") {
                            crispValues[variable] = 0;
                        }
                        inOut[variable] = io;
                        intervals[variable] = {lower, upper};
                        o1 = true;
                    }
                } else if (option == 2) {
                    if (!o1) {
                        cout << "!! Enter the variables first !!" << endl << endl;
                        continue;
                    }
                    cout << "Enter the variable name:" << endl;
                    cin >> variable;
                    if (!intervals.count(variable)) {
                        cout << "!! this variable isn't exist !!" << endl << endl;
                        continue;
                    }
                    cout << "Enter the fuzzy set name, type (TRI/TRAP) and values: (Press x to finish)" << endl << endl;
                    while (true) {
                        cin >> setName;
                        if (setName == "x")break;

                        cin >> io;
                        vector<pair<double, double>> v;
                        if (io == "TRI") {
                            for (int i = 0; i < 3; ++i) {
                                cin >> val;
                                if (i == 1)v.push_back({val, 1});
                                else v.push_back({val, 0});
                            }
                        } else if (io == "TRAP") {
                            for (int i = 0; i < 4; ++i) {
                                cin >> val;
                                if (i == 1 || i == 2)v.push_back({val, 1});
                                else v.push_back({val, 0});
                            }
                        } else {
                            cout << "!! Enter TRAP or TRI !!" << endl << endl;
                            continue;
                        }
                        if (fuzzySets[variable].count(setName)) {
                            cout << "!! this set Name is exist !!" << endl << endl;
                            continue;
                        }
                        //set the vector of points to the setname map on the fuzzy set map of each variable
                        fuzzySets[variable][setName] = v;

                        o2 = true;
                    }

                } else if (option == 3) {
                    if (!o2) {
                        cout << "!! Enter the variables or fuzzy set first !!" << endl << endl;
                        continue;
                    }
                    cout<<"Enter the rules in this format: (Press x to finish)\n"
                          "(not) IN_variable set operator IN_variable set => OUT_variable set\n"
                          "use not only for the first variable and use in the others {or,or_not,and,and_not}\n"<<endl;
                    while(true){//for each rule until x
                        vector<string>rule;
                        int counter=0;
                        //I use this counter to check
                        // if 0 we should get variable
                        // if 1 we should get set of previous variable
                        // if 2 we should get operator
                        string word,previous="";cin>>word;
                        if(word=="x")break;
                        else if(word!="not"&&!intervals.count(word)){
                            cout<<"!! you should follow the input requirements"<<endl;
                            break;
                        }
                        else if(word=="not"){
                            rule.push_back(word);
                            //the next word
                            cin>>word;
                        }
                        bool checkError=false;
                        while(word!="=>"){
                            //not var or set or operator
                            if(!(counter%3==0&&intervals.count(word))&&!(counter%3==1&&fuzzySets[previous].count(word))){
                                if(word!="or"&&word!="and"&&word!="or_not"&&word!="and_not"){
                                    checkError=true;
                                }
                            }
                            rule.push_back(word);
                            //to check set
                            previous=word;
                            cin>>word;
                            counter++;
                        }
                        cin>>word;
                        if(intervals.count(word)&&!checkError){
                            rule.push_back(word);
                        }
                        else checkError= true;
                        previous=word;
                        cin>>word;
                        if(fuzzySets[previous].count(word)&&!checkError){
                            rule.push_back(word);
                        }
                        else checkError= true;
                        if(!checkError)rules.push_back(rule);
                        else{
                            cout<<"!!Error you should follow the input requirements"<<endl;
                            break;
                        }
                        o3=true;
                    }


                } else if (option == 4) {
                    if (!o3) {
                        cout << "!! Enter the variables or fuzzy sets or rules first  !!" << endl << endl;
                        continue;
                    }
                    cout<<"Enter the crisp values: "<<endl;
                    for(auto it:crispValues){
                        cout<<it.first<<": ";
                        cin>>val;
                        crispValues[it.first]=val;
                        cout<<endl;
                    }
                    simulation();

                } else if (option == 5) {

                    cout <<endl<< "intervals map" << endl;
                    for (auto it: intervals) {
                        cout << it.first << " " << it.second.first << " " << it.second.second << endl;
                    }
                    cout <<endl<< "fuzzySets map" << endl;
                    for (auto it: fuzzySets) {
                        cout << it.first << " ";
                        for (auto itt: it.second) {
                            cout << itt.first << " " << endl;
                            for (auto ittt: itt.second) {
                                cout << ittt.first << " " << ittt.second << endl;
                            }
                        }
                    }
                    cout <<endl<< "rules vector" << endl;
                    for (auto it: rules) {
                        for(auto itt:it){
                            cout<<itt<<" ";
                        }
                        cout<<endl;

                    }
                    cout<<endl<<"fuzzification step"<<endl;
                    for (auto it: fuzzySetsResults) {
                        cout << it.first << " "<<endl;
                        for (auto itt: it.second) {
                            cout << itt.first << " " <<itt.second<< endl;
                        }
                        cout<<endl;
                    }
                    cout<<endl<<"inference rules"<<endl;
                    for(auto it:inferenceRules){
                        for(auto itt:it.second)
                            cout<<it.first<<" "<<itt.first<<" "<<itt.second<<endl;
                    }
                    cout<<endl<<"centeroied"<<endl;
                    for (auto it: centerioed) {
                        cout << it.first << " "<<endl;
                        for (auto itt: it.second) {
                            cout << itt.first << " " <<itt.second<< endl;
                        }
                    }


                } else if (option == 6) {
                    break;
                } else {
                    cout << "!! Enter valid number from 1-->6 !!" << endl << endl;
                    continue;
                }
            }
        } else if (q == 2) {
            return 0;
        } else {
            cout << "!! Enter valid input 1 or 2 !!" << endl;
        }
    }
}







/*
proj_funding IN 0 100
exp_level IN 0 60
risk OUT 0 100
x


proj_funding high or exp_level expert => risk low
proj_funding high or exp_level expert and exp_level intermediate => risk low
not proj_funding high or exp_level expert => risk high
proj_funding medium and exp_level intermediate => risk normal
proj_funding and exp_level beginner => risk normal
proj_funding low and exp_level beginner => risk high
proj_funding very_low and_not exp_level expert => risk high
x

dirt IN 0 100
fabric IN 0 100
time OUT 0 60
x

dirt

small TRAP 0 0 20 40
medium TRAP 20 40 60 80
large TRAP 60 80 100 100
x

fabric

soft TRAP 0 0 20 40
ordinary  TRAP 20 40 60 80
stiff TRAP 60 80 100 100
x

 time

very_small TRI 0 0 15
small TRI 0 15 30
standard TRI 15 30 45
large TRI 30 45 60
very_large TRI 45 60 60
x

Rules

dirt small and fabric soft => time very_small
dirt medium and fabric ordinary => time standard
dirt small and_not fabric soft or dirt medium and fabric soft => time small
dirt medium and fabric stiff => time large
dirt large and_not fabric soft => time very_large
dirt large and fabric soft => time standard
x

*/



/*

proj_funding IN 0 100
exp_level IN 0 60
duration IN 0 80
risk OUT 0 100
x

*/









