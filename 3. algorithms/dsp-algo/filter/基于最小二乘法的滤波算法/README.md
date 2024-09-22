Ref: https://blog.csdn.net/liyuanbhu/article/details/11119081

### 基于最小二乘法的数据平滑算法

取相邻N个数据点，拟合曲线，然后用曲线上相应位置的数据值作为滤波后的结果。（Savitzky-Golay 滤波器）

例子中含：

3点线性平滑、5点线性平滑、7点线性平滑

5点二次平滑、7点二次平滑

5点三次平滑、7点三次平滑