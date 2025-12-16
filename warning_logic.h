//接口七 预警计算 
#ifndef _WARNING_LOGIC_H
#define _WARNING_LOGIC_H

#include "ordered_list.h"

// 计算前三日均用量（题目要求）
int calculateThreeDayAverage(Medicine *med);

// 设置预警阈值（前三日均值的10%，如果无则用最近一次用量）
void setWarningThreshold(Medicine *med);

// 检查并更新所有药品预警状态
void updateAllWarnings(OrderedList *inventory);

// 获取预警响应时间（从预警开始到解除的时间差）
double getWarningResponseTime(Medicine *med);

// 自动触发预警检查（可设置为定时任务）
void autoCheckWarnings(OrderedList *inventory);

#endif
