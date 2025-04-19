#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

namespace timer {
    typedef long long ll;

    namespace csv {
        inline std::vector<std::string> split(const std::string &s, char delimiter) {
            std::vector<std::string> tokens;
            std::string token;
            std::istringstream tokenStream(s);
            while (std::getline(tokenStream, token, delimiter)) {
                tokens.push_back(token);
            }
            return tokens;
        }

        inline std::string join(const std::vector<std::string> &vec, char delimiter) {
            std::string result;
            for (size_t i = 0; i < vec.size(); ++i) {
                if (i != 0) {
                    result += delimiter;
                }
                result += vec[i];
            }
            return result;
        }
    } // namespace csv


    inline void create_k_means_header() {
        std::ifstream in_file("oncetime/oncetime.csv");
        bool file_exists_and_not_empty = in_file.good() && (in_file.peek() != std::ifstream::traits_type::eof());
        in_file.close();

        std::ios_base::openmode mode = file_exists_and_not_empty ? std::ios::trunc : std::ios::app;
        std::ofstream csv_file("oncetime/oncetime.csv", mode);

        const std::vector<std::string> header = {"Kmeans ID", "Depth", "Construction(us)"};
        const std::string header_line = csv::join(header, ',');
        if (csv_file.is_open()) {
            csv_file << header_line << "\n";
            csv_file.close();
        } else {
            std::cerr << "ERROR::Failed to open CSV file for writing!!!" << std::endl;
        }
    }
    inline void write_k_means_time(int id, int depth, long long time) {
        // 将耗时写入CSV文件
        std::ofstream csv_file("oncetime/oncetime.csv", std::ios::app);
        const std::vector<std::string> row = {std::to_string(id), std::to_string(depth), std::to_string(time)};
        const std::string row_content = csv::join(row, ',');
        if (csv_file.is_open()) {
            csv_file << row_content << std::endl;
            csv_file.close();
        } else {
            std::cerr << "ERROR::Failed to open CSV file for writing!!!" << std::endl;
        }
    }

    inline void time_prefix_sum() {
        const std::string inputFile = "oncetime/oncetime.csv";
        const std::string outputFile = "oncetime/totaltime.csv";
        const std::string inputColumn = "Construction(us)";
        const std::string outputColumn = "Used Time Since Start(us)";

        std::ifstream inFile(inputFile);
        std::ofstream outFile(outputFile);

        if (!inFile.is_open()) { throw std::runtime_error("Error opening input file: " + inputFile); }
        if (!outFile.is_open()) { throw std::runtime_error("Error opening output file: " + outputFile); }

        std::string line;
        bool isHeader = true;
        int targetIndex = -1;
        long long prefixSum = 0;

        while (std::getline(inFile, line)) {
            std::vector<std::string> row = csv::split(line, ',');

            if (isHeader) {
                isHeader = false;
                for (size_t i = 0; i < row.size(); i++) {
                    if (row[i] == inputColumn) {
                        targetIndex = static_cast<int>(i);
                        row[i] = outputColumn;
                    }
                }
                outFile << csv::join(row, ',') << "\n";
                continue;
            }

            if (targetIndex != -1 && targetIndex <static_cast<int>(row.size())) {
                long long value = std::stoll(row[targetIndex]);
                prefixSum += value;
                row[targetIndex] = std::to_string(prefixSum);
            }

            outFile << csv::join(row, ',') << "\n";
        }

        inFile.close();
        outFile.close();
    }
}