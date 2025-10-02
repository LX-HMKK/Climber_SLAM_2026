#include <functional>
#include <memory>
#include <string>
#include "serial/serial.h"
#include "rclcpp/rclcpp.hpp"
#include "message_filters/subscriber.h"
#include "message_filters/time_synchronizer.h"
#include "std_msgs/msg/string.hpp"

using namespace std;

class monitor : public rclcpp::Node
{
public:
    monitor() : Node("monitor"), serialPort("/dev/ttyUSB0", 115200, serial::Timeout::simpleTimeout(20000))
    {
        if (serialPort.isOpen())
        {
            RCLCPP_INFO(this->get_logger(), "serial port is open");
        }
        else 
        {
            RCLCPP_ERROR(this->get_logger(), "serial port error");
        }
    }

    void readSerialPort()
    {
        std::vector<uint8_t> receivedata(10); // 假设我们要接收10个字节的数据
        if (serialPort.isOpen())
        {
            size_t bytes_read = serialPort.read(receivedata.data(), receivedata.size());
            RCLCPP_INFO(this->get_logger(), "Bytes read: %zu", bytes_read); // 使用bytes_read变量
            std::string data_str(receivedata.begin(), receivedata.end());
            RCLCPP_INFO(this->get_logger(), "Received data: %s", data_str.c_str());
        }
    }

private:
    serial::Serial serialPort; // 将成员变量声明移至此处
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto monitor_node = std::make_shared<monitor>();
    //rclcpp::spin(monitor_node);
    while(1) // 将 while(1) 改为 while(true)，这是更常见的写法
    {
        monitor_node->readSerialPort();
    }
    rclcpp::shutdown();
    return 0;
}