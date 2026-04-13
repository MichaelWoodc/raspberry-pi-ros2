###############################################
# ROS 2 Kilted Binary Install – Ubuntu 24.04
# Clean, minimal, correct, and dependency-safe
###############################################

# 1. Fix locale (ROS requires UTF‑8)
sudo apt update
sudo apt install -y locales
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

# 2. Ensure Ubuntu sources include noble-updates (critical!)
# Edit this file and ensure Suites includes:
#   noble noble-updates noble-backports
sudo nano /etc/apt/sources.list.d/ubuntu.sources

# Example correct block:
# Types: deb
# URIs: http://ports.ubuntu.com/ubuntu-ports/
# Suites: noble noble-updates noble-backports
# Components: main restricted universe multiverse
# Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
#
# Types: deb
# URIs: http://ports.ubuntu.com/ubuntu-ports/
# Suites: noble-security
# Components: main restricted universe multiverse
# Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg

# 3. Update and repair system packages
sudo apt update
sudo apt --fix-broken install -y

# 4. Install required tools for extracting ROS binaries
sudo apt install -y tar bzip2 wget

# 5. Create ROS install directory
mkdir -p ~/ros2_kilted
cd ~/ros2_kilted

# 6. Download ROS 2 Kilted (ARM64, Noble)
wget https://github.com/ros2/ros2/releases/download/release-kilted-20250728/ros2-kilted-20250728-linux-noble-arm64.tar.bz2

# 7. Extract ROS 2
tar xf ros2-kilted-20250728-linux-noble-arm64.tar.bz2

# 8. Install rosdep
sudo apt install -y python3-rosdep
sudo rosdep init
rosdep update

# 9. Install ROS 2 dependencies (skip middleware you don’t need)
rosdep install --from-paths ~/ros2_kilted/ros2-linux/share \
  --ignore-src -y \
  --skip-keys "cyclonedds fastcdr fastdds iceoryx_binding_c rmw_connextdds rti-connext-dds-7.3.0 urdfdom_headers"

# 10. Source ROS 2 for this shell
source ~/ros2_kilted/ros2-linux/setup.bash

# 11. Make ROS 2 sourcing persistent
echo "source ~/ros2_kilted/ros2-linux/setup.bash" >> ~/.bashrc
source ~/.bashrc

# 12. Verify installation
ros2 --version
###############################################




































## FULL history to get ros2 running on Ubuntu Server on a Raspberry Pi 5:

science@pi-ubuntu-server3:~$ history
    1  lsblk
    2  history
    3  sudo raspi-config
    4  sudo apt install raspi-config
    5  sudo shutdown now
    6  ifconfig
    7  ip all
    8  ip a
    9  locale  # check for UTF-8
   10  sudo apt update && sudo apt install locales
   11  sudo locale-gen en_US en_US.UTF-8
   12  sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
   13  export LANG=en_US.UTF-8
   14  locale  # verify settings
   15  sudo apt update && sudo apt upgrade -y
   16  sudo apt install software-properties-common
   17  sudo add-apt-repository universe
   18  sudo apt update && sudo apt install curl -y
   19  export ROS_APT_SOURCE_VERSION=$(curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest | grep -F "tag_name" | awk -F'"' '{print $4}')
   20  curl -L -o /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.$(. /etc/os-release && echo ${UBUNTU_CODENAME:-${VERSION_CODENAME}})_all.deb"
   21  sudo dpkg -i /tmp/ros2-apt-source.deb
   22  sudo apt install tar bzip2 wget -y
   23  sudo apt install libbz2-1.0/noble
   24  sudo apt install tar bzip2 wget -y
   25  sudo apt update && sudo apt install ros-dev-tools
   26  mkdir -p ~/ros2_kilted
   27  cd ~/ros2_kilted
   28  wget https://github.com/ros2/ros2/releases/download/release-kilted-20250728/ros2-kilted-20250728-linux-noble-arm64.tar.bz2
   29  tar xf ros2-kilted-20250728-linux-noble-arm64.tar.bz2
   30  sudo apt update && sudo apt upgrade -y
   31  sudo apt update
   32  sudo apt install -y python3-rosdep
   33  sudo rosdep init
   34  rosdep update
   35  rosdep install --from-paths ~/ros2_kilted/ros2-linux/share --ignore-src -y --skip-keys "cyclonedds fastcdr fastdds iceoryx_binding_c rmw_connextdds rti-connext-dds-7.3.0 urdfdom_headers"
   36  sudo rm /etc/apt/sources.list.d/ros2.list
   37  sudo apt update
   38  sudo apt --fix-broken install
   39  sudo apt install --reinstall zlib1g zlib1g-dev
   40  ls /etc/apt/sources.list.d/
   41  sudo rm /etc/apt/sources.list.d/ros2.list
   42  sudo rm /etc/apt/sources.list.d/ros-latest.list
   43  sudo rm /etc/apt/sources.list.d/ros2-latest.list
   44  sudo rm /etc/apt/sources.list.d/ros2.sources
   45  sudo apt update
   46  sudo apt --fix-broken install
   47  sudo apt install --reinstall zlib1g zlib1g-dev
   48  ls /etc/apt/sources.list.d/
   49  cd ~
   50  cat /etc/apt/sources.list
   51  apt-cache policy zlib1g
   52  cat /etc/apt/sources.list.d/ubuntu.sources
   53  sudo nano /etc/apt/sources.list.d/ubuntu.sources
   54  sudo apt update
   55  sudo apt --fix-broken install
   56  sudo apt install --reinstall zlib1g zlib1g-dev
   57  rosdep update
   58  rosdep install --from-paths ~/ros2_kilted/ros2-linux/share --ignore-src -y --skip-keys "cyclonedds fastcdr fastdds iceoryx_binding_c rmw_connextdds rti-connext-dds-7.3.0 urdfdom_headers"
   59  source ~/ros2_kilted/ros2-linux/setup.bash
   60  ros2 --version
   61  ros2
   62  echo "source ~/ros2_kilted/ros2-linux/setup.bash" >> ~/.bashrc
   63  source ~/.bashrc
   64  ros2
   65  history
