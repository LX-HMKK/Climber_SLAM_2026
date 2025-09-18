#include "rclcpp/rclcpp.hpp"
#include "serial/serial.h"
#include <iostream>
#include <string>
#include "std_msgs/msg/string.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using namespace std;
using std::placeholders::_1;

#define Uart_timeOut 100  // 读取串口等待时间
#define CMD_BUFF_LEN 10

// 全局变量定义
uint8_t mode = 0;   // 运动模式
serial::Serial sp;  // 串口对象
serial::Timeout to = serial::Timeout::simpleTimeout(Uart_timeOut);
int Baudrate = 115200;   // 波特率
string usbType = "/dev/ttySLAM";    // 串口设备路径
bool uartType = false;  // 串口是否打开标志
rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher;     // ROS2发布者

// 函数声明g::Twist::SharedPtr msg);
void nav_sub(const std_msgs::msg::String::SharedPtr msg);
void upadate(uint8_t data[], uint8_t len);
void readSerialPort(const geometry_msgs::msg::PoseStamped& goal_pose);

class SerialNode : public rclcpp::Node
{
public:
     rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_;
     //rclcpp::Subscription<std_msgs::msg::String>::SharedPtr nav_sub_;
     rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub;
     rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr action_client_;
     rclcpp::Subscription<std_msgs::msg::String>::SharedPtr navisub_;
     rclcpp::TimerBase::SharedPtr timer_;
     geometry_msgs::msg::PoseStamped goal_pose;
         	
    SerialNode() : Node("serial_port")
    {
        uartInit();

        goal_pose.header.stamp = this->now();  // 使用当前时间戳
	    goal_pose.header.frame_id = "map";  // 设置目标坐标系为 map
	    goal_pose.header.stamp = this->now();  // 使用当前时间戳
	    goal_pose.pose.position.x = 1.0;       // 设置目标 X 坐标
	    goal_pose.pose.position.y = 2.0;       // 设置目标 Y 坐标

    // 订阅cmd_vel和navipub话题，设置回调函数 
	sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "cmd_vel_nav", 10, std::bind(&SerialNode::cmd_vel_cbk, this, _1));
    pub = this->create_publisher<std_msgs::msg::String>("/judge", 10);// 创建judge话题发布者
    timer_ = this->create_wall_timer(
            100ms,
            [this]() { readSerialPort(goal_pose); }
    );
            
