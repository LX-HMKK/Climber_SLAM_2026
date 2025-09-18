#!/bin/bash

cmds=(
	"colcon build --symlink-install"
	#"sudo chmod 666 /dev/tty*"
	"ros2 launch rm_bringup bringup.launch.py"
	"ros2 launch livox_ros_driver2 msg_MID360_launch.py"
	#"ros2 launch rplidar_ros rplidar_a2m7_launch.py"
	"ros2 launch linefit_ground_segmentation_ros segmentation.launch.py" 
	"ros2 launch fast_lio mapping.launch.py"
	"ros2 launch imu_complementary_filter complementary_filter.launch.py"
	"ros2 launch pointcloud_to_laserscan pointcloud_to_laserscan_launch.py"
	"ros2 launch icp_registration icp.launch.py"
	"ros2 launch rm_navigation bringup_launch.py"
	"ros2 run rm_communication talker"
)

work_dir="/home/old-nuc/climber_ws/"

# 创建主终端窗口
gnome-terminal --tab --title="Command 0" --working-directory="$work_dir" \
    -- bash -c "source install/setup.bash; ${cmds[0]}; exec bash"

# 添加新标签页
for ((i=1; i<${#cmds[@]}; i++)); do
    gnome-terminal --tab --title="Command $i" --working-directory="$work_dir" \
        -- bash -c "source install/setup.bash; ${cmds[i]}; exec bash"
done

#!/bin/bash

#cmds=(
#	"colcon build --symlink-install"
#	"sudo chmod 666 /dev/tty*"
#	"ros2 launch rm_bringup bringup.launch.py"
#	"ros2 launch livox_ros_driver2 msg_MID360_launch.py"
#	#"ros2 launch rplidar_ros rplidar_a2m7_launch.py"
#	"ros2 launch linefit_ground_segmentation_ros segmentation.launch.py" 
#	"ros2 launch fast_lio mapping.launch.py"
#	"ros2 launch imu_complementary_filter complementary_filter.launch.py"
#	"ros2 launch pointcloud_to_laserscan pointcloud_to_laserscan_launch.py"
#	"ros2 launch icp_registration icp.launch.py"
#	"ros2 launch rm_navigation bringup_launch.py"
#	"ros2 run rm_communication talker"
#	)

#for cmd in "${cmds[@]}";
#do
#	echo Current CMD : "$cmd"
#	gnome-terminal -- bash -c "cd /home/old-nuc/climber_ws/;source install/setup.bash;$cmd;exec bash;"
#	sleep 0.2
#done
