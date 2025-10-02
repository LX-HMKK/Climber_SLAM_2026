import os
import sys
import yaml
from ament_index_python.packages import get_package_share_directory
from launch.substitutions import Command, LaunchConfiguration
from launch.actions import DeclareLaunchArgument
sys.path.append(os.path.join(get_package_share_directory('rm_bringup'), 'launch'))
def generate_launch_description():
    from launch_ros.descriptions import ComposableNode
    from launch_ros.actions import ComposableNodeContainer, Node, SetParameter
    from launch.actions import TimerAction, Shutdown
    from launch import LaunchDescription
    # 添加命名空间参数
    namespace_arg = DeclareLaunchArgument(
        'namespace',
        default_value='',
        description='Namespace for the nodes'
    )
    namespace = LaunchConfiguration('namespace')
    launch_params = yaml.safe_load(open(os.path.join(
        get_package_share_directory('rm_bringup'), 'config', 'launch_params.yaml')))
    SetParameter(name='rune', value=launch_params['rune']),
    robot_description = Command(['xacro ', os.path.join(
        get_package_share_directory('rm_robot_description'), 'urdf', 'sentry.urdf.xacro'),
        ' xyz:=', launch_params['base2camera']['xyz'],
        ' rpy:=', launch_params['base2camera']['rpy']])
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        namespace=namespace,  # 添加命名空间
        parameters=[{'robot_description': robot_description, 'publish_frequency': 1000.0}]
    )
    node_params = os.path.join(
        get_package_share_directory('rm_bringup'), 'config', 'node_params.yaml')
    # 以下是您现有的注释代码，我已经在每个节点添加了命名空间参数
    # 如果您取消注释这些代码，请确保添加 namespace=namespace 参数
    # if launch_params['video_play']:
    #     image_node = ComposableNode(
    #         package='rm_camera_driver',
    #         plugin='fyt::camera_driver::VideoPlayerNode',
    #         name='video_player',
    #         namespace=namespace,  # 添加命名空间
    #         parameters=[node_params],
    #         extra_arguments=[{'use_intra_process_comms': True}]
    #     )
    # else:
    #     image_node = ComposableNode(
    #         package='rm_camera_driver',
    #         plugin='fyt::camera_driver::DahengCameraNode',
    #         name='camera_driver',
    #         namespace=namespace,  # 添加命名空间
    #         parameters=[node_params],
    #         extra_arguments=[{'use_intra_process_comms': True}]
    #     )
    # 串口部分和其他注释代码也需要类似修改
    # 创建启动描述
    ld = LaunchDescription()
    # 添加命名空间参数
    ld.add_action(namespace_arg)
    # 添加其他节点
    ld.add_action(robot_state_publisher)
    # 添加其他您需要的节点...
    return ld