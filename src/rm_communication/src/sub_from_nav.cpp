#include "rclcpp/rclcpp.hpp"

class sub_from_nav : public rclcpp::Node
{
public:
    // 构造函数,有一个参数为节点名称
    sub_from_nav(std::string name) : Node(name)
    {
        RCLCPP_INFO(this->get_logger(), "大家好，我是%s.", name.c_str());
    }

private:
    // 声明节点
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    /*创建对应节点的共享指针对象*/
    auto node = std::make_shared<sub_from_nav>("sub_from_nav");
    /* 运行节点，并检测退出信号*/
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
