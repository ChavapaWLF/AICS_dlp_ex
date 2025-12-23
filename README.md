# 深度学习处理器运算器 by Chavi

## 一键测试

```bash
cd sim
./run_all_tests.sh
```

**输出示例**：

```
========== Deep Learning Processor Test Report ==========

[1] Inner Product (4x16-bit):
  Test1: A=[1,2,3,4] W=[1,0,0,0] Expected=1 Actual=1 [PASS]
  Test2: A=[1,1,1,1] W=[1,1,1,1] Expected=4 Actual=4 [PASS]
  Test3: A=[1,2,3,4] W=[2,0,0,0] Expected=2 Actual=2 [PASS]
  Test4: A=[2,3,1,4] W=[1,2,0,0] Expected=8 Actual=8 [PASS]

[2] Matrix-Vector Mult (4x4x16-bit):
  Test1: A=[1,2,3,4] Expected=[1,2,3,4] Actual=[1,2,3,4] [PASS]
  Test2: A=[1,1,1,1] Expected=[4,4,4,4] Actual=[4,4,4,4] [PASS]
  Test3: A=[1,2,3,4] Expected=[2,4,6,8] Actual=[2,4,6,8] [PASS]
  Test4: A=[2,3,1,4] Expected=[8,5,9,8] Actual=[8,5,9,8] [PASS]

[3] Matrix-Matrix Mult (4x4x4x16-bit):
  Test1: Matrix(4x4x4) Expected=[1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4] Actual=[1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4] [PASS]
  Test2: Matrix(4x4x4) Expected=[4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4] Actual=[4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4] [PASS]
  Test3: Matrix(4x4x4) Expected=[2,4,6,8,2,4,6,8,2,4,6,8,2,4,6,8] Actual=[2,4,6,8,2,4,6,8,2,4,6,8,2,4,6,8] [PASS]
  Test4: Matrix(4x4x4) Expected=[8,5,9,8,8,5,9,8,8,5,9,8,8,5,9,8] Actual=[8,5,9,8,8,5,9,8,8,5,9,8,8,5,9,8] [PASS]

==========================================================
Result: 12/12 tests passed
Status: ALL PASS
```

## 手动测试

### 编译

```bash
cd sim
make clean
make all
```

### 单独测试某个运算器

```bash
# 内积运算器 - 测试1
./obj_dir/inner_product_4x16/Vinner_product_4x16 ../data/test1

# 矩阵乘向量 - 测试2
./obj_dir/matrix_vector_mult_4x4x16/Vmatrix_vector_mult_4x4x16 ../data/test2

# 矩阵乘法 - 测试3
./obj_dir/matrix_mult_4x4x4x16/Vmatrix_mult_4x4x4x16 ../data/test3
```

### 生成波形文件

不带参数运行会生成波形文件：

```bash
cd sim
./obj_dir/inner_product_4x16/Vinner_product_4x16
# 生成 wave_inner.vcd

gtkwave wave_inner.vcd &
```

## 测试数据说明

```
data/
├── test1/    # 单位矩阵测试
│   ├── config
│   ├── neuron
│   ├── weight
│   └── result
├── test2/    # 全1矩阵测试
├── test3/    # 对角缩放测试
└── test4/    # 非对称矩阵测试
```


