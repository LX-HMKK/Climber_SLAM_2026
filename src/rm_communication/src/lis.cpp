#include <functional>
#include <memory>
#include <string>
#include "serial/serial.h"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std;

class monitor : public rclcpp::Node
{
public:
    monitor() : Node("monitor"), serialPort("/dev/ttyUSB0", 115200, serial::Timeout::simpleTimeout(2000))
    {
        if (serialPort.isOpen())
        {
            RCLCPP_INFO(this->get_logger(), "serial port is open");
        }
        else 
        {
            RCLCPP_ERROR(this->get_logger(), "serial port error");
        }
        this->timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&monitor::readSerialPort, this));
    }

    void readSerialPort()
    {
        std::vector<uint8_t> receivedata(10); // 假设我们要接收10个字节的数据
        if (serialPort.isOpen())
        {
          RCLCPP_INFO(this->get_logger(), "ok");
            size_t bytes_read = serialPort.read(receivedata.data(), receivedata.size());
            if (bytes_read > 0)
            {
                std::string data_str(receivedata.begin(), receivedata.end());
                RCLCPP_INFO(this->get_logger(), "Received data: %s", data_str.c_str());
            }
            else
            {
                RCLCPP_INFO(this->get_logger(), "ok");
            }
        }
    }

private:
    serial::Serial serialPort;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<monitor>());
    rclcpp::shutdown();
    return 0;
}
