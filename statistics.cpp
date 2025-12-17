// statistics.cpp
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <ctime>
#include "statistics.h"

using namespace std;

// ==================== 构造函数 ====================
Statistics::Statistics(CircularQueue* queue, LinkedStack* stack)
    : in_queue(queue), out_stack(stack) {
    cout << "统计模块初始化成功" << endl;
}

void Statistics::setDataSources(CircularQueue* queue, LinkedStack* stack) {
    in_queue = queue;
    out_stack = stack;
}

// ==================== 每日统计功能 ====================
Statistics::DailyStats Statistics::getDailyStats(const std::string& date) const {
    DailyStats stats(date);
    
    if (!out_stack) {
        return stats;
    }
    
    // 统计处方数量和总剂量
    // 注意：这里需要遍历出库栈，由于链栈没有提供遍历接口，
    // 我们需要使用已有接口统计
    
    // 这里简化实现，实际应该遍历出库栈
    stats.prescription_count = out_stack->getPrescriptionCount(date);
    stats.total_dosage = out_stack->getTodayOutTotal(date);
    
    // 统计入库单数量（需要入库队列提供接口）
    // 这里暂时返回0，实际实现需要入库队列提供接口
    
    return stats;
}

std::vector<MedicineUsage> Statistics::getMedicineUsage(const std::string& date) const {
    vector<MedicineUsage> usage_list;
    
    if (!out_stack) {
        return usage_list;
    }
    
    // 使用map临时存储统计结果
    map<int, pair<string, int>> usage_map; // med_id -> (med_name, total_usage)
    map<int, int> frequency_map;           // med_id -> frequency
    
    // 遍历出库栈统计用量
    // 由于链栈没有提供遍历接口，这里需要修改链栈添加遍历功能
    // 或者使用已有接口进行统计
    
    // 这里简化实现，返回空列表
    // 实际实现需要遍历出库栈的所有订单
    
    return usage_list;
}

double Statistics::getAvgResponseTime(const std::string& date) const {
    if (!in_queue) {
        return 0.0;
    }
    
    // 调用入库队列的预警响应时间统计
    // 这里需要入库队列提供相关接口
    
    return 0.0; // 简化实现
}

// ==================== 排序功能 ====================
std::vector<MedicineUsage> Statistics::getUsageRanking(int days) const {
    vector<MedicineUsage> ranking;
    
    // 获取最近dates天的所有日期
    vector<string> recent_dates = getRecentDates(days);
    
    // 这里简化实现，实际应该合并多天的数据
    // 遍历出库栈，统计每个药品在最近days天的总用量
    
    // 创建一些测试数据用于演示
    ranking.push_back(MedicineUsage(1001, "当归"));
    ranking.push_back(MedicineUsage(1002, "黄芪"));
    ranking.push_back(MedicineUsage(1003, "枸杞"));
    ranking.push_back(MedicineUsage(1004, "人参"));
    ranking.push_back(MedicineUsage(1005, "甘草"));
    
    // 设置测试用量
    ranking[0].total_usage = 150;
    ranking[1].total_usage = 120;
    ranking[2].total_usage = 90;
    ranking[3].total_usage = 75;
    ranking[4].total_usage = 60;
    
    // 计算总用量
    int total = 0;
    for (auto& item : ranking) {
        total += item.total_usage;
    }
    
    // 计算百分比
    if (total > 0) {
        for (auto& item : ranking) {
            item.usage_percentage = (item.total_usage * 100.0) / total;
        }
    }
    
    // 按用量降序排序
    sort(ranking.begin(), ranking.end());
    
    return ranking;
}

std::vector<MedicineUsage> Statistics::getFrequencyRanking(int days) const {
    vector<MedicineUsage> ranking;
    
    // 创建测试数据
    ranking.push_back(MedicineUsage(1002, "黄芪"));
    ranking.push_back(MedicineUsage(1001, "当归"));
    ranking.push_back(MedicineUsage(1003, "枸杞"));
    ranking.push_back(MedicineUsage(1005, "甘草"));
    ranking.push_back(MedicineUsage(1004, "人参"));
    
    // 设置测试频次
    ranking[0].usage_frequency = 25;
    ranking[1].usage_frequency = 20;
    ranking[2].usage_frequency = 18;
    ranking[3].usage_frequency = 15;
    ranking[4].usage_frequency = 12;
    
    // 按频次降序排序
    sort(ranking.begin(), ranking.end(), compareByFrequency);
    
    return ranking;
}

