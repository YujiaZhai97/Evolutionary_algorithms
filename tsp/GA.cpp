// GA.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include<fstream>
#include<sstream>
#include<deque>
#include<string>
#include <cstdlib>
#include <ctime>
#include<set>
#include<time.h>
#include<list>
#include<vector>
#include<random>
#include<iterator>
#include<algorithm>
#define CLOCK_PER_SECOND ((clock_t)1000)
#define popsize 20  //80
#define length 24   //1000
#define variation_rate 0.8
#define mutation_rate 0.1;
using namespace std;

struct Coordinate {
    float x = 0;
    float y = 0;
};

struct Distance {
    float sum = 0;
    deque<Coordinate> route;
};


static Coordinate chrom[popsize][length];
static Coordinate choose[popsize][length];
static Distance longestResult;
static Distance shortestResult;
static float bestDistance;
static deque<Coordinate> bestPath;





template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

vector<int> randomGenerator() {
    vector<int> foo;
    for (int i = 0; i < length; i++) {
        foo.push_back(i);
    }
    vector<int> num;
    set<int> used;
    while (num.size() < length) {
        int r = *select_randomly(foo.begin(), foo.end());
        if (used.count(r) == 0) {
            num.push_back(r);
            used.insert(r);
        }
    }
    return num;

}

vector<int> randomSelect2path(){
    vector<int> foo;
    for (int i = 0; i < popsize; i++) {
        foo.push_back(i);
    }
    vector<int> num;
    set<int> used;
    while (num.size() < 2 ){
        int r = *select_randomly(foo.begin(), foo.end());
        if (used.count(r) == 0) {
            num.push_back(r);
            used.insert(r);
        }
    }
    return num;

}
vector<int> randomSelect100path() {
    vector<int> foo;
    for (int i = 0; i < popsize; i++) {
        foo.push_back(i);
    }
    vector<int> num;
    set<int> used;
    while (num.size() < popsize) {
        int r = *select_randomly(foo.begin(), foo.end());
        if (used.count(r) == 0) {
            num.push_back(r);
            used.insert(r);
        }
    }
    return num;
}

vector<int> randomGenerator2points() {
    vector<int> foo;
    for (int i = 0; i < length; i++) {
        foo.push_back(i);
    }
    vector<int> num;
    set<int> used;
    while (num.size() < 2) {
        int r = *select_randomly(foo.begin(), foo.end());
        if (used.count(r) == 0) {
            num.push_back(r);
            used.insert(r);
        }
    }
    return num;
}

float distance(Coordinate p1, Coordinate p2) {
    float dist;
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    dist = sqrt(pow(dx, 2) + pow(dy, 2));

    return dist;
}

float SumOfPath(deque<Coordinate> points) {
    Coordinate first;
    Coordinate pre;
    Coordinate cur;
    float sum = 0;

    first = points[0];
    pre = first;
    for (int i = 1; i < length; i++) {
        cur = points[i];
        sum += distance(cur, pre);
        pre = cur;
    }

    return sum;
}

Distance RandomSearch(deque<Coordinate> allPoints) {
    Distance result;
    unsigned seed = time(0);
    srand(seed);
    vector<int> nums(length);
    nums = randomGenerator();
    for (int i : nums) {
        result.route.push_back(allPoints[i]);
    }
    Coordinate first = result.route[0];
    Coordinate pre;
    Coordinate now;
    pre = first;
    int count = 1;
    while (count < length) {
        now = result.route[count];
        result.sum += sqrt(pow((pre.x - now.x), 2) + pow((pre.y - now.y), 2));
        pre = now;
        count++;
    }
    result.sum += sqrt(pow((first.x - now.x), 2) + pow((first.y - now.y), 2));
    return result;
};

