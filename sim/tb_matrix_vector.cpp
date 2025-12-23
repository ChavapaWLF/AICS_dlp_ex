#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vmatrix_vector_mult_4x4x16.h"

using namespace std;

vluint64_t sim_time = 0;

bool load_config(const string& path, int& vec_size) {
    ifstream f(path);
    if (!f) { return false; }
    string line;
    if (!getline(f, line)) { return false; }
    try {
        vec_size = stoi(line, nullptr, 16);
        return true;
    } catch (...) {
        return false;
    }
}

bool load_16bit_data(const string& path, vector<int16_t>& data) {
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
    int vec_size;
    vector<int16_t> activations, weights;
    vector<int32_t> expected_results, actual_results;

    if (!load_config(data_dir + "/config", vec_size) ||
        !load_16bit_data(data_dir + "/neuron", activations) ||
        !load_16bit_data(data_dir + "/weight", weights) ||
        !load_32bit_results(data_dir + "/result", expected_results)) {
        return 1;
    }
    
    int matrix_size = vec_size * vec_size;
    if (weights.size() < matrix_size || activations.size() < vec_size || expected_results.size() < vec_size) {
        cerr << "ERROR: Insufficient data" << endl;
        return 1;
    }
    if (activations.size() > vec_size) activations.resize(vec_size);
    if (expected_results.size() > vec_size) expected_results.resize(vec_size);
    if (weights.size() > matrix_size) weights.resize(matrix_size);

    cout << "A=[";
    for (size_t i = 0; i < activations.size(); i++) {
        cout << (int)activations[i];
        if (i < activations.size() - 1) cout << ",";
    }
    cout << "] Expected=[";
    for (size_t i = 0; i < expected_results.size(); i++) {
        cout << expected_results[i];
        if (i < expected_results.size() - 1) cout << ",";
    }
    cout << "]";

    Vmatrix_vector_mult_4x4x16* dut = new Vmatrix_vector_mult_4x4x16;
    VerilatedVcdC* wave = nullptr;
    if (argc <= 1) {
        wave = new VerilatedVcdC;
        dut->trace(wave, 99);
        wave->open("wave_matrix_vector.vcd");
    }

    dut->clk = 0;
    dut->reset = 1;
    dut->enable = 0;
    for (int i = 0; i < 4; i++) {
        dut->activations[i] = 0;
        for (int j = 0; j < 4; j++) {
            dut->weights[i][j] = 0;
        }
    }

    // 仿真主循环
    const int CLK_PERIOD = 2;
    bool result_captured = false;

    while (sim_time < 80) {
        if (sim_time % (CLK_PERIOD / 2) == 0) {
            dut->clk = !dut->clk;
        }

        if (sim_time == 5) {
            dut->reset = 0;
        }

        if (sim_time == 9 && dut->clk == 0) {
            // 加载数据
            for (int i = 0; i < vec_size; i++) {
                dut->activations[i] = activations[i];
                for (int j = 0; j < vec_size; j++) {
                    dut->weights[i][j] = weights[i*vec_size + j];
                }
            }
        }

        if (sim_time == 11 && dut->clk == 0) {
            dut->enable = 1;
        }

        if (sim_time == 19 && dut->clk == 0) {
            dut->enable = 0;
        }

        // 捕获结果
        if (dut->clk == 1 && dut->reset == 0 && !result_captured && sim_time > 30) {
            for (int i = 0; i < vec_size; i++) {
                actual_results.push_back(dut->results[i]);
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
    for (int i = 0; i < vec_size; i++) {
        cout << actual_results[i];
        if (i < vec_size - 1) cout << ",";
    }
    bool all_pass = true;
    for (int i = 0; i < vec_size; i++) {
        if (actual_results[i] != expected_results[i]) {
            all_pass = false;
            break;
        }
    }
    cout << "] [" << (all_pass ? "PASS" : "FAIL") << "]" << endl;

    return all_pass ? 0 : 1;
}