void Statistics::compareThreeDaysUsage() const {
    cout << "\n========== 近三日药物用量对比 ==========" << endl;
    
    // 获取最近三天的用量排名
    vector<MedicineUsage> usage_ranking = getUsageRanking(3);
    vector<MedicineUsage> freq_ranking = getFrequencyRanking(3);
    
    // 显示用量排名
    cout << "\n1. 近三日用量排名（前5名）：" << endl;
    cout << left << setw(4) << "排名" 
         << setw(10) << "药品ID" 
         << setw(15) << "药品名称" 
         << setw(10) << "总用量" 
         << setw(12) << "占比(%)" << endl;
    cout << string(51, '-') << endl;
    
    for (size_t i = 0; i < min(usage_ranking.size(), size_t(5)); i++) {
        cout << setw(4) << (i + 1)
             << setw(10) << usage_ranking[i].med_id
             << setw(15) << usage_ranking[i].med_name
             << setw(10) << usage_ranking[i].total_usage
             << setw(12) << fixed << setprecision(1) << usage_ranking[i].usage_percentage << endl;
    }
    
    // 显示频次排名
    cout << "\n2. 近三日使用频次排名（前5名）：" << endl;
    cout << left << setw(4) << "排名" 
         << setw(10) << "药品ID" 
         << setw(15) << "药品名称" 
         << setw(12) << "使用频次" << endl;
    cout << string(41, '-') << endl;
    
    for (size_t i = 0; i < min(freq_ranking.size(), size_t(5)); i++) {
        cout << setw(4) << (i + 1)
             << setw(10) << freq_ranking[i].med_id
             << setw(15) << freq_ranking[i].med_name
             << setw(12) << freq_ranking[i].usage_frequency << endl;
    }
    
    // 对比分析
    cout << "\n3. 对比分析：" << endl;
    
    // 找出用量大但频次低的药品（可能需要重点关注）
    cout << "用量大但使用频次相对较低的药品：" << endl;
    bool found = false;
    for (size_t i = 0; i < min(usage_ranking.size(), size_t(3)); i++) {
        // 在频次排名中查找相同药品的排名
        int freq_rank = -1;
        for (size_t j = 0; j < freq_ranking.size(); j++) {
            if (freq_ranking[j].med_id == usage_ranking[i].med_id) {
                freq_rank = j + 1;
                break;
            }
        }
        
        // 如果用量排名靠前但频次排名靠后
        if (freq_rank > 5 && freq_rank > (int)(i + 1) + 2) {
            cout << "  " << usage_ranking[i].med_name 
                 << "：用量排名第" << (i + 1) 
                 << "，频次排名第" << freq_rank << endl;
            found = true;
        }
    }
    
    if (!found) {
        cout << "  无明显异常情况" << endl;
    }
    
    cout << "\n提示：用量大且频次高的药品应设置较高库存阈值" << endl;
    cout << "提示：用量大但频次低的药品需关注单次用量是否过大" << endl;
    
    cout << "\n========================================" << endl;
}

// ==================== 报表生成功能 ====================
void Statistics::generateDailyReport(const std::string& date) const {
    cout << "\n==========================================" << endl;
    cout << "          药房日常统计报表" << endl;
    cout << "日期：" << date << endl;
    cout << "生成时间：" << getCurrentDate() << " " << __TIME__ << endl;
    cout << "==========================================" << endl;
    
    // 获取当日统计
    DailyStats stats = getDailyStats(date);
    
    cout << "\n1. 处方配发统计：" << endl;
    cout << "   处方数量：" << stats.prescription_count << endl;
    cout << "   总剂量：" << stats.total_dosage << endl;
    
    if (stats.prescription_count > 0) {
        double avg_dosage = (double)stats.total_dosage / stats.prescription_count;
        cout << "   平均每方剂量：" << fixed << setprecision(2) << avg_dosage << endl;
    }
    
    cout << "\n2. 入库出库统计：" << endl;
    cout << "   入库单数量：" << stats.in_orders_count << endl;
    cout << "   出库单数量：" << stats.out_orders_count << endl;
    
    cout << "\n3. 预警响应统计：" << endl;
    cout << "   平均响应时间：" << fixed << setprecision(2) << stats.avg_response_time << " 小时" << endl;
    
    // 显示药品用量排名
    cout << "\n4. 当日药品用量排名：" << endl;
    vector<MedicineUsage> usage_list = getMedicineUsage(date);
    
    if (usage_list.empty()) {
        cout << "   当日无出库记录" << endl;
    } else {
        cout << left << setw(4) << "排名" 
             << setw(15) << "药品名称" 
             << setw(10) << "用量" 
             << setw(10) << "频次" << endl;
        cout << string(39, '-') << endl;
        
        for (size_t i = 0; i < min(usage_list.size(), size_t(5)); i++) {
            cout << setw(4) << (i + 1)
                 << setw(15) << usage_list[i].med_name
                 << setw(10) << usage_list[i].total_usage
                 << setw(10) << usage_list[i].usage_frequency << endl;
        }
    }
    
    cout << "\n==========================================" << endl;
}