void init(deque<Coordinate> allPoints) {
    
    unsigned seed = time(0);
    srand(seed);

    for (int j = 0; j < popsize;j++) {
        
        vector<int> nums(length);
        nums = randomGenerator();
        for (int i : nums) {
            cout << i << ", ";
        }
        for (int k = 0; k < length;k++) {
            int i = nums[k];
            chrom[j][k].x = allPoints[i].x;
            chrom[j][k].y = allPoints[i].y;
        }
    }
    for (Coordinate i : chrom[0]) {
        cout << i.x << ", " << i. y << endl;
    }
    
   }

deque<Coordinate> PathSet(int index) {
    deque<Coordinate> path;
    // cout << index << endl; 
    for (int i = 0; i < length;i++) {
        path.push_back(chrom[index][i]);
        //path[i].y = chrom[index][i].y;
    }
    return path;
}
deque<Coordinate> PathSet2(int index) {
    deque<Coordinate> path;
    for (int i = 0; i < length;i++) {
        path.push_back(choose[index][i]);
    }
    return path;
}

void PathPut(Distance path, int index) {
    for (int i = 0; i < length; i++) {
        chrom[index][i] = path.route[i];
    }
}

void PathPut2(Distance path, int index) {
    for (int i = 0; i < length; i++) {
        choose[index][i] = path.route[i];
    }
}


void TournamentSelect(int goal) {
    // randomly get two path
    // keep the desire one
    // till get half number of them and save in choose
    vector<int> paths;
    Distance path1;
    Distance path2;
    for (int i = 0; i < popsize * variation_rate ; i++) {
        paths = randomSelect2path();
        //cout << paths[0] << ", " << paths[1] << endl;
        path1.route = PathSet(paths[0]);
        path2.route = PathSet(paths[1]);
        path1.sum = SumOfPath(path1.route);
        path2.sum = SumOfPath(path2.route);
        switch (goal) {
        case 1:
            // return the shorter path
            if (path1.sum <= path2.sum) {
                PathPut2(path1, i);
                PathPut(path1, i);
            }
            PathPut2(path2, i);
            PathPut(path2, i);
            break;
        case 2:
            // return the longer path
            if (path1.sum >= path2.sum) {
                PathPut2(path1, i);
                PathPut(path1, i);
            }
            PathPut2(path2, i);
            PathPut(path2, i);
            break;
        }
    }
    //cout << "finish tournament" << endl;

    
}

bool myfunction(float i, float j) { return (i < j); }//ascending order
bool myfunction2(float i, float j) { return (i > j); }//descending order

int getIndex(vector<float> v, float K)
{
    auto it = find(v.begin(),
        v.end(), K);

    // If element was found 
    if (it != v.end()) {
        // calculating the index 
        // of K 
        int index = distance(v.begin(),
            it);
        return index;
    }
    else {
        // If the element is not 
        // present in the vector 
        return -1;
    }
}

void PortionSelection(int goal) {
    // calculate the sums of all paths
    // keep the top half of them
    float sum[popsize];
    vector<float> sumRec;
    deque<Coordinate> path;
    // calculate sums of all paths

    //cout << "get here" << endl;
    for (int i = 0; i < popsize; i++) {
        path = PathSet(i);
        sum[i] = SumOfPath(path);
        cout << sum[i] << endl;
        sumRec.push_back(sum[i]);
    }
    //cout << "get here2" << endl;
    if (goal == 1) {
        // sort from small to large
        sort(sum, sum+popsize, myfunction);
    } else {
        // sort from large to small
        sort(sum, sum + popsize, myfunction2);
    }
    //cout << "get here3" << endl;
    Distance result;
    for (int i = 0; i < popsize * variation_rate; i++) {
        //cout << "get here4" << endl;
        int k = getIndex(sumRec, sum[i]);
        //cout << "get here4 " << k<<endl;
        int index = int(k);
        //cout << "get here5" <<index<< endl;
        result.route = PathSet(index);
        PathPut2(result, i);
        PathPut(result, i);
    }
    //cout << "finish proportion" << endl;
}

