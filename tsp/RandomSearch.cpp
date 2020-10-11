// RandomSearch.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
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
#define CLOCK_PER_SECOND ((clock_t)1000)

using namespace std;

struct Coordinate {
    float x=0;
    float y=0;
};

struct Distance {
    float sum = 0;
    deque<Coordinate> route;
};

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
    for (int i = 0; i < 1000; i++) {
        foo.push_back(i);
    }
        vector<int> num;
    set<int> used;
    while (num.size() < 1000) {
        int r = *select_randomly(foo.begin(), foo.end());
        if (used.count(r) == 0) {
            num.push_back(r);
            used.insert(r);
        }
    }
    return num;
}

Distance RandomSearch(deque<Coordinate> allPoints) {
    Distance result;
    unsigned seed = time(0);
    srand(seed);
    vector<int> nums(1000);
    nums = randomGenerator();
    for (int i : nums) {
        result.route.push_back(allPoints[i]);
    }
    Coordinate first = result.route[0];
    Coordinate pre;
    Coordinate now;
    pre = first;
    int count = 1;
    while ( count < 1000) {
        now = result.route[count];
        result.sum += sqrt(pow((pre.x - now.x), 2) + pow((pre.y - now.y), 2));
        pre = now;
        count++;
    }
    result.sum += sqrt(pow((first.x - now.x), 2) + pow((first.y - now.y), 2));
    return result;
}
    
void Process(int round, deque<Coordinate> allPoints) {
    ofstream output;//daochu text
    int count = 0;
    clock_t start;
    clock_t now;
    float period;
    Distance nowresult;
    Distance bestresult;
    bestresult.sum = numeric_limits<float>::max();
    cout << "start" << round << "round" << endl;
    list<Distance> allresult;
    while (count < 2) {
        period = 0;
        start = clock();
        now = clock();
        cout << "count " << count << endl;
        while (period <= 10) {
            nowresult = RandomSearch(allPoints);
            allresult.push_back(nowresult);
                    
            if (nowresult.sum < bestresult.sum) {
                bestresult = nowresult;
            }
            now = clock();
            period = (float(now - start) / CLOCK_PER_SECOND);
                    }
        output.open("output_" + to_string(round) + ".txt", fstream::app);//fangzhishanchuyuanyoude
        for (Distance i : allresult) {
            output << i.sum << endl;
        }
        output.close();
        count++;
        allresult.clear();
    }
    output.open("path_" + to_string(round) + ".txt", fstream::app);
    for (Coordinate i : bestresult.route) {
        output << i.x << "," << i.y << endl;
    }
    output.close();
}

int main()
{
    ifstream input("D:\\CppRepository\\RandomSearch\\RandomSearch\\tsp.txt");
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
    for (int i = 1;i < 2;i++) {
        Process(i, allPoints);
    }
    return 0;
    
}