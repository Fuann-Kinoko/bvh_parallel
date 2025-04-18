#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

int timePrefixSum() {
    vector<double> data;

    // 读取文件
    ifstream input_file("oncetime/oncetime.csv");
    if (!input_file.is_open()) {
        cerr << "错误：无法打开输入文件" << endl;
        return 1;
    }

    // 解析数据
    string line;
    while (getline(input_file, line)) {
        try {
            data.push_back(stod(line));
        } catch (const invalid_argument&) {
            cerr << "错误：无效数据格式 - " << line << endl;
            input_file.close();
            return 1;
        }
    }
    input_file.close();

    // 计算前缀和
    vector<double> prefix_sums;
    double sum = 0;
    for (const auto& num : data) {
        sum += num;
        prefix_sums.push_back(sum);
    }

    // 输出结果
    ofstream output_file("oncetime/oncetime.csv");
    if (!output_file.is_open()) {
        cerr << "错误：无法创建输出文件" << endl;
        return 1;
    }

    for (const auto& s : prefix_sums) {
        output_file << s << "\n";
    }
    output_file.close();

    return 0;
}