void select(string sel, string var, string goal) {
    int mycase;
    int mygoal;
    if (sel == "tournament_selection") {
        mycase = 1;
    }
    else { // portion selection
        mycase = 2;
    }

    if (goal == "shortest") {
        mygoal = 1;
    }
    else { // longest
        mygoal = 2;
    }

    switch (mycase)
    {
    case 1:
        //Tornamentselect
        //cout << "start tournament" << endl;
        TournamentSelect(mygoal);

        break;
    case 2:
        // portion_select
        cout << "start proportion" << endl;
        PortionSelection(mygoal);
        break;
        
    }


}



void mutation() {
    // for each path, there are 10% chance to have mutation
    // if mutation, get two random position and swap them
    vector<int> num;
    Distance path;
    Coordinate temp;
    //cout << "start mutation" << endl;
    for (int i = 0; i < popsize; i++) {
        num = randomSelect2path();
        if (num[0] / 100 <= 0.1) {
            // this path will mutate
            path.route = PathSet(i);

            num = randomGenerator2points();
            temp = path.route[num[0]];
            path.route[num[0]] = path.route[num[1]];
            path.route[num[1]] = temp;

            PathPut(path, i);
        }
    }
    //cout << "end mutation" << endl;
}

int myFind(Coordinate point, deque<Coordinate> set) {
    int k=-1;
    for (Coordinate i : set) {
        k++;
        if (i.x == point.x && i.y == point.y) {
            return k;
        }

    }
    return -1;
}

inline bool operator<(const Coordinate& a, const Coordinate& b)
{
    return a.x < b.x;
}

void SinglePoint(int goal) {

    Distance path1;
    Distance path2;
    vector<int> paths;
    vector<int> num;
    int r;
    Coordinate temp1;
    Coordinate temp2;
    vector<Distance> tmpResult;
    set<Coordinate> u1;
    set<Coordinate> u2;
    int n = int(popsize * (1 - variation_rate) / 2);
    for (int i = 0; i < n ; i++) {
        paths = randomSelect2path();
        path1.route = PathSet2(paths[0]%(popsize/2));
        path2.route = PathSet2(paths[1]%(popsize/2));
        //path1.sum = SumOfPath(path1.route);
        //path2.sum = SumOfPath(path2.route);
        num = randomGenerator2points();
        r = num[0];
        
        for (int k = r; k < length;k++) {
            temp1 = path1.route[k];
            temp2 = path2.route[k];
            path1.route[k] = path2.route[k];
            path2.route[k] = temp1;
            u1.insert(temp1);
            u2.insert(temp2);
            if (u1.count(temp2) == 1) {
                u1.erase(temp2);
            }
            if (u2.count(temp1) == 1) {
                u2.erase(temp1);
            }
        }
        vector<Coordinate> k1(u1.begin(),u1.end());
        vector<Coordinate> k2(u2.begin(),u2.end());

        int n = u1.size();
        int k = 0;
        int m = 0;
        for (int i = 0; i < r; i++) {
            if (k < k2.size() && u1.count(path2.route[i]) == 1) {
                path2.route[i] = k2[k];
                k++;
            }

            if (m<k1.size() && u2.count(path1.route[i]) == 1) {
                path1.route[i] = k1[m];
                m++;
            }
        }
            
        PathPut(path1, int(popsize * variation_rate) - 1 + i * 2);
        PathPut(path2, int(popsize * variation_rate) + i * 2);
    }
    //cout << "stop elimination" << endl;
}



