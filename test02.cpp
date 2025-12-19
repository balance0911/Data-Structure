// main.cpp - 中药库存预警系统入库逻辑修正版
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <conio.h>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <map>
#include <set>

using namespace std;

// medicine.h - 核心数据结构定义
#define MAX_NAME_LEN 50
#define MAX_ORIGIN_LEN 50
#define MAX_SPEC_LEN 20
#define MAX_MEDICINES 300
#define HISTORY_DAYS 7  // 记录7天用量历史

// 药品基本信息结构
typedef struct {
    int id;                     // 药品编号（唯一标识）
    char name[MAX_NAME_LEN];    // 药品名称
    char origin[MAX_ORIGIN_LEN]; // 产地
    char spec[MAX_SPEC_LEN];    // 规格
    int stock;                  // 当前库存量
    int warning_threshold;      // 预警阈值
    int last_usage;             // 最近一次用量
    int usage_history[HISTORY_DAYS]; // 近7天用量历史
    int is_warning;             // 预警状态：0-正常，1-预警
    time_t warning_time;        // 预警开始时间
    time_t response_time;       // 预警响应时间（解除时间）
} Medicine;

// 入库单结构（用于循环队列）
typedef struct {
    int med_id;                 // 药品编号
    int quantity;               // 入库数量
    char operator_name[20];     // 操作员
    time_t in_time;             // 入库时间
    char date[11];              // 入库日期
} InOrder;

// 出库单结构（用于链栈）
typedef struct OutOrderNode {
    int med_id;                 // 药品编号
    int quantity;               // 出库数量
    char prescription_no[30];   // 处方号
    time_t out_time;            // 出库时间
    char date[11];              // 出库日期
    struct OutOrderNode *next;  // 下一节点指针
} OutOrderNode;

// ordered_list.h - 有序顺序表
typedef struct {
    Medicine *medicines;        // 药品数组
    int length;                 // 当前长度
    int capacity;               // 总容量
} OrderedList;

// 创建有序表
OrderedList* createOrderedList(int capacity) {
    OrderedList *list = new OrderedList;
    list->medicines = new Medicine[capacity];
    list->length = 0;
    list->capacity = capacity;
    return list;
}

// 销毁有序表
void destroyOrderedList(OrderedList *list) {
    if (list != nullptr) {
        delete[] list->medicines;
        delete list;
    }
}

// 二分查找药品
Medicine* findMedicine(OrderedList *list, int id) {
    int left = 0, right = list->length - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (list->medicines[mid].id == id) {
            return &list->medicines[mid];
        } else if (list->medicines[mid].id < id) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return nullptr;
}

// 插入药品（保持有序）
int insertMedicine(OrderedList *list, Medicine med) {
    if (list->length >= list->capacity) {
        return -1; // 满了
    }
    
    // 找到插入位置
    int pos = list->length;
    for (int i = 0; i < list->length; i++) {
        if (list->medicines[i].id > med.id) {
            pos = i;
            break;
        }
    }
    
    // 向后移动元素
    for (int i = list->length; i > pos; i--) {
        list->medicines[i] = list->medicines[i-1];
    }
    
    list->medicines[pos] = med;
    list->length++;
    return 0;
}

// 删除药品
int deleteMedicine(OrderedList *list, int id) {
    for (int i = 0; i < list->length; i++) {
        if (list->medicines[i].id == id) {
            // 向前移动元素
            for (int j = i; j < list->length - 1; j++) {
                list->medicines[j] = list->medicines[j+1];
            }
            list->length--;
            return 0;
        }
    }
    return -1; // 未找到
}

// circular_queue.h - 循环队列入库管理
typedef struct {
    InOrder *orders;            // 订单数组
    int front;                  // 队头指针
    int rear;                   // 队尾指针
    int capacity;               // 队列容量
    int size;                   // 当前大小
} CircularQueue;

// 创建循环队列
CircularQueue* createQueue(int capacity) {
    CircularQueue* queue = new CircularQueue;
    queue->orders = new InOrder[capacity];
    queue->front = 0;
    queue->rear = 0;
    queue->size = 0;
    queue->capacity = capacity;
    return queue;
}

// 销毁队列
void destroyQueue(CircularQueue *queue) {
    if (queue != nullptr) {
        delete[] queue->orders;
        delete queue;
    }
}

// 入队
bool enqueue(CircularQueue *queue, InOrder order) {
    if (queue->size == queue->capacity) {
        return false; // 队列满
    }
    queue->orders[queue->rear] = order;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->size++;
    return true;
}

