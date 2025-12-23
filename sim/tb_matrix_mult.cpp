#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vmatrix_mult_4x4x4x16.h"

using namespace std;

vluint64_t sim_time = 0;

// 加载矩阵维度配置
bool load_config(const string& path, int& rowsA, int& colsA, int& colsB) {
    ifstream f(path);
    if (!f) { return false; }
    string line;
    if (!getline(f, line)) { return false; }
    stringstream ss(line);
    string a, b, c;
    if (!(ss >> a >> b >> c)) return false;
    try {
        rowsA = stoi(a, nullptr, 16);
        colsA = stoi(b, nullptr, 16);
        colsB = stoi(c, nullptr, 16);
        return true;
    } catch (...) {
        return false;
    }
}

// 加载16位矩阵数据
bool load_16bit_matrix(const string& path, vector<int16_t>& data) {
    ifstream f(path);
    if (!f) { return false; }
    string line;
    data.clear();
    while (getline(f, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string bin;
        while (ss >> bin) {
            if (bin.size() != 16) return false;
            try {
                uint16_t bits = stoi(bin, nullptr, 2);
                data.push_back(static_cast<int16_t>(bits));
            } catch (...) {
                return false;
            }
        }
    }
    return true;
}

// 加载32位结果数据
bool load_32bit_results(const string& path, vector<int32_t>& results) {
    ifstream f(path);
    if (!f) { return false; }
    string line;
    results.clear();
    while (getline(f, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string bin;
        while (ss >> bin) {
            if (bin.size() != 32) return false;
            try {
                uint32_t bits = stoul(bin, nullptr, 2);
                results.push_back(static_cast<int32_t>(bits));
            } catch (...) {
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char**argv) {
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);

    string data_dir = (argc > 1) ? string(argv[1]) : "../data";
    int rowsA, colsA, colsB;
    vector<int16_t> activations, weights;
    vector<int32_t> expected_results, actual_results;

    if (!load_config(data_dir + "/config", rowsA, colsA, colsB) ||
        !load_16bit_matrix(data_dir + "/neuron", activations) ||
        !load_16bit_matrix(data_dir + "/weight", weights) ||
        !load_32bit_results(data_dir + "/result", expected_results)) {
        return 1;
    }

    if (activations.size() != (size_t)(rowsA * colsA) ||
        weights.size() != (size_t)(colsA * colsB) ||
        expected_results.size() != (size_t)(rowsA * colsB)) {
        cerr << "ERROR: Data size mismatch" << endl;
        return 1;
    }

    cout << "Matrix(" << rowsA << "x" << colsA << "x" << colsB << ") Expected=[";
    for (size_t i = 0; i < expected_results.size(); i++) {
        cout << expected_results[i];
        if (i < expected_results.size() - 1) cout << ",";
    }
    cout << "]";

    Vmatrix_mult_4x4x4x16* dut = new Vmatrix_mult_4x4x4x16;
    VerilatedVcdC* wave = nullptr;
    if (argc <= 1) {
        wave = new VerilatedVcdC;
        dut->trace(wave, 99);
        wave->open("wave_matrix_mult.vcd");
    }

    // 初始化信号
    dut->clk = 0;
    dut->reset = 1;
    dut->enable = 0;

    // 清零输入矩阵
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            dut->activations[i][j] = 0;
            dut->weights[i][j] = 0;
        }
    }

    // 仿真参数
    const int CLK_PERIOD = 2;
    bool result_captured = false;

    // 仿真主循环
    while (sim_time < 120) {
        // 时钟翻转
        if (sim_time % (CLK_PERIOD / 2) == 0) {
            dut->clk = !dut->clk;
        }

        // 释放复位
        if (sim_time == 5) {
            dut->reset = 0;
        }

        // 加载矩阵数据
        if (sim_time == 9 && dut->clk == 0) {
            for (int i = 0; i < rowsA; i++) {
                for (int j = 0; j < colsA; j++) {
                    dut->activations[i][j] = activations[i * colsA + j];
                }
            }
            for (int i = 0; i < colsA; i++) {
                for (int j = 0; j < colsB; j++) {
                    dut->weights[i][j] = weights[i * colsB + j];
                }
            }
        }

        // 启动计算
        if (sim_time == 11 && dut->clk == 0) {
            dut->enable = 1;
        }

        // 关闭使能
        if (sim_time == 19 && dut->clk == 0) {
            dut->enable = 0;
        }

        // 捕获结果
        if (dut->clk == 1 && dut->reset == 0 && !result_captured && sim_time > 60) {
            for (int i = 0; i < rowsA; i++) {
                for (int j = 0; j < colsB; j++) {
                    actual_results.push_back(dut->results[i][j]);
                }
            }
            result_captured = true;
        }

        dut->eval();
        if (wave) wave->dump(sim_time);
        sim_time++;
    }

    if (wave) {
        wave->close();
        delete wave;
    }
    delete dut;

    cout << " Actual=[";
    for (size_t i = 0; i < actual_results.size(); i++) {
        cout << actual_results[i];
        if (i < actual_results.size() - 1) cout << ",";
    }
    bool all_pass = true;
    for (size_t i = 0; i < actual_results.size(); i++) {
        if (actual_results[i] != expected_results[i]) {
            all_pass = false;
            break;
        }
    }
    cout << "] [" << (all_pass ? "PASS" : "FAIL") << "]" << endl;
    return all_pass ? 0 : 1;
}