void TwoPoint(int goal) {
    Distance path1;
    Distance path2;
    vector<int> paths;
    Coordinate temp1;
    Coordinate temp2;
    set<Coordinate> u1;
    set<Coordinate> u2;

    int r1;
    int r2;
    int swift;
    for (int i = 0; i < popsize * (1 - variation_rate) / 2; i++) {
        paths = randomSelect2path();
        path1.route = PathSet2(paths[0]% (popsize / 2));
        path2.route = PathSet2(paths[1]% (popsize / 2));
        path1.sum = SumOfPath(path1.route);
        path2.sum = SumOfPath(path2.route);
        unsigned seed = time(0);
        srand(seed);
        vector<int> swap(2);
        swap = randomGenerator2points();
        while(abs((swap[0]-swap[1])<10)){
            swap = randomGenerator2points();
        }
        
        r1 = swap[0];
        r2 = swap[1];
        if (r2 < r1) {
            swift = r1;
            r1 = r2;
            r2 = swift;
        }
        for (int k = r1;k <=  r2;k++) {
            temp1 = path1.route[k];
            temp2 = path2.route[k];
            path1.route[k] = path2.route[k];
            path2.route[k] = temp1;
            u1.insert(temp1);
            u2.insert(temp2);
            if (u1.count(temp2) == 1) {
                u1.erase(temp2);
            }
            if (u2.count(temp1) == 1) {
                u2.erase(temp1);
            }


        }

        vector<Coordinate> k1(u1.begin(), u1.end());
        vector<Coordinate> k2(u2.begin(), u2.end());

        int n = u1.size();
        int k = 0;
        int m = 0;
        for (int i = 0; i < r1; i++) {
            if (k < k2.size() && u1.count(path2.route[i]) == 1) {
                path2.route[i] = k2[k];
                k++;
            }

            if (m < k1.size() && u2.count(path1.route[i]) == 1) {
                path1.route[i] = k1[m];
                m++;
            }
        }

        for (int i = r2+1; i < length; i++) {
            if (k < k2.size() && u1.count(path2.route[i]) == 1) {
                path2.route[i] = k2[k];
                k++;
            }

            if (m < k1.size() && u2.count(path1.route[i]) == 1) {
                path1.route[i] = k1[m];
                m++;
            }
        }
        // chongfubufen

        //path1 = eliDuplicate2(path1, path2, r1, r2);
        //path2 = eliDuplicate2(path2, path1, r1, r2);
        
        PathPut(path1, int(popsize * variation_rate) - 1 + i * 2);
        PathPut(path2, int(popsize * variation_rate) + i * 2);
    
    }
}

void crossover(string sel, string var, string goal) {
    int mycase;
    int mygoal;
    if (var == "single") {
        mycase = 1;
    }
    else { // two point
        mycase = 2;
    }

    if (goal == "shortest") {
        mygoal = 1;
    }
    else { // longest
        mygoal = 2;
    }

    switch (mycase)
    {

    case 1:
        // single ponit
        //cout << "start sigle point crossover" << endl;
        SinglePoint(mygoal);

        break;
    case 2:
        // portion_select
        //cout << "start two point crossover" << endl;
        TwoPoint(mygoal);
        break;
    }
}

deque<Coordinate> findBestResult(string goal){

    deque<Coordinate> bestPath;
    deque<Coordinate> curPath;
    bestPath = PathSet(0);

    
    if(goal == "longest"){
        for(int i=1; i<100; i++){
            curPath = PathSet(i);
            if(SumOfPath(curPath) > SumOfPath(bestPath)){
                bestPath = curPath;
            }
        }
    }else{ // goal == "shortest"
        for(int i=1; i<100; i++){
            curPath = PathSet(i);
            if(SumOfPath(curPath) < SumOfPath(bestPath)){
                bestPath = curPath;
            }
        }
    }

    return bestPath;
}