// 出队
bool dequeue(CircularQueue *queue, InOrder *order) {
    if (queue->size == 0) {
        return false; // 队列空
    }
    *order = queue->orders[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return true;
}

// linked_stack.h - 链栈出库管理
class LinkedStack {
private:
    OutOrderNode* top;          // 栈顶指针
    int size;                   // 栈的大小
    
public:
    LinkedStack() : top(nullptr), size(0) {}
    ~LinkedStack() { clear(); }
    
    bool isEmpty() const { return top == nullptr; }
    int getSize() const { return size; }
    
    void push(int med_id, int quantity, const char* prescription_no, const char* date) {
        OutOrderNode* newNode = new OutOrderNode;
        newNode->med_id = med_id;
        newNode->quantity = quantity;
        strcpy(newNode->prescription_no, prescription_no);
        strcpy(newNode->date, date);
        newNode->out_time = time(nullptr);
        newNode->next = top;
        top = newNode;
        size++;
    }
    
    bool pop(OutOrderNode& order) {
        if (top == nullptr) return false;
        
        OutOrderNode* temp = top;
        order = *top;
        top = top->next;
        delete temp;
        size--;
        return true;
    }
    
    void clear() {
        while (top != nullptr) {
            OutOrderNode* temp = top;
            top = top->next;
            delete temp;
            size--;
        }
    }
    
    OutOrderNode* getTop() const { return top; }
    
    // 获取指定日期的出库统计
    int getDailyOutTotal(const string& date) const {
        int total = 0;
        OutOrderNode* current = top;
        while (current != nullptr) {
            if (string(current->date) == date) {
                total += current->quantity;
            }
            current = current->next;
        }
        return total;
    }
    
    // 获取指定药品的出库统计
    int getMedicineOutTotal(int med_id, const string& date) const {
        int total = 0;
        OutOrderNode* current = top;
        while (current != nullptr) {
            if (current->med_id == med_id && string(current->date) == date) {
                total += current->quantity;
            }
            current = current->next;
        }
        return total;
    }
    
    // 获取指定日期的处方数量
    int getPrescriptionCount(const string& date) const {
        int count = 0;
        OutOrderNode* current = top;
        while (current != nullptr) {
            if (string(current->date) == date) {
                count++;
            }
            current = current->next;
        }
        return count;
    }
    
    // 获取指定日期的所有出库单
    vector<OutOrderNode> getDailyOrders(const string& date) const {
        vector<OutOrderNode> orders;
        OutOrderNode* current = top;
        while (current != nullptr) {
            if (string(current->date) == date) {
                orders.push_back(*current);
            }
            current = current->next;
        }
        return orders;
    }
    
    // 获取所有出库单用于统计
    vector<OutOrderNode> getAllOrders() const {
        vector<OutOrderNode> orders;
        OutOrderNode* current = top;
        while (current != nullptr) {
            orders.push_back(*current);
            current = current->next;
        }
        return orders;
    }
};

// warning_logic.h - 预警逻辑
int calculateThreeDayAverage(Medicine *med) {
    if (med == nullptr) return 0;
    
    int sum = 0, count = 0;
    // 前三日用量索引：D-0→6，D-1→5，D-2→4
    int three_days[3] = {
        med->usage_history[6],  // 当天用量
        med->usage_history[5],  // 前一天用量
        med->usage_history[4]   // 前两天用量
    };

    for (int i = 0; i < 3; i++) {
        if (three_days[i] > 0) {
            sum += three_days[i];
            count++;
        }
    }

    return (count == 0) ? 0 : (sum / count);
}

void setWarningThreshold(Medicine *med) {
    if (med == nullptr) return;
    
    int three_day_avg = calculateThreeDayAverage(med);
    int threshold;

    if (three_day_avg > 0) {
        threshold = static_cast<int>(three_day_avg * 0.1);
    } else {
        threshold = static_cast<int>(med->last_usage * 0.1);
    }

    // 确保阈值至少为1，防止出现0阈值导致永远预警
    threshold = max(threshold, 1);
    
    med->warning_threshold = threshold;
}

// 实时检查并更新预警状态
void checkAndHandleWarnings(OrderedList *inventory, int med_id) {
    if (inventory == nullptr) return;
    
    Medicine *med = findMedicine(inventory, med_id);
    if (med == nullptr) return;
    
    time_t now = time(NULL);
    
    // 检查是否触发预警
    if (med->stock < med->warning_threshold && med->is_warning == 0) {
        med->is_warning = 1;
        med->warning_time = now;
        cout << "\n【预警触发】药品ID=" << med->id 
             << "，名称=" << med->name 
             << "，库存=" << med->stock << "g < 阈值=" << med->warning_threshold << "g" << endl;
    }
    // 检查是否解除预警
    else if (med->stock >= med->warning_threshold && med->is_warning == 1) {
        med->is_warning = 0;
        med->response_time = now;
        double response_hour = difftime(med->response_time, med->warning_time) / 3600.0;
        cout << "\n【预警解除】药品ID=" << med->id 
             << "，名称=" << med->name 
             << "，响应时间=" << response_hour << "小时" << endl;
    }
}

// 更新所有药品的预警状态
void updateAllWarnings(OrderedList *inventory) {
    if (inventory == nullptr || inventory->length == 0) {
        cout << "警告：库存表为空，无需更新预警状态！" << endl;
        return;
    }

    cout << "\n===== 开始更新所有药品预警状态 =====" << endl;
    for (int i = 0; i < inventory->length; i++) {
        checkAndHandleWarnings(inventory, inventory->medicines[i].id);
    }
    cout << "===== 预警状态更新完成 =====" << endl;
}

// 处理入库单 - 修正版本：仅处理队列中的订单，不再重复增加库存
bool processInOrder(CircularQueue *queue, OrderedList *inventory) {
    if (queue == nullptr || inventory == nullptr) {
        cout << "错误：参数无效！" << endl;
        return false;
    }
    
    if (queue->size == 0) {
        cout << "提示：没有待处理的入库单" << endl;
        return false;
    }
    
    InOrder order;
    int processed_count = 0;
    while (dequeue(queue, &order)) {
        Medicine *med = findMedicine(inventory, order.med_id);
        if (med != nullptr) {
            // 不再增加库存，因为入库管理中已经增加过了
            // 只做日志记录
            cout << "已处理入库单：药品ID " << order.med_id << "，数量 " << order.quantity 
                 << "，操作员：" << order.operator_name << endl;
            
            // 检查预警状态（如果之前库存不足，入库后可能解除预警）
            checkAndHandleWarnings(inventory, order.med_id);
            processed_count++;
        } else {
            cout << "警告：处理入库单失败，药品ID " << order.med_id << " 不存在" << endl;
        }
    }
    
    cout << "共处理了 " << processed_count << " 个入库单" << endl;
    return processed_count > 0;
}

// 统计功能结构
struct MedicineUsage {
    int med_id;
    string med_name;
    int total_usage;
    int usage_frequency;
    double usage_percentage;
    
    MedicineUsage(int id = 0, const string& name = "")
        : med_id(id), med_name(name), total_usage(0), usage_frequency(0), usage_percentage(0.0) {}
    
    bool operator<(const MedicineUsage& other) const {
        return total_usage > other.total_usage; // 降序排列
    }
};

class Statistics {
private:
    CircularQueue* in_queue;
    LinkedStack* out_stack;
    OrderedList* inventory;
    
public:
    Statistics(OrderedList* inv, CircularQueue* queue = nullptr, LinkedStack* stack = nullptr) 
        : inventory(inv), in_queue(queue), out_stack(stack) {}
    
    void setDataSources(OrderedList* inv, CircularQueue* queue, LinkedStack* stack) {
        inventory = inv;
        in_queue = queue;
        out_stack = stack;
    }
    
    struct DailyStats {
        string date;
        int prescription_count;
        int total_dosage;
        int in_orders_count;
        int out_orders_count;
        double avg_response_time;
        
        DailyStats(const string& d = "") : date(d), prescription_count(0),
            total_dosage(0), in_orders_count(0), out_orders_count(0), avg_response_time(0.0) {}
    };
    
    // 获取每日统计
    DailyStats getDailyStats(const string& date) const {
        DailyStats stats(date);
        
        // 统计处方数量和总剂量
        stats.prescription_count = out_stack->getPrescriptionCount(date);
        stats.total_dosage = out_stack->getDailyOutTotal(date);
        
        // 统计入库单数量
        stats.in_orders_count = 0;
        for (int i = 0; i < in_queue->size; i++) {
            int index = (in_queue->front + i) % in_queue->capacity;
            if (string(in_queue->orders[index].date) == date) {
                stats.in_orders_count++;
            }
        }
        
        // 统计出库单数量
        stats.out_orders_count = out_stack->getPrescriptionCount(date);
        
        // 计算平均响应时间
        stats.avg_response_time = getAvgResponseTime(date);
        
        return stats;
    }
    
    // 获取指定日期的药品使用统计
    vector<MedicineUsage> getMedicineUsage(const string& date) const {
        vector<MedicineUsage> usage_list;
        map<int, int> usage_map; // med_id -> total_usage
        map<int, int> freq_map;  // med_id -> usage_frequency
        
        // 遍历出库单统计使用情况
        OutOrderNode* current = out_stack->getTop();
        while (current != nullptr) {
            if (string(current->date) == date) {
                usage_map[current->med_id] += current->quantity;
                freq_map[current->med_id]++;
            }
            current = current->next;
        }
        
        // 创建药品使用统计
        for (const auto& pair : usage_map) {
            int med_id = pair.first;
            int total_usage = pair.second;
            int frequency = freq_map[med_id];
            
            Medicine* med = findMedicine(inventory, med_id);
            string med_name = (med != nullptr) ? med->name : "未知药品";
            
            MedicineUsage usage(med_id, med_name);
            usage.total_usage = total_usage;
            usage.usage_frequency = frequency;
            usage_list.push_back(usage);
        }
        
        // 按用量排序
        sort(usage_list.begin(), usage_list.end());
        return usage_list;
    }
    
    // 获取近三日用量排名
    vector<MedicineUsage> getUsageRanking(int days = 3) const {
        vector<MedicineUsage> usage_list;
        map<int, int> usage_map; // med_id -> total_usage
        map<int, int> freq_map;  // med_id -> usage_frequency
        
        // 获取最近几天的日期
        vector<string> recent_dates;
        time_t now = time(nullptr);
        for (int i = 0; i < days; i++) {
            time_t day = now - (i * 24 * 3600);
            char buffer[11];
            struct tm* timeinfo = localtime(&day);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
            recent_dates.push_back(buffer);
        }
        
        // 遍历出库单统计近几日使用情况
        OutOrderNode* current = out_stack->getTop();
        while (current != nullptr) {
            string order_date(current->date);
            if (find(recent_dates.begin(), recent_dates.end(), order_date) != recent_dates.end()) {
                usage_map[current->med_id] += current->quantity;
                freq_map[current->med_id]++;
            }
            current = current->next;
        }
        
        // 创建药品使用统计
        for (const auto& pair : usage_map) {
            int med_id = pair.first;
            int total_usage = pair.second;
            int frequency = freq_map[med_id];
            
            Medicine* med = findMedicine(inventory, med_id);
            string med_name = (med != nullptr) ? med->name : "未知药品";
            
            MedicineUsage usage(med_id, med_name);
            usage.total_usage = total_usage;
            usage.usage_frequency = frequency;
            usage_list.push_back(usage);
        }
        
        // 按用量排序
        sort(usage_list.begin(), usage_list.end());
        return usage_list;
    }
    
    // 获取近三日频次排名
    vector<MedicineUsage> getFrequencyRanking(int days = 3) const {
        vector<MedicineUsage> usage_list;
        map<int, int> freq_map;  // med_id -> usage_frequency
        
        // 获取最近几天的日期
        vector<string> recent_dates;
        time_t now = time(nullptr);
        for (int i = 0; i < days; i++) {
            time_t day = now - (i * 24 * 3600);
            char buffer[11];
            struct tm* timeinfo = localtime(&day);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
            recent_dates.push_back(buffer);
        }
        
        // 遍历出库单统计近几日使用频次
        OutOrderNode* current = out_stack->getTop();
        while (current != nullptr) {
            string order_date(current->date);
            if (find(recent_dates.begin(), recent_dates.end(), order_date) != recent_dates.end()) {
                freq_map[current->med_id]++;
            }
            current = current->next;
        }
        
        // 创建药品使用统计
        for (const auto& pair : freq_map) {
            int med_id = pair.first;
            int frequency = pair.second;
            
            Medicine* med = findMedicine(inventory, med_id);
            string med_name = (med != nullptr) ? med->name : "未知药品";
            
            MedicineUsage usage(med_id, med_name);
            usage.total_usage = 0; // 频次统计中不关注用量
            usage.usage_frequency = frequency;
            usage_list.push_back(usage);
        }
        
        // 按频次排序
        sort(usage_list.begin(), usage_list.end(), [](const MedicineUsage& a, const MedicineUsage& b) {
            return a.usage_frequency > b.usage_frequency;
        });
        return usage_list;
    }
    
    // 获取平均预警响应时间
    double getAvgResponseTime(const string& date) const {
        if (inventory == nullptr) return 0.0;
        
        double total_time = 0.0;
        int count = 0;
        
        for (int i = 0; i < inventory->length; i++) {
            Medicine med = inventory->medicines[i];
            if (med.response_time != 0 && med.warning_time != 0 && med.is_warning == 0) {
                double response_time = difftime(med.response_time, med.warning_time) / 3600.0;
                total_time += response_time;
                count++;
            }
        }
        
        return count > 0 ? total_time / count : 0.0;
    }
    
    // 比较近三日用量
    void compareThreeDaysUsage() const {
        cout << "\n========== 近三日药物用量对比 ==========" << endl;
        
        vector<MedicineUsage> usage_ranking = getUsageRanking(3);
        vector<MedicineUsage> freq_ranking = getFrequencyRanking(3);
        
        cout << "\n1. 近三日用量排名（前5名）：" << endl;
        cout << left << setw(4) << "排名" 
             << setw(10) << "药品ID" 
             << setw(15) << "药品名称" 
             << setw(10) << "总用量" << endl;
        cout << string(40, '-') << endl;
        
        for (size_t i = 0; i < min(usage_ranking.size(), size_t(5)); i++) {
            cout << setw(4) << (i + 1)
                 << setw(10) << usage_ranking[i].med_id
                 << setw(15) << usage_ranking[i].med_name
                 << setw(10) << usage_ranking[i].total_usage << endl;
        }
        
        cout << "\n2. 近三日使用频次排名（前5名）：" << endl;
        cout << left << setw(4) << "排名" 
             << setw(10) << "药品ID" 
             << setw(15) << "药品名称" 
             << setw(12) << "使用频次" << endl;
        cout << string(37, '-') << endl;
        
        for (size_t i = 0; i < min(freq_ranking.size(), size_t(5)); i++) {
            cout << setw(4) << (i + 1)
                 << setw(10) << freq_ranking[i].med_id
                 << setw(15) << freq_ranking[i].med_name
                 << setw(12) << freq_ranking[i].usage_frequency << endl;
        }
        
        cout << "\n========================================" << endl;
    }
    
    // 生成每日报表
    void generateDailyReport(const string& date) const {
        cout << "\n==========================================" << endl;
        cout << "          药房日常统计报表" << endl;
        cout << "日期：" << date << endl;
        cout << "==========================================" << endl;
        
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
    
    // 生成库存台账
    void generateInReport(const string& date) const {
        cout << "\n==========================================" << endl;
        cout << "          中药库存当日台账" << endl;
        cout << "日期：" << date << endl;
        cout << "==========================================" << endl;
        
        cout << left << setw(10) << "药品ID" 
             << setw(15) << "药品名称" 
             << setw(12) << "前日结余" 
             << setw(12) << "当日入库" 
             << setw(12) << "当日出库" 
             << setw(12) << "当日结余" << endl;
        cout << string(73, '-') << endl;
        
        int total_daily_in = 0;
        int total_daily_out = 0;
        
        for (int i = 0; i < inventory->length; i++) {
            Medicine med = inventory->medicines[i];
            
            // 计算当日入库量
            int daily_in = 0;
            for (int j = 0; j < in_queue->size; j++) {
                int index = (in_queue->front + j) % in_queue->capacity;
                if (in_queue->orders[index].med_id == med.id && 
                    string(in_queue->orders[index].date) == date) {
                    daily_in += in_queue->orders[index].quantity;
                }
            }
            
            // 计算当日出库量
            int daily_out = out_stack->getMedicineOutTotal(med.id, date);
            
            // 计算前日结余（当前库存 - 当日入库 + 当日出库）
            int prev_balance = med.stock - daily_in + daily_out;
            int current_balance = med.stock;
            
            cout << setw(10) << med.id
                 << setw(15) << med.name
                 << setw(12) << prev_balance
                 << setw(12) << daily_in
                 << setw(12) << daily_out
                 << setw(12) << current_balance << endl;
                 
            total_daily_in += daily_in;
            total_daily_out += daily_out;
        }
        
        cout << string(73, '-') << endl;
        cout << left << setw(10) << "合计"
             << setw(15) << ""
             << setw(12) << ""
             << setw(12) << total_daily_in
             << setw(12) << total_daily_out
             << setw(12) << "" << endl;
        cout << "==========================================" << endl;
    }
};

// medicine_management.h - 药品管理
int validateMedicineID(OrderedList *list, int id) {
    if (id <= 0) {
        cout << "错误：药品ID必须为正整数！" << endl;
        return -1;
    }
    if (findMedicine(list, id) != nullptr) {
        cout << "错误：药品ID=" << id << "已存在，请勿重复添加！" << endl;
        return -2;
    }
    return 0; // 验证通过
}

int validateMedicineInfo(Medicine *med) {
    if (strlen(med->name) == 0 || strlen(med->name) >= MAX_NAME_LEN) {
        cout << "错误：药品名称不能为空且长度不能超过" << MAX_NAME_LEN-1 << "字符！" << endl;
        return -1;
    }
    if (strlen(med->origin) == 0 || strlen(med->origin) >= MAX_ORIGIN_LEN) {
        cout << "错误：产地不能为空且长度不能超过" << MAX_ORIGIN_LEN-1 << "字符！" << endl;
        return -2;
    }
    if (strlen(med->spec) == 0 || strlen(med->spec) >= MAX_SPEC_LEN) {
        cout << "错误：规格不能为空且长度不能超过" << MAX_SPEC_LEN-1 << "字符！" << endl;
        return -3;
    }
    if (med->stock < 0) {
        cout << "错误：库存量不能为负数！" << endl;
        return -4;
    }
    if (med->warning_threshold < 0) {
        cout << "错误：预警阈值不能为负数！" << endl;
        return -5;
    }
    return 0; // 验证通过
}

// 安全读取字符串输入
void safeReadString(char* buffer, int maxSize) {
    cin.getline(buffer, maxSize);
    // 移除换行符
    int len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
}

int addMedicine(OrderedList *list) {
    if (list == nullptr || list->length >= list->capacity) {
        cout << "错误：库存表已满，无法新增药品！" << endl;
        return -1;
    }

    Medicine med = {0};
    cout << "\n===== 新增药品 =====" << endl;

    while (true) {
        cout << "请输入药品ID（正整数）：";
        cin >> med.id;
        cin.ignore(); // 清除缓冲区中的换行符
        if (validateMedicineID(list, med.id) == 0) {
            break;
        }
        cout << "请重新输入！" << endl;
    }

    cout << "请输入药品名称：";
    safeReadString(med.name, MAX_NAME_LEN);
    cout << "请输入药品产地：";
    safeReadString(med.origin, MAX_ORIGIN_LEN);
    cout << "请输入药品规格：";
    safeReadString(med.spec, MAX_SPEC_LEN);
    cout << "请输入初始库存量（g）：";
    cin >> med.stock;
    cin.ignore(); // 清除缓冲区
    cout << "请输入初始预警阈值（g）：";
    cin >> med.warning_threshold;
    cin.ignore(); // 清除缓冲区

    med.last_usage = 0;
    memset(med.usage_history, 0, sizeof(med.usage_history));
    med.is_warning = 0;
    med.warning_time = 0;
    med.response_time = 0;

    if (validateMedicineInfo(&med) != 0) {
        cout << "新增失败：药品信息不合法！" << endl;
        return -2;
    }

    if (insertMedicine(list, med) == 0) {
        cout << "新增成功！药品名称：" << med.name << "（ID:" << med.id << "）" << endl;
        return 0;
    } else {
        cout << "新增失败：未知错误！" << endl;
        return -3;
    }
}

int modifyMedicine(OrderedList *list) {
    if (list == nullptr || list->length == 0) {
        cout << "错误：库存表为空，无药品可修改！" << endl;
        return -1;
    }

    int id;
    cout << "\n===== 修改药品 =====" << endl;
    cout << "请输入要修改的药品ID：";
    cin >> id;
    cin.ignore(); // 清除缓冲区

    Medicine *med = findMedicine(list, id);
    if (med == nullptr) {
        cout << "错误：未找到ID=" << id << "的药品！" << endl;
        return -2;
    }

    cout << "\n当前药品信息：" << endl;
    cout << "ID：" << med->id << "（不可修改）" << endl;
    cout << "名称：" << med->name << endl;
    cout << "产地：" << med->origin << endl;
    cout << "规格：" << med->spec << endl;
    cout << "库存量：" << med->stock << "g" << endl;
    cout << "预警阈值：" << med->warning_threshold << "g" << endl;

    cout << "\n请输入新信息（直接回车保留原信息）：" << endl;

    char input[MAX_NAME_LEN];
    cout << "名称（原：" << med->name << "）：";
    safeReadString(input, MAX_NAME_LEN);
    if (strlen(input) > 0) {
        strncpy(med->name, input, MAX_NAME_LEN-1);
        med->name[MAX_NAME_LEN-1] = '\0';
    }

    cout << "产地（原：" << med->origin << "）：";
    safeReadString(input, MAX_ORIGIN_LEN);
    if (strlen(input) > 0) {
        strncpy(med->origin, input, MAX_ORIGIN_LEN-1);
        med->origin[MAX_ORIGIN_LEN-1] = '\0';
    }

    cout << "规格（原：" << med->spec << "）：";
    safeReadString(input, MAX_SPEC_LEN);
    if (strlen(input) > 0) {
        strncpy(med->spec, input, MAX_SPEC_LEN-1);
        med->spec[MAX_SPEC_LEN-1] = '\0';
    }

    cout << "库存量（原：" << med->stock << "g）：";
    safeReadString(input, 20);
    if (strlen(input) > 0) {
        int new_stock = atoi(input);
        if (new_stock >= 0) {
            med->stock = new_stock;
        } else {
            cout << "警告：库存量不能为负，保留原值！" << endl;
        }
    }

    cout << "预警阈值（原：" << med->warning_threshold << "g）：";
    safeReadString(input, 20);
    if (strlen(input) > 0) {
        int new_threshold = atoi(input);
        if (new_threshold >= 0) {
            med->warning_threshold = new_threshold;
        } else {
            cout << "警告：预警阈值不能为负，保留原值！" << endl;
        }
    }

    if (validateMedicineInfo(med) != 0) {
        cout << "修改失败：部分信息不合法，已自动保留原合法值！" << endl;
        return -3;
    }

    cout << "修改成功！" << endl;
    return 0;
}

int removeMedicine(OrderedList *list) {
    if (list == nullptr || list->length == 0) {
        cout << "错误：库存表为空，无药品可删除！" << endl;
        return -1;
    }

    int id;
    cout << "\n===== 删除药品 =====" << endl;
    cout << "请输入要删除的药品ID：";
    cin >> id;
    cin.ignore(); // 清除缓冲区

    Medicine *med = findMedicine(list, id);
    if (med == nullptr) {
        cout << "错误：未找到ID=" << id << "的药品！" << endl;
        return -2;
    }

    cout << "确认要删除药品：" << med->name << "（ID:" << id << "）吗？（Y/N）：";
    char confirm = _getch();
    cout << endl;

    if (confirm == 'Y' || confirm == 'y') {
        if (deleteMedicine(list, id) == 0) {
            cout << "删除成功！" << endl;
            return 0;
        } else {
            cout << "删除失败：未知错误！" << endl;
            return -3;
        }
    } else {
        cout << "已取消删除！" << endl;
        return -4;
    }
}

void queryMedicine(OrderedList *list) {
    if (list == nullptr || list->length == 0) {
        cout << "错误：库存表为空，无药品可查询！" << endl;
        return;
    }

    int choice;
    cout << "\n===== 查询药品 =====" << endl;
    cout << "1. 按ID精确查询" << endl;
    cout << "2. 按名称模糊查询" << endl;
    cout << "请选择查询方式：";
    cin >> choice;
    cin.ignore(); // 清除缓冲区

    switch (choice) {
        case 1: {
            int id;
            cout << "请输入药品ID：";
            cin >> id;
            cin.ignore(); // 清除缓冲区
            Medicine *med = findMedicine(list, id);
            if (med != nullptr) {
                cout << "\n查询结果：" << endl;
                cout << "ID：" << med->id << endl;
                cout << "名称：" << med->name << endl;
                cout << "产地：" << med->origin << endl;
                cout << "规格：" << med->spec << endl;
                cout << "库存量：" << med->stock << "g" << endl;
                cout << "预警阈值：" << med->warning_threshold << "g" << endl;
                cout << "预警状态：" << (med->is_warning ? "预警中" : "正常") << endl;
            } else {
                cout << "查询结果：未找到ID=" << id << "的药品！" << endl;
            }
            break;
        }

        case 2: {
            char keyword[MAX_NAME_LEN];
            cout << "请输入药品名称关键字：";
            safeReadString(keyword, MAX_NAME_LEN);

            cout << "\n查询结果（名称包含\"" << keyword << "\"的药品）：" << endl;
            cout << "ID\t名称\t\t产地\t\t规格\t\t库存量\t预警状态" << endl;
            cout << "------------------------------------------------------------" << endl;

            int count = 0;
            for (int i = 0; i < list->length; i++) {
                Medicine med = list->medicines[i];
                if (strstr(med.name, keyword) != nullptr) {
                    printf("%d\t%s\t\t%s\t\t%s\t\t%d\t%s\n",
                           med.id, med.name, med.origin, med.spec,
                           med.stock, med.is_warning ? "预警中" : "正常");
                    count++;
                }
            }

            if (count == 0) {
                cout << "无匹配结果！" << endl;
            } else {
                cout << "共找到" << count << "个匹配药品！" << endl;
            }
            break;
        }

        default:
            cout << "错误：无效查询方式！" << endl;
            break;
    }
}

void showAllMedicines(OrderedList *list) {
    if (list == nullptr || list->length == 0) {
        cout << "库存表为空！" << endl;
        return;
    }

    cout << "\n===== 所有药品信息（按ID升序） =====" << endl;
    cout << "ID\t名称\t\t产地\t\t规格\t\t库存量（g）\t预警阈值（g）\t预警状态" << endl;
    cout << "------------------------------------------------------------------------" << endl;

    for (int i = 0; i < list->length; i++) {
        Medicine med = list->medicines[i];
        printf("%d\t%s\t\t%s\t\t%s\t\t%d\t\t%d\t\t%s\n",
               med.id, med.name, med.origin, med.spec,
               med.stock, med.warning_threshold,
               med.is_warning ? "预警中" : "正常");
    }

    cout << "\n合计：" << list->length << "种药品" << endl;
}

void medicineManagementMenu(OrderedList *list) {
    int choice;
    while (true) {
        system("cls");
        cout << "==================== 药品管理模块 ====================" << endl;
        cout << "1. 新增药品" << endl;
        cout << "2. 修改药品信息" << endl;
        cout << "3. 删除药品" << endl;
        cout << "4. 查询药品" << endl;
        cout << "5. 显示所有药品" << endl;
        cout << "0. 返回主菜单" << endl;
        cout << "======================================================" << endl;
        cout << "请选择操作（0-5）：";
        cin >> choice;
        cin.ignore(); // 清除缓冲区

        switch (choice) {
            case 1: addMedicine(list); break;
            case 2: modifyMedicine(list); break;
            case 3: removeMedicine(list); break;
            case 4: queryMedicine(list); break;
            case 5: showAllMedicines(list); break;
            case 0: cout << "返回主菜单！" << endl; return;
            default: cout << "错误：无效操作，请重新选择！" << endl; break;
        }

        cout << "\n按任意键继续..." << endl;
        _getch();
    }
}

// 主菜单
void systemMainMenu(OrderedList *inventory, CircularQueue *inQueue, LinkedStack *outStack) {
    Statistics stats(inventory, inQueue, outStack);
    
    int choice;
    while (true) {
        system("cls");
        cout << "==================== 中药库存预警系统 ====================" << endl;
        cout << "1. 药品管理模块" << endl;
        cout << "2. 库存预警管理" << endl;
        cout << "3. 库存台账管理" << endl;
        cout << "4. 统计报表功能" << endl;
        cout << "5. 入库管理" << endl;
        cout << "6. 出库管理" << endl;
        cout << "7. 处理入库单" << endl;
        cout << "0. 退出系统" << endl;
        cout << "========================================================" << endl;
        cout << "请选择操作（0-7）：";
        cin >> choice;
        cin.ignore(); // 清除缓冲区
        
        switch (choice) {
            case 1: medicineManagementMenu(inventory); break;
            
            case 2: {
                system("cls");
                cout << "==================== 库存预警管理 ====================" << endl;
                cout << "1. 手动检查预警状态" << endl;
                cout << "2. 设置预警阈值" << endl;
                cout << "3. 显示预警药品" << endl;
                cout << "4. 批量更新所有预警" << endl;
                cout << "0. 返回主菜单" << endl;
                cout << "======================================================" << endl;
                
                int subChoice;
                cout << "请选择操作：";
                cin >> subChoice;
                cin.ignore(); // 清除缓冲区
                
                switch (subChoice) {
                    case 1: {
                        int id;
                        cout << "请输入药品ID：";
                        cin >> id;
                        cin.ignore(); // 清除缓冲区
                        
                        Medicine *med = findMedicine(inventory, id);
                        if (med == nullptr) {
                            cout << "错误：未找到该药品！" << endl;
                            break;
                        }
                        
                        // 显示当前状态
                        cout << "\n当前状态：" << endl;
                        cout << "药品ID: " << med->id << endl;
                        cout << "药品名称: " << med->name << endl;
                        cout << "当前库存: " << med->stock << "g" << endl;
                        cout << "预警阈值: " << med->warning_threshold << "g" << endl;
                        cout << "预警状态: " << (med->is_warning ? "预警中" : "正常") << endl;
                        
                        // 执行检查
                        cout << "\n正在检查预警状态..." << endl;
                        checkAndHandleWarnings(inventory, id);
                        
                        // 再次显示状态
                        cout << "\n检查后状态：" << endl;
                        cout << "预警状态: " << (med->is_warning ? "预警中" : "正常") << endl;
                        break;
                    }
                    case 2: {
                        int id;
                        cout << "请输入药品ID：";
                        cin >> id;
                        cin.ignore(); // 清除缓冲区
                        Medicine *med = findMedicine(inventory, id);
                        if (med != nullptr) {
                            setWarningThreshold(med);
                            cout << "预警阈值已更新！新阈值：" << med->warning_threshold << "g" << endl;
                            
                            // 重新检查预警状态
                            checkAndHandleWarnings(inventory, id);
                        } else {
                            cout << "未找到该药品！" << endl;
                        }
                        break;
                    }
                    case 3: {
                        cout << "\n======= 当前预警药品 =======\n";
                        bool found_warning = false;
                        for (int i = 0; i < inventory->length; i++) {
                            if (inventory->medicines[i].is_warning) {
                                cout << "ID: " << inventory->medicines[i].id 
                                     << ", 名称: " << inventory->medicines[i].name
                                     << ", 库存: " << inventory->medicines[i].stock << "g"
                                     << ", 阈值: " << inventory->medicines[i].warning_threshold << "g" << endl;
                                found_warning = true;
                            }
                        }
                        if (!found_warning) {
                            cout << "当前无预警药品！" << endl;
                        }
                        break;
                    }
                    case 4: 
                        updateAllWarnings(inventory);
                        break;
                    case 0: break;
                    default: cout << "无效操作！" << endl;
                }
                break;
            }
            
            case 3: {
                system("cls");
                cout << "==================== 库存台账管理 ====================" << endl;
                char dateStr[11];
                time_t now = time(nullptr);
                strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", localtime(&now));
                
                cout << "当前日期: " << dateStr << endl;
                stats.generateInReport(dateStr);
                break;
            }
            
            case 4: {
                system("cls");
                cout << "==================== 统计报表功能 ====================" << endl;
                cout << "1. 生成日报表" << endl;
                cout << "2. 近三日用量排名" << endl;
                cout << "3. 近三日频次排名" << endl;
                cout << "4. 用量对比分析" << endl;
                cout << "5. 预警响应时间统计" << endl;
                cout << "0. 返回主菜单" << endl;
                cout << "=====================================================" << endl;
                
                int subChoice;
                cout << "请选择操作：";
                cin >> subChoice;
                cin.ignore(); // 清除缓冲区
                
                time_t now = time(nullptr);
                char dateStr[11];
                strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", localtime(&now));
                
                switch (subChoice) {
                    case 1: 
                        stats.generateDailyReport(dateStr);
                        break;
                    case 2: {
                        vector<MedicineUsage> ranking = stats.getUsageRanking(3);
                        cout << "\n======= 近三日用量排名 =======\n";
                        for (size_t i = 0; i < ranking.size(); i++) {
                            cout << (i+1) << ". " << ranking[i].med_name 
                                 << " (ID:" << ranking[i].med_id << ") - 用量:" 
                                 << ranking[i].total_usage << endl;
                        }
                        break;
                    }
                    case 3: {
                        vector<MedicineUsage> ranking = stats.getFrequencyRanking(3);
                        cout << "\n======= 近三日频次排名 =======\n";
                        for (size_t i = 0; i < ranking.size(); i++) {
                            cout << (i+1) << ". " << ranking[i].med_name 
                                 << " (ID:" << ranking[i].med_id << ") - 频次:" 
                                 << ranking[i].usage_frequency << endl;
                        }
                        break;
                    }
                    case 4: 
                        stats.compareThreeDaysUsage();
                        break;
                    case 5: {
                        double avgTime = stats.getAvgResponseTime(dateStr);
                        cout << "\n======= 预警响应时间统计 =======\n";
                        cout << "平均响应时间: " << fixed << setprecision(2) << avgTime << " 小时" << endl;
                        break;
                    }
                    case 0: break;
                    default: cout << "无效操作！" << endl;
                }
                break;
            }
            
            case 5: {
                system("cls");
                cout << "==================== 入库管理 ====================" << endl;
                int medId, quantity;
                char operatorName[20];
                
                cout << "请输入药品ID：";
                cin >> medId;
                cin.ignore(); // 清除缓冲区
                cout << "请输入入库数量：";
                cin >> quantity;
                cin.ignore(); // 清除缓冲区
                cout << "请输入操作员姓名：";
                safeReadString(operatorName, 20);
                
                // 查找药品
                Medicine *med = findMedicine(inventory, medId);
                if (med == nullptr) {
                    cout << "错误：药品ID " << medId << " 不存在！" << endl;
                    break;
                }
                
                // 增加库存
                med->stock += quantity;
                cout << "入库成功！药品ID " << medId << "，数量 " << quantity 
                     << "，当前库存 " << med->stock << endl;
                
                // 实时检查预警状态
                checkAndHandleWarnings(inventory, medId);
                
                // 添加入库单到队列（用于记录和后续处理）
                InOrder order;
                order.med_id = medId;
                order.quantity = quantity;
                strcpy(order.operator_name, operatorName);
                order.in_time = time(nullptr);
                strftime(order.date, sizeof(order.date), "%Y-%m-%d", localtime(&order.in_time));
                
                if (enqueue(inQueue, order)) {
                    cout << "入库单已添加到队列！" << endl;
                } else {
                    cout << "警告：入库单队列已满！" << endl;
                }
                break;
            }
            
            case 6: {
                system("cls");
                cout << "==================== 出库管理 ====================" << endl;
                int medId, quantity;
                char prescriptionNo[30];
                
                cout << "请输入药品ID：";
                cin >> medId;
                cin.ignore(); // 清除缓冲区
                cout << "请输入出库数量：";
                cin >> quantity;
                cin.ignore(); // 清除缓冲区
                cout << "请输入处方号：";
                safeReadString(prescriptionNo, 30);
                
                time_t now = time(nullptr);
                char dateStr[11];
                strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", localtime(&now));
                
                // 先减少库存
                Medicine* med = findMedicine(inventory, medId);
                if (med == nullptr) {
                    cout << "错误：药品ID " << medId << " 不存在！" << endl;
                    break;
                }
                
                if (med->stock < quantity) {
                    cout << "错误：库存不足！当前库存 " << med->stock << "，请求出库 " << quantity << endl;
                    break;
                }
                
                med->stock -= quantity;
                
                // 添加出库单
                outStack->push(medId, quantity, prescriptionNo, dateStr);
                cout << "出库成功！当前库存：" << med->stock << endl;
                
                // 更新用量历史
                if (med->usage_history[6] == 0) { // 如果是当天第一次出库
                    med->usage_history[6] = quantity;
                    med->last_usage = quantity;
                } else {
                    med->usage_history[6] += quantity;
                }
                
                // 实时检查预警状态
                checkAndHandleWarnings(inventory, medId);
                break;
            }
            
            case 7: {
                system("cls");
                cout << "==================== 处理入库单 ====================" << endl;
                cout << "正在处理入库单..." << endl;
                processInOrder(inQueue, inventory);
                cout << "入库单处理完成！" << endl;
                break;
            }
            
            case 0: 
                destroyOrderedList(inventory);
                destroyQueue(inQueue);
                delete outStack;
                exit(0);
            
            default: cout << "无效操作！" << endl;
        }
        
        cout << "\n按任意键返回主菜单...";
        _getch();
    }
}

int main() {
    // 创建系统组件
    OrderedList *inventory = createOrderedList(MAX_MEDICINES);
    CircularQueue *inQueue = createQueue(100);
    LinkedStack *outStack = new LinkedStack();
    
    if (inventory == nullptr || inQueue == nullptr || outStack == nullptr) {
        cout << "错误：创建系统组件失败！" << endl;
        return -1;
    }
    
    // 进入系统主菜单
    systemMainMenu(inventory, inQueue, outStack);
    
    return 0;
}


