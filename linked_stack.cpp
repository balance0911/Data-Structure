// linked_stack.cpp
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include "linked_stack.h"

using namespace std;

// ==================== 构造函数和析构函数 ====================
LinkedStack::LinkedStack() : top(nullptr), size(0) {
    cout << "链栈初始化成功" << endl;
}

LinkedStack::~LinkedStack() {
    clear();
    cout << "链栈已销毁" << endl;
}

// ==================== 基本栈操作 ====================
bool LinkedStack::isEmpty() const {
    return top == nullptr;
}

int LinkedStack::getSize() const {
    return size;
}

// ==================== 栈的核心操作 ====================
void LinkedStack::push(const OutOrderNode& order) {
    // 创建新节点
    OutOrderNode* new_node = new OutOrderNode(order);
    
    // 新节点指向原栈顶
    new_node->next = top;
    
    // 更新栈顶指针
    top = new_node;
    
    // 栈大小增加
    size++;
    
    cout << "出库单入栈成功，当前栈大小: " << size << endl;
}

bool LinkedStack::pop(OutOrderNode& order) {
    if (isEmpty()) {
        cout << "提示：栈为空，无法出栈" << endl;
        return false;
    }
    
    // 获取栈顶节点
    OutOrderNode* temp = top;
    
    // 复制数据
    order.med_id = temp->med_id;
    order.med_name = temp->med_name;
    order.quantity = temp->quantity;
    order.prescription_no = temp->prescription_no;
    order.patient_name = temp->patient_name;
    order.out_time = temp->out_time;
    
    // 更新栈顶指针
    top = top->next;
    
    // 删除原栈顶节点
    delete temp;
    
    // 栈大小减少
    size--;
    
    cout << "出库单出栈成功，当前栈大小: " << size << endl;
    return true;
}

bool LinkedStack::peek(OutOrderNode& order) const {
    if (isEmpty()) {
        return false;
    }
    
    order.med_id = top->med_id;
    order.med_name = top->med_name;
    order.quantity = top->quantity;
    order.prescription_no = top->prescription_no;
    order.patient_name = top->patient_name;
    order.out_time = top->out_time;
    
    return true;
}

// ==================== 出库业务功能 ====================
void LinkedStack::addOutOrder(int med_id, const std::string& med_name, int quantity,
                              const std::string& prescription_no, const std::string& patient_name) {
    // 参数验证
    if (med_id <= 0) {
        cout << "错误：药品ID必须大于0" << endl;
        return;
    }
    
    if (quantity <= 0) {
        cout << "错误：出库数量必须大于0" << endl;
        return;
    }
    
    if (prescription_no.empty()) {
        cout << "错误：处方号不能为空" << endl;
        return;
    }
    
    // 创建出库单节点
    OutOrderNode new_order(med_id, med_name, quantity, prescription_no, patient_name);
    
    // 入栈操作
    push(new_order);
    
    cout << "\n出库单创建成功：" << endl;
    cout << "药品: " << med_name << " (ID: " << med_id << ")" << endl;
    cout << "出库数量: " << quantity << endl;
    cout << "处方号: " << prescription_no << endl;
    cout << "患者: " << patient_name << endl;
    cout << "出库时间: " << timeToString(new_order.out_time) << endl;
}

bool LinkedStack::processOutOrder() {
    if (isEmpty()) {
        cout << "提示：没有待处理的出库单" << endl;
        return false;
    }
    
    OutOrderNode order;
    if (pop(order)) {
        cout << "\n========== 处理出库单 ==========" << endl;
        cout << "药品: " << order.med_name << " (ID: " << order.med_id << ")" << endl;
        cout << "出库数量: " << order.quantity << endl;
        cout << "处方号: " << order.prescription_no << endl;
        cout << "患者: " << order.patient_name << endl;
        cout << "出库时间: " << timeToString(order.out_time) << endl;
        cout << "状态: 已出库" << endl;
        cout << "================================" << endl;
        
        return true;
    }
    
    return false;
}

void LinkedStack::displayAllOutOrders() const {
    if (isEmpty()) {
        cout << "提示：出库栈为空" << endl;
        return;
    }
    
    cout << "\n========== 出库单栈详情 ==========" << endl;
    cout << "栈状态：当前大小 = " << size << endl;
    cout << "说明：栈顶是最新的出库单" << endl;
    cout << "------------------------------------" << endl;
    
    OutOrderNode* current = top;
    int count = 1;
    
    while (current != nullptr) {
        cout << "出库单 " << count << " (距栈顶: " << (count - 1) << "):" << endl;
        cout << "  药品ID: " << current->med_id << endl;
        cout << "  药品名称: " << current->med_name << endl;
        cout << "  出库数量: " << current->quantity << endl;
        cout << "  处方号: " << current->prescription_no << endl;
        cout << "  患者姓名: " << current->patient_name << endl;
        cout << "  出库时间: " << timeToString(current->out_time) << endl;
        cout << "  状态: " << (count == 1 ? "待处理" : "等待处理") << endl;
        cout << "------------------------------------" << endl;
        
        current = current->next;
        count++;
    }
}

// ==================== 出库统计 ====================
int LinkedStack::getTodayOutTotal(const std::string& date) const {
    int total = 0;
    OutOrderNode* current = top;
    
    while (current != nullptr) {
        // 检查日期是否匹配
        string order_date = timeToString(current->out_time).substr(0, 10);
        if (order_date == date) {
            total += current->quantity;
        }
        current = current->next;
    }
    
    return total;
}

int LinkedStack::getMedOutTotal(int med_id, const std::string& date) const {
    int total = 0;
    OutOrderNode* current = top;
    
    while (current != nullptr) {
        string order_date = timeToString(current->out_time).substr(0, 10);
        if (current->med_id == med_id && order_date == date) {
            total += current->quantity;
        }
        current = current->next;
    }
    
    return total;
}

int LinkedStack::getPrescriptionCount(const std::string& date) const {
    int count = 0;
    OutOrderNode* current = top;
    
    while (current != nullptr) {
        string order_date = timeToString(current->out_time).substr(0, 10);
        if (order_date == date) {
            count++;
        }
        current = current->next;
    }
    
    return count;
}

// ==================== 遍历栈 ====================
void LinkedStack::traverse(void (*visit)(OutOrderNode*)) const {
    OutOrderNode* current = top;
    while (current != nullptr) {
        visit(current);
        current = current->next;
    }
}

// ==================== 私有辅助函数 ====================
std::string LinkedStack::timeToString(time_t t) const {
    char buffer[20];
    struct tm* timeinfo = localtime(&t);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return string(buffer);
}

void LinkedStack::clear() {
    while (!isEmpty()) {
        OutOrderNode* temp = top;
        top = top->next;
        delete temp;
        size--;
    }
}