void Process(string sel,string var, string goal, deque<Coordinate> allPoints) {
    ofstream output;
    deque<Coordinate> bestPath;
    deque<Coordinate> tempPath;
    float bestSum;
    deque<float> sums;
    init(allPoints);
    //for (Coordinate i : chrom[0]) {
    //    cout << i.x << ", " << i.y << endl;
    //}
    tempPath = PathSet(0);
    bestPath = tempPath;
    bestSum = SumOfPath(tempPath);
    //cout << "initial success" << endl;
    for (int run = 0; run < 3;run++) { //4 
        cout << "run " << run << endl;
        int iter = 0;

        for (int batch = 0;batch < 20; batch++) {//50

            for (int t = 0; t < 50; t++) {   // 4000
                // select the better part
                cout << "batch " << batch << "t" <<t <<endl;
                select(sel, var, goal); 

                crossover(sel, var, goal);

                mutation();
                // we should record the best path and the best distance
                // as well as all distances
                tempPath = findBestResult(goal);
                sums.push_back(SumOfPath(tempPath));
                if (goal == "shortest") {
                    if (SumOfPath(tempPath) < bestSum) {
                        bestPath = tempPath;
                        bestSum = SumOfPath(tempPath);
                    }
                }
                iter++;
            }
            cout << "batch " << batch << "best result" << SumOfPath(bestPath) << endl;
            output.open("GA_sum_" + sel + "_" + var + "_" + goal + "_" + to_string(run) + ".txt", fstream::app);
            for (float i : sums) {
                output << i << endl;
            }
            output.close();
            sums.clear();

        }
        output.open("GA_path_" + sel + "_" + var + "_" + goal + "_" + to_string(run) + ".txt", fstream::app);
        for (Coordinate i : bestPath) {
            output << i.x << " " << i.y << endl;
        }
        output.close();


    }

}

void Processtry(string sel, string var, string goal, deque<Coordinate> allPoints) {
    ofstream output;
    deque<Coordinate> bestPath;
    deque<Coordinate> tempPath;
    float bestSum;
    deque<float> sums;
    init(allPoints);
    //for (Coordinate i : chrom[0]) {
    //    cout << i.x << ", " << i.y << endl;
    //}
    tempPath = PathSet(0);
    bestPath = tempPath;
    bestSum = SumOfPath(tempPath);
    //cout << "initial success" << endl;
    for (int run = 0; run < 1;run++) { //4 
        cout << "run " << run << endl;
        int iter = 0;

        for (int batch = 0;batch < 1; batch++) {//50

            for (int t = 0; t < 1; t++) {   // 4000
                // select the better part
                cout << "batch " << batch << "t" << t << endl;
                select(sel, var, goal);

                crossover(sel, var, goal);

                mutation();
                // we should record the best path and the best distance
                // as well as all distances
                tempPath = findBestResult(goal);
                sums.push_back(SumOfPath(tempPath));
                if (goal == "shortest") {
                    if (SumOfPath(tempPath) < bestSum) {
                        bestPath = tempPath;
                        bestSum = SumOfPath(tempPath);
                    }
                }
                iter++;
            }
            cout << "batch " << batch << "best result" << SumOfPath(bestPath) << endl;
            output.open("GA_sum_" + sel + "_" + var + "_" + goal + "_" + to_string(run) + ".txt", fstream::app);
            for (float i : sums) {
                output << i << endl;
            }
            output.close();
            sums.clear();

        }
        output.open("GA_path_" + sel + "_" + var + "_" + goal + "_" + to_string(run) + ".txt", fstream::app);
        for (Coordinate i : bestPath) {
            output << i.x << " " << i.y << endl;
        }
        output.close();


    }



}

int main()
{
    //ifstream input("D:\\CppRepository\\GA\\GA\\tsp.txt");
    ifstream input("D:\\CppRepository\\GA\\GA\\circle.txt");
    string str;
    deque<Coordinate> allPoints;
    int i = 0;
    while (getline(input, str)) {
        istringstream in(str);
        Coordinate temp;
        float x, y;
        in >> x >> y;
        temp.x = x;
        temp.y = y;
        allPoints.push_back(temp);
        i++;
    }

    cout << "file read" << endl;
    // cout << allPoints[0].x << allPoints[0].y << endl;
    //for (int i = 1;i < 2;i++) {
    //    Process(i, allPoints);
    //}

    //,"tournament_selection"
    //========================================
    string selection[2] = { "portion_select" };
    string variation[2] = { "single", "two" };
    string goals[2] = { "shortest", "longest" };
    for (string sel : selection) {

        for (string var : variation) {

            for (string goal : goals) {

                //cout << "start process" << endl;
                Process(sel, var, goal, allPoints);
                //Processtry(sel, var, goal, allPoints);


            }


        }
    }
    return 0;

}