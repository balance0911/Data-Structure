// statistics.h
#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>
#include <vector>
#include <algorithm>
#include "circular_queue.h"
#include "linked_stack.h"

// 药品用量统计结构
struct MedicineUsage {
    int med_id;                 // 药品编号
    std::string med_name;       // 药品名称
    int total_usage;            // 总用量
    int usage_frequency;        // 使用频次（开了多少次）
    double usage_percentage;    // 用量占比
    
    MedicineUsage(int id = 0, const std::string& name = "")
        : med_id(id), med_name(name), total_usage(0), usage_frequency(0), usage_percentage(0.0) {}
    
    // 用于排序的比较函数
    bool operator<(const MedicineUsage& other) const {
        return total_usage > other.total_usage; // 降序排列
    }
};

// 统计报表类
class Statistics {
private:
    CircularQueue* in_queue;    // 入库队列
    LinkedStack* out_stack;     // 出库栈
    
public:
    // 构造函数
    Statistics(CircularQueue* queue = nullptr, LinkedStack* stack = nullptr);
    
    // 设置数据源
    void setDataSources(CircularQueue* queue, LinkedStack* stack);
    
    // 每日统计（题目要求：每日配发处方量和剂量）
    struct DailyStats {
        std::string date;               // 统计日期
        int prescription_count;         // 处方数量
        int total_dosage;               // 总剂量
        int in_orders_count;            // 入库单数量
        int out_orders_count;           // 出库单数量
        double avg_response_time;       // 平均响应时间
        
        DailyStats(const std::string& d = "") : date(d), prescription_count(0),
            total_dosage(0), in_orders_count(0), out_orders_count(0), avg_response_time(0.0) {}
    };
    
    // 统计功能
    DailyStats getDailyStats(const std::string& date) const;
    std::vector<MedicineUsage> getMedicineUsage(const std::string& date) const;
    double getAvgResponseTime(const std::string& date) const;
    
    // 排序功能（题目要求：近三日药物用量和使用频次的排序和对比）
    std::vector<MedicineUsage> getUsageRanking(int days = 3) const;
    std::vector<MedicineUsage> getFrequencyRanking(int days = 3) const;
    void compareThreeDaysUsage() const;
    
    // 报表生成
    void generateDailyReport(const std::string& date) const;
    void generateUsageReport(int days = 3) const;
    void generateWarningReport() const;
    
    // 辅助函数
    static bool compareByUsage(const MedicineUsage& a, const MedicineUsage& b);
    static bool compareByFrequency(const MedicineUsage& a, const MedicineUsage& b);
    
private:
    // 私有辅助函数
    std::string getCurrentDate() const;
    std::vector<std::string> getRecentDates(int days) const;
    bool isDateInRange(const std::string& date, int days) const;
};

#endif // STATISTICS_H