	using Action = nav2_msgs::action::NavigateToPose;
	action_client_ = rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(
    		this, "navigate_to_pose");// 在构造函数中初始化                
    }

    void uartInit()
    {
        usbType = "/dev/ttySLAM";
        Baudrate = 115200;//波特率
        //创建timeout
        to = serial::Timeout::simpleTimeout(Uart_timeOut);
        //设置要打开的串口名称
        sp.setPort(usbType);
        //设置串口通信的波特率
        sp.setBaudrate(Baudrate);
        //串口设置timeout
        sp.setTimeout(to);

        try
        {
            sp.open();//打开串口
        }
        catch (serial::IOException& e)
        {
            RCLCPP_ERROR(this->get_logger(), "Unable to open port.");
            uartType = false;
            return;
        }

        if (sp.isOpen())//判断串口是否打开成功
        {
            RCLCPP_INFO(this->get_logger(), "/dev/ttyUSBSLAM is opened.");
        }
        else
        {
            uartType = false;
            return;
        }
        uartType = true;
    }

    void cmd_vel_cbk(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        // 构造要发送的数据包
        uint8_t send_buff[CMD_BUFF_LEN] = { 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdd };
        // 转换为整数并保留符号
        int16_t linear_x_int = static_cast<int16_t>(msg->linear.x * 10000);
        int16_t linear_y_int = static_cast<int16_t>(msg->linear.y * 10000);
        int16_t angular_z_int = static_cast<int16_t>(msg->angular.z * 10000);
        // 处理线性X速度
        send_buff[1] = (linear_x_int >> 8) & 0xFF; // 高字节（包含符号位）
        send_buff[2] = linear_x_int & 0xFF;        // 低字节
        // 处理线性Y速度
        send_buff[3] = (linear_y_int >> 8) & 0xFF;
        send_buff[4] = linear_y_int & 0xFF;
        // 处理角速度Z
        send_buff[5] = (angular_z_int >> 8) & 0xFF;
        send_buff[6] = angular_z_int & 0xFF;
        send_buff[7] = mode;
        // 打印接收到的速度信息
        RCLCPP_INFO(this->get_logger(), "Received linear.x: %f, linear.y: %f, angular.z: %f",
            msg->linear.x, msg->linear.y, msg->angular.z);
        // 发送数据包
        if (uartType)
        {
            sp.write(send_buff, CMD_BUFF_LEN);
        }
    }

    void upadate(uint8_t data[], uint8_t len)
    {
        if (len == 14 && data[0] == 0xff && data[len - 1] == 0xdd)
        {
            int MOhp = ((data[1] << 8) & 0xff00) | (data[2] & 0x00ff);
            int TOhp = ((data[3] << 8) & 0xff00) | (data[4] & 0x00ff);
            int Mhp = ((data[5] << 8) & 0xff00) | (data[6] & 0x00ff);
            uint8_t Start = 4;//data[7];
            float aimX = data[8] + data[9] / 100.0;
            float aimY = data[10] + data[11] / 100.0;
            char key = data[12];
            // 将接收到的数据组装成字符串并发布
            std_msgs::msg::String send_data;
            send_data.data = to_string(MOhp) + "|" + to_string(TOhp) + "|" + to_string(Mhp) + "|" + to_string(Start) + "|" + to_string(aimX) + "|" + to_string(aimY) + "|" + key;
            pub->publish(send_data);
            RCLCPP_INFO(this->get_logger(), "Received data (bytes): %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9]);
        }
    }

    // void readSerialPort(const geometry_msgs::msg::PoseStamped& goal_pose)
    // {
    //     using namespace std::placeholders;// 使用 NavigateToPose 动作类型
    //     using Action = nav2_msgs::action::NavigateToPose;// 使用动作目标句柄类型
    //     using GoalHandle = rclcpp_action::ClientGoalHandle<Action>;
        
    //     // 创建一个 NavigateToPose 的目标消息
    //     auto goal_msg = Action::Goal();
    //     goal_msg.pose = goal_pose;  // 设置目标姿势
    //     auto send_goal_options = rclcpp_action::Client<Action>::SendGoalOptions();

    //     if (sp.isOpen())
    //     {
    //         std::vector<uint8_t> receivedata(CMD_BUFF_LEN); // 接收数据缓冲区
    //         size_t bytes_read = sp.read(receivedata.data(), receivedata.size());
    //         RCLCPP_INFO(this->get_logger(), "Bytes read: %zu", bytes_read); // 打印读取的字节数
    //         uint8_t receive_buff[CMD_BUFF_LEN] = {0};
    //         size_t receive_size = std::min(bytes_read, receivedata.size());
    //         memcpy(receive_buff, receivedata.data(), receive_size);

    //         if (bytes_read >=3 && receive_buff[0] == 0xee && receive_buff[2] == 0xcc)
    //         {
    //             if (receive_buff[1] == 02)
    //             {
    //                 geometry_msgs::msg::PoseStamped goal_pose;
    //                 goal_pose.header.frame_id = "map";  // 设置目标坐标系为 map
    //                 goal_pose.header.stamp = this->now();  // 使用当前时间戳
    //                 goal_pose.pose.position.x = 1.0;       // 设置目标 X 坐标
    //                 goal_pose.pose.position.y = 2.0;       // 设置目标 Y 坐标
    //                 auto goal_msg = Action::Goal();
    //                 goal_msg.pose = goal_pose;  // 设置目标姿势
    //                 auto send_goal_options = rclcpp_action::Client<Action>::SendGoalOptions();
    //                 send_goal_options.goal_response_callback =
    //                     std::bind(&SerialNode::goal_response_callback, this, _1);
    //                 send_goal_options.feedback_callback =
    //                     std::bind(&SerialNode::feedback_callback, this, _1, _2);
    //                 send_goal_options.result_callback =
    //                     std::bind(&SerialNode::result_callback, this, _1);
                   
    //                 action_client_->async_send_goal(goal_msg, send_goal_options);// 使用动作客户端异步发送目标
    //             }
                
    //             if (receive_buff[1] == 01)
    //             {
    //                 geometry_msgs::msg::PoseStamped goal_pose;
    //                 goal_pose.header.frame_id = "map";  // 设置目标坐标系为 map
    //                 goal_pose.header.stamp = this->now();  // 使用当前时间戳
    //                 goal_pose.pose.position.x = -0.8;       // 设置目标 X 坐标
    //                 goal_pose.pose.position.y = 2.0;       // 设置目标 Y 坐标
    //                 auto goal_msg = Action::Goal();
    //                 goal_msg.pose = goal_pose;  // 设置目标姿势
    //                 auto send_goal_options = rclcpp_action::Client<Action>::SendGoalOptions();
    //                 send_goal_options.goal_response_callback =
    //                     std::bind(&SerialNode::goal_response_callback, this, _1);
    //                 send_goal_options.feedback_callback =
    //                     std::bind(&SerialNode::feedback_callback, this, _1, _2);
    //                 send_goal_options.result_callback =
    //                     std::bind(&SerialNode::result_callback, this, _1);
                   
    //                 action_client_->async_send_goal(goal_msg, send_goal_options);// 使用动作客户端异步发送目标
    //             }
                
    //             // 打印接收到的数据（十六进制形式）
    //             RCLCPP_INFO(this->get_logger(), "Received data: ");
    //             for (uint8_t byte : receivedata)
    //             {
    //                 RCLCPP_INFO(this->get_logger(), "%02x ", byte);
    //             }
    //         }
    //     }
    //     else
    //     {
    //         RCLCPP_ERROR(this->get_logger(), "Serial port is not open.");
    //     }
    // }
    void readSerialPort(const geometry_msgs::msg::PoseStamped& goal_pose)
{
    using namespace std::placeholders;
    using Action = nav2_msgs::action::NavigateToPose;
    using GoalHandle = rclcpp_action::ClientGoalHandle<Action>;

    if (sp.isOpen())
    {
        // 动态调整缓冲区大小
        std::vector<uint8_t> receivedata;
        size_t bytes_available = sp.available(); // 获取可用数据量
        //memcpy(receive_buff, receivedata.data(), receive_size);
        if (bytes_available > 0)
        {
            receivedata.resize(bytes_available); // 调整缓冲区大小
            size_t bytes_read = sp.read(receivedata.data(), receivedata.size()); // 读取数据
            RCLCPP_INFO(this->get_logger(), "Bytes read: %zu", bytes_read); // 打印读取的字节数

            // 清空缓冲区
            receivedata.resize(bytes_read);

            // 处理接收到的数据
            // if (bytes_read >= 3 && receivedata[0] == 0xee && receivedata[2] == 0xcc)
            // {
            //     if (receivedata[1] == 0x03)
            //     {
            //         // 设置目标位置并发送导航目标
            //         geometry_msgs::msg::PoseStamped goal_pose;
            //         goal_pose.header.frame_id = "map";  // 设置目标坐标系为 map
            //         goal_pose.header.stamp = this->now();  // 使用当前时间戳
            //         goal_pose.pose.position.x = -1.8;       // 设置目标 X 坐标
            //         goal_pose.pose.position.y = -1.0;       // 设置目标 Y 坐标
            //         auto goal_msg = Action::Goal();
            //         goal_msg.pose = goal_pose;  // 设置目标姿势
            //         auto send_goal_options = rclcpp_action::Client<Action>::SendGoalOptions();
            //         send_goal_options.goal_response_callback =
            //             std::bind(&SerialNode::goal_response_callback, this, _1);
            //         send_goal_options.feedback_callback =
            //             std::bind(&SerialNode::feedback_callback, this, _1, _2);
            //         send_goal_options.result_callback =
            //             std::bind(&SerialNode::result_callback, this, _1);
                   
            //         action_client_->async_send_goal(goal_msg, send_goal_options);// 使用动作客户端异步发送目标
            //     }
                
            //     if (receivedata[1] == 0x01)
            //     {
            //         // 设置目标位置并发送导航目标
            //         geometry_msgs::msg::PoseStamped goal_pose;
            //         goal_pose.header.frame_id = "map";  // 设置目标坐标系为 map
            //         goal_pose.header.stamp = this->now();  // 使用当前时间戳
            //         goal_pose.pose.position.x = -0.8;       // 设置目标 X 坐标
            //         goal_pose.pose.position.y = 2.0;       // 设置目标 Y 坐标
            //         auto goal_msg = Action::Goal();
            //         goal_msg.pose = goal_pose;  // 设置目标姿势
            //         auto send_goal_options = rclcpp_action::Client<Action>::SendGoalOptions();
            //         send_goal_options.goal_response_callback =
            //             std::bind(&SerialNode::goal_response_callback, this, _1);
            //         send_goal_options.feedback_callback =
            //             std::bind(&SerialNode::feedback_callback, this, _1, _2);
            //         send_goal_options.result_callback =
            //             std::bind(&SerialNode::result_callback, this, _1);
                   
            //         action_client_->async_send_goal(goal_msg, send_goal_options);// 使用动作客户端异步发送目标
            //     }
                
            //     // 打印接收到的数据（十六进制形式）
            //     RCLCPP_INFO(this->get_logger(), "Received data: ");
            //     for (uint8_t byte : receivedata)
            //     {
            //         RCLCPP_INFO(this->get_logger(), "%02x ", byte);
            //     }
            // }
        }
    }
    else
    {
        RCLCPP_ERROR(this->get_logger(), "Serial port is not open.");
    }
}
    
    // void SerialNode::nav_sub_cb(const std_msgs::msg::String::SharedPtr msg);
    
    void goal_response_callback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr goal_handle)
	{
    		if (!goal_handle) {
        	RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
    			} 
    		else {
        		RCLCPP_INFO(this->get_logger(), "Goal accepted by server, waiting for result");
    			}
	}

	
    void feedback_callback(rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr,
                       const std::shared_ptr<const nav2_msgs::action::NavigateToPose::Feedback>)
	{
   		 // 可以打印反馈信息，如当前位姿
    		RCLCPP_INFO(this->get_logger(), "Feedback received");
	}

    void result_callback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult& result)
	{
    		switch (result.code) {
        	case rclcpp_action::ResultCode::SUCCEEDED:
            		RCLCPP_INFO(this->get_logger(), "Goal succeeded!");
           		break;
        	case rclcpp_action::ResultCode::ABORTED:
            		RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
            		break;
        	case rclcpp_action::ResultCode::CANCELED:
            		RCLCPP_INFO(this->get_logger(), "Goal was canceled");
            		break;
        	default:
            		RCLCPP_ERROR(this->get_logger(), "Unknown result code");
    		}
	}

private:
    
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SerialNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