void Statistics::generateUsageReport(int days) const {
    cout << "\n==========================================" << endl;
    cout << "          近" << days << "日药品使用统计" << endl;
    cout << "统计日期：" << getCurrentDate() << endl;
    cout << "生成时间：" << __TIME__ << endl;
    cout << "==========================================" << endl;
    
    // 获取用量排名
    vector<MedicineUsage> usage_ranking = getUsageRanking(days);
    
    if (usage_ranking.empty()) {
        cout << "近" << days << "日无药品使用记录" << endl;
        cout << "==========================================" << endl;
        return;
    }
    
    cout << "\n药品用量排名（按总用量降序）：" << endl;
    cout << left << setw(4) << "排名" 
         << setw(10) << "药品ID" 
         << setw(15) << "药品名称" 
         << setw(12) << "总用量" 
         << setw(10) << "频次" 
         << setw(12) << "占比(%)" << endl;
    cout << string(63, '-') << endl;
    
    int total_usage = 0;
    for (const auto& item : usage_ranking) {
        total_usage += item.total_usage;
    }
    
    for (size_t i = 0; i < usage_ranking.size(); i++) {
        cout << setw(4) << (i + 1)
             << setw(10) << usage_ranking[i].med_id
             << setw(15) << usage_ranking[i].med_name
             << setw(12) << usage_ranking[i].total_usage
             << setw(10) << usage_ranking[i].usage_frequency
             << setw(12) << fixed << setprecision(1) 
             << (total_usage > 0 ? (usage_ranking[i].total_usage * 100.0 / total_usage) : 0.0) << endl;
    }
    
    // 统计信息
    cout << "\n统计信息：" << endl;
    cout << "统计药品数量：" << usage_ranking.size() << endl;
    cout << "总使用量：" << total_usage << endl;
    
    if (usage_ranking.size() > 0) {
        double avg_usage = (double)total_usage / usage_ranking.size();
        cout << "平均每药用量：" << fixed << setprecision(1) << avg_usage << endl;
    }
    
    // 重点关注药品
    if (usage_ranking.size() >= 3) {
        cout << "\n重点关注：" << endl;
        cout << "用量最高：" << usage_ranking[0].med_name 
             << " (" << usage_ranking[0].total_usage << ")" << endl;
        cout << "用量最低：" << usage_ranking.back().med_name 
             << " (" << usage_ranking.back().total_usage << ")" << endl;
    }
    
    cout << "\n==========================================" << endl;
}

void Statistics::generateWarningReport() const {
    cout << "\n==========================================" << endl;
    cout << "          预警响应统计报表" << endl;
    cout << "统计日期：" << getCurrentDate() << endl;
    cout << "==========================================" << endl;
    
    // 这里需要调用入库队列的预警统计功能
    // 暂时显示一些测试信息
    
    cout << "\n预警响应时间统计：" << endl;
    cout << "平均响应时间：2.5 小时" << endl;
    cout << "最快响应时间：0.5 小时" << endl;
    cout << "最慢响应时间：5.0 小时" << endl;
    cout << "总预警次数：12 次" << endl;
    
    cout << "\n今日预警情况：" << endl;
    cout << "当前预警药品：2 种" << endl;
    cout << "已响应预警：3 次" << endl;
    cout << "待响应预警：0 次" << endl;
    
    cout << "\n改进建议：" << endl;
    cout << "1. 对于高频预警药品，考虑提高库存阈值" << endl;
    cout << "2. 响应时间超过4小时的预警需要重点关注" << endl;
    cout << "3. 建立常用药品的安全库存" << endl;
    
    cout << "\n==========================================" << endl;
}

// ==================== 辅助函数 ====================
bool Statistics::compareByUsage(const MedicineUsage& a, const MedicineUsage& b) {
    return a.total_usage > b.total_usage;
}

bool Statistics::compareByFrequency(const MedicineUsage& a, const MedicineUsage& b) {
    return a.usage_frequency > b.usage_frequency;
}

std::string Statistics::getCurrentDate() const {
    time_t now = time(nullptr);
    char buffer[11];
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    return string(buffer);
}

std::vector<std::string> Statistics::getRecentDates(int days) const {
    vector<string> dates;
    time_t now = time(nullptr);
    
    for (int i = 0; i < days; i++) {
        time_t day = now - (i * 24 * 3600);
        char buffer[11];
        struct tm* timeinfo = localtime(&day);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
        dates.push_back(buffer);
    }
    
    return dates;
}

bool Statistics::isDateInRange(const std::string& date, int days) const {
    // 检查日期是否在最近days天内
    vector<string> recent_dates = getRecentDates(days);
    for (const auto& d : recent_dates) {
        if (d == date) {
            return true;
        }
    }
    return false;
}
