#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vinner_product_4x16.h"

using namespace std;

vluint64_t sim_time = 0;

// 读取16位二进制数据
bool load_16bit_data(const string& path, vector<int16_t>& data) {
    ifstream f(path);
    if (!f) { cerr << "ERROR: Cannot open " << path << endl; return false; }
    
    string line;
    data.clear();
    while (getline(f, line)) {
        if (line.empty() || line[0] == '/') continue;
        size_t comment_pos = line.find("//");
        if (comment_pos != string::npos) line = line.substr(0, comment_pos);
        
        stringstream ss(line);
        string bin;
        if (ss >> bin && bin.size() == 16) {
            try {
                int value = stoi(bin, nullptr, 2);
                data.push_back(static_cast<int16_t>(value));
            } catch (...) {}
        }
    }
    return true;
}

// 读取32位二进制结果
bool load_32bit_result(const string& path, int32_t& result) {
    ifstream f(path);
    if (!f) { cerr << "ERROR: Cannot open " << path << endl; return false; }
    
    string line;
    if (getline(f, line)) {
        size_t comment_pos = line.find("//");
        if (comment_pos != string::npos) line = line.substr(0, comment_pos);
        
        stringstream ss(line);
        string bin;
        if (ss >> bin && bin.size() == 32) {
            try {
                long long value = stoll(bin, nullptr, 2);
                result = static_cast<int32_t>(value);
                return true;
            } catch (...) {}
        }
    }
    return false;
}

int main(int argc, char**argv) {
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);

    // 获取数据目录（从命令行参数或使用默认值）
    string data_dir = (argc > 1) ? string(argv[1]) : "../data";

    // 加载数据
    vector<int16_t> neuron, weight;
    int32_t expected_result;
    
    if (!load_16bit_data(data_dir + "/neuron", neuron) ||
        !load_16bit_data(data_dir + "/weight", weight) ||
        !load_32bit_result(data_dir + "/result", expected_result)) {
        return 1;
    }
    
    if (neuron.size() < 4 || weight.size() < 4) {
        cerr << "ERROR: Insufficient data" << endl;
        return 1;
    }
    if (weight.size() > 4) weight.resize(4);

    // 打印关键信息
    cout << "A=[";
    for (int i = 0; i < 4; i++) {
        cout << (int)neuron[i];
        if (i < 3) cout << ",";
    }
    cout << "] W=[";
    for (int i = 0; i < 4; i++) {
        cout << (int)weight[i];
        if (i < 3) cout << ",";
    }
    cout << "] Expected=" << expected_result;

    // 初始化DUT（禁用波形以加速测试）
    Vinner_product_4x16* dut = new Vinner_product_4x16;
    VerilatedVcdC* wave = nullptr;
    if (argc <= 1) {  // 仅在单次测试时生成波形
        wave = new VerilatedVcdC;
        dut->trace(wave, 99);
        wave->open("wave_inner.vcd");
    }

    // 初始化信号
    dut->clk = 0;
    dut->reset = 1;  // 初始复位
    dut->enable = 0;
    for (int i = 0; i < 4; i++) {
        dut->activations[i] = 0;
        dut->weights[i] = 0;
    }

    // 仿真参数
    const int CLK_PERIOD = 2;  // 完整时钟周期
    int32_t actual_result = 0;
    bool result_captured = false;

    // 仿真循环
    while (sim_time < 60) {
        if (sim_time % (CLK_PERIOD / 2) == 0) {
            dut->clk = !dut->clk;
        }
        if (sim_time == 5) dut->reset = 0;
        if (sim_time == 9 && dut->clk == 0) {
            for (int i = 0; i < 4; i++) {
                dut->activations[i] = neuron[i];
                dut->weights[i] = weight[i];
            }
        }
        if (sim_time == 11 && dut->clk == 0) dut->enable = 1;
        if (sim_time == 15 && dut->clk == 0) dut->enable = 0;
        if (dut->clk == 1 && dut->reset == 0 && !result_captured && sim_time > 25) {
            actual_result = dut->result;
            result_captured = true;
        }
        dut->eval();
        if (wave) wave->dump(sim_time);
        sim_time++;
    }

    // 清理资源
    if (wave) {
        wave->close();
        delete wave;
    }
    delete dut;

    // 结果验证
    cout << " Actual=" << actual_result;
    cout << " [" << (actual_result == expected_result ? "PASS" : "FAIL") << "]" << endl;

    return (actual_result == expected_result) ? 0 : 1;
}
