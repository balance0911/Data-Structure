// linked_stack.h
#ifndef LINKED_STACK_H
#define LINKED_STACK_H

#include <string>
#include <ctime>

// 出库单节点结构（链栈节点）
struct OutOrderNode {
    int med_id;                 // 药品编号
    std::string med_name;       // 药品名称
    int quantity;               // 出库数量
    std::string prescription_no;// 处方号（题目要求：配发处方）
    std::string patient_name;   // 患者姓名
    time_t out_time;            // 出库时间
    
    // 指针域
    OutOrderNode* next;         // 指向下一个节点的指针
    
    // 构造函数
    OutOrderNode(int id = 0, const std::string& name = "", int qty = 0,
                 const std::string& pres_no = "", const std::string& patient = "")
        : med_id(id), med_name(name), quantity(qty), 
          prescription_no(pres_no), patient_name(patient), next(nullptr) {
        out_time = time(nullptr);
    }
};

// 链栈类
class LinkedStack {
private:
    OutOrderNode* top;          // 栈顶指针
    int size;                   // 栈的大小
    
public:
    // 构造函数和析构函数
    LinkedStack();
    ~LinkedStack();
    
    // 基本栈操作
    bool isEmpty() const;
    int getSize() const;
    
    // 栈的核心操作
    void push(const OutOrderNode& order);
    bool pop(OutOrderNode& order);
    bool peek(OutOrderNode& order) const;
    
    // 出库业务功能
    void addOutOrder(int med_id, const std::string& med_name, int quantity,
                     const std::string& prescription_no, const std::string& patient_name);
    bool processOutOrder();  // 处理出库单（出栈）
    void displayAllOutOrders() const;
    
    // 出库统计
    int getTodayOutTotal(const std::string& date) const;
    int getMedOutTotal(int med_id, const std::string& date) const;
    int getPrescriptionCount(const std::string& date) const;
    
    // 获取栈顶元素（用于统计）
    OutOrderNode* getTop() const { return top; }
    
    // 遍历栈（用于统计功能）
    void traverse(void (*visit)(OutOrderNode*)) const;
    
private:
    // 辅助函数
    std::string timeToString(time_t t) const;
    void clear();  // 清空栈
};

#endif // LINKED_STACK_H